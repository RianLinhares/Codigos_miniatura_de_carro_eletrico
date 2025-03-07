
import paho.mqtt.client as mqtt
import os
from pygame import mixer
import time

# Configuração do MQTT
BROKER_IP = "test.mosquitto.org"
TOPIC = "car/status"

# Dicionário de mapeamento de mensagens para estados de colisão
mapa_mensagens_colisao = {
    "Distante": "baixo_colisao.mp3",  # Áudio para baixo índice de colisão
    "Media_distancia": "medio_colisao.mp3",  # Áudio para médio índice de colisão
    "Perto": "alto_colisao.mp3"  # Áudio para alto índice de colisão
}

# Inicializa o mixer do Pygame para tocar áudio
mixer.init()

# Variáveis de controle de estado
audio_tocando = False
audio_atual = None

def tocar_audio(audio_file):
    global audio_tocando, audio_atual
    # Verifica se o arquivo existe e toca o áudio
    if os.path.exists(audio_file):
        if not audio_tocando or audio_file != audio_atual:
            try:
                mixer.music.load(audio_file)
                mixer.music.play(loops=-1, start=0.0)  # 'loops=-1' faz o áudio tocar indefinidamente
                audio_tocando = True
                audio_atual = audio_file
            except Exception as e:
                print(f"Erro ao tocar áudio: {e}")

def parar_audio():
    global audio_tocando, audio_atual
    mixer.music.stop()  # Para o áudio
    audio_tocando = False
    audio_atual = None

# Callback para conexão ao MQTT
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Conectado ao broker MQTT!")
        client.subscribe(TOPIC)
    else:
        print(f"Falha na conexão, código de retorno: {rc}")

# Callback para recebimento de mensagens
def on_message(client, userdata, msg):
    global audio_tocando
    mensagem = msg.payload.decode()
    print(f"Mensagem recebida: {mensagem}")
    if mensagem == "parar":
        parar_audio()  # Para o áudio, mas o programa continuará em execução
        print("Áudio parado.")
    elif mensagem in mapa_mensagens_colisao:
        if not audio_tocando or mapa_mensagens_colisao[mensagem] != audio_atual:
            # Toca o áudio correspondente ao novo comando, se necessário
            tocar_audio(mapa_mensagens_colisao[mensagem])

# Configuração do cliente MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Conectando ao broker MQTT
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

# Este código irá rodar até que o processo seja interrompido
try:
    while True:
        # Aguarda novos comandos enquanto o áudio continua tocando
        time.sleep(1)  # Aguardar o recebimento das mensagens MQTT
except KeyboardInterrupt:
    print("Processo interrompido pelo usuário.")
