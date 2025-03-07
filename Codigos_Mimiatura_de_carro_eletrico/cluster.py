import sys
import paho.mqtt.client as mqtt
import time
from PyQt5.QtWidgets import QApplication, QWidget
from PyQt5.QtGui import QPainter, QPolygon, QColor, QPen
from PyQt5.QtCore import Qt, QTimer, QPoint

# Configuração do MQTT
BROKER_IP = "test.mosquitto.org"
TOPIC = "car/status"
mundanca_de_estado = None  # Variável global

# Dicionário de mapeamento de mensagens para estados
mapa_mensagens = {
    "sdl": "lsd",
    "sdd": "dsd",
    "sel": "lse",
    "sed": "dse",
    "pal": "pal",
    "pad": "dpa",
    "lha": "lha",  # Luz alta ligada
    "dlha": "dlha",# Luz alta desligada
    "lhb": "lhb",  # Luz baixa ligada
    "dlhb": "dlhb",  # Luz baixa desligada
    "parar": "des"
}

class InstrumentCluster(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Painel de Instrumentos - Setas e Faróis")
        self.setGeometry(100, 100, 350, 250)  # Aumentado para melhor disposição dos elementos

        # Estados das setas e faróis
        self.seta_esquerda_ativa = False
        self.seta_direita_ativa = False
        self.pisca_estado = False
        self.luz_alta_ativa = False
        self.luz_baixa_ativa = False

        # Timer para piscar as setas
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.toggle_pisca)

        # Timer para verificar mudanças de estado
        self.estado_timer = QTimer(self)
        self.estado_timer.timeout.connect(self.verificar_mudanca_estado)
        self.estado_timer.start(500)  # Verifica mudanças a cada 500ms

        self.ultimo_estado = ""

    def paintEvent(self, event):
        painter = QPainter(self)

        # Cor padrão (desligado)
        cor_desligada = QColor(100, 100, 100)  # Cinza
        cor_piscando = QColor(0, 255, 0)  # Verde

        cor_esquerda = cor_piscando if self.seta_esquerda_ativa and self.pisca_estado else cor_desligada
        cor_direita = cor_piscando if self.seta_direita_ativa and self.pisca_estado else cor_desligada

        # Desenha a seta esquerda
        seta_esquerda = QPolygon([
            QPoint(100, 130), QPoint(150, 100), QPoint(150, 160)
        ])
        painter.setBrush(cor_esquerda)
        painter.drawPolygon(seta_esquerda)

        # Desenha a seta direita
        seta_direita = QPolygon([
            QPoint(250, 130), QPoint(200, 100), QPoint(200, 160)
        ])
        painter.setBrush(cor_direita)
        painter.drawPolygon(seta_direita)

        # ** Desenho da luz alta (à esquerda da seta esquerda) **
        if self.luz_alta_ativa:
            pen = QPen(QColor(0, 0, 255))  # Cor azul
            pen.setWidth(3)  # Define a espessura da linha (aumente conforme necessário)
            painter.setPen(pen)
            painter.setBrush(Qt.NoBrush)

            # Desenha o elipse (farol)
            painter.drawEllipse(30, 70, 30, 20)

            # Desenha os feixes de luz mais grossos
            painter.drawLine(60, 75, 90, 75)
            painter.drawLine(60, 80, 90, 80)
            painter.drawLine(60, 85, 90, 85)

        # ** Desenho da luz baixa (abaixo da luz alta, sem sobrepor a seta) **
        if self.luz_baixa_ativa:
            pen2 = QPen(QColor(0, 255, 0))  # Cor verde
            pen2.setWidth(3)  # Define a espessura da linha (aumente conforme necessário)
            painter.setPen(pen2)
            painter.setBrush(Qt.NoBrush)

            painter.drawEllipse(30, 100, 30, 20)

            painter.drawLine(60, 105, 80, 115)
            painter.drawLine(60, 110, 80, 120)
            painter.drawLine(60, 115, 80, 125)

    def verificar_mudanca_estado(self):
        global mundanca_de_estado

        if mundanca_de_estado != self.ultimo_estado:
            self.ultimo_estado = mundanca_de_estado

            if mundanca_de_estado == "lsd":
                self.seta_direita_ativa = True
                self.seta_esquerda_ativa = False
                self.timer.start(500)
            elif mundanca_de_estado == "dsd":
                self.seta_direita_ativa = False
                self.timer.stop()
            elif mundanca_de_estado == "lse":
                self.seta_esquerda_ativa = True
                self.seta_direita_ativa = False
                self.timer.start(500)
            elif mundanca_de_estado == "dse":
                self.seta_esquerda_ativa = False
                self.timer.stop()
            elif mundanca_de_estado == "pal":
                self.seta_esquerda_ativa = True
                self.seta_direita_ativa = True
                self.timer.start(500)
            elif mundanca_de_estado == "dpa":
                self.seta_esquerda_ativa = False
                self.seta_direita_ativa = False
                self.timer.stop()
            elif mundanca_de_estado == "lha":
                self.luz_alta_ativa = True
            elif mundanca_de_estado == "dlha":
                self.luz_alta_ativa = False
            elif mundanca_de_estado == "lhb":
                self.luz_baixa_ativa = True
            elif mundanca_de_estado == "dlhb":
                self.luz_baixa_ativa = False
            elif mundanca_de_estado == "des":
                self.luz_baixa_ativa = False
                self.luz_alta_ativa = False

            self.update()

    def toggle_pisca(self):
        self.pisca_estado = not self.pisca_estado
        self.update()

# Callback para conexão ao MQTT
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Conectado ao broker MQTT!")
        client.subscribe(TOPIC)
    else:
        print(f"Falha na conexão, código de retorno: {rc}")

# Callback para recebimento de mensagens
def on_message(client, userdata, msg):
    global mundanca_de_estado
    mensagem = msg.payload.decode()
    print(f"Mensagem recebida: {mensagem}")
    if mensagem in mapa_mensagens:
        mundanca_de_estado = mapa_mensagens[mensagem]

# Configuração do cliente MQTT
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

while True:
    try:
        print("Tentando conectar ao broker MQTT...")
        client.connect(BROKER_IP, 1883, 60)
        break
    except Exception as e:
        print(f"Erro ao conectar: {e}")
        print("Tentando novamente em 5 segundos...")
        time.sleep(5)

client.loop_start()

app = QApplication(sys.argv)
window = InstrumentCluster()
window.show()
sys.exit(app.exec_())