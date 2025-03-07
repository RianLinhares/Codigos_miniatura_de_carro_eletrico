import pygame
import paho.mqtt.client as mqtt

# Configurações do MQTT
BROKER = "test.mosquitto.org"  # Substitua pelo endereço do seu broker
PORT = 1883  # Porta padrão do MQTT
TOPIC = "car/control"  # Tópico para enviar comandos

# Inicializar o pygame e o joystick
pygame.init()

# Inicializar cliente MQTT
client = mqtt.Client()
client.connect(BROKER, PORT, 60)

def is_close_to_zero(value, epsilon=0.05):
    """Verifica se o valor está próximo de zero, dentro de uma margem (epsilon)."""
    return abs(value) < epsilon

def send_mqtt(message):
    """Envia a mensagem via MQTT."""
    client.publish(TOPIC, message)
    print(f"Mensagem enviada via MQTT: {message}")

def main():
    # Lista para armazenar os joysticks conectados
    joysticks = []

    # Loop por todos os joysticks conectados
    for i in range(pygame.joystick.get_count()):
        joysticks.append(pygame.joystick.Joystick(i))
        joysticks[-1].init()
        print(f"Joystick detectado: '{joysticks[-1].get_name()}'")

    keepPlaying = True
    clock = pygame.time.Clock()

    while keepPlaying:
        clock.tick(60)  # 60 FPS

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                keepPlaying = False
            elif event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
                keepPlaying = False

            elif event.type == pygame.JOYAXISMOTION:
                # Atualizar variáveis de eixos
                eixo_x = joysticks[event.joy].get_axis(0)  # Eixo X
                eixo_y = joysticks[event.joy].get_axis(1)  # Eixo Y
                print(f"Eixo X: {eixo_x:.2f}, Eixo Y: {eixo_y:.2f}")

                if -1.05 <= eixo_y <= -0.95:  # Para "Forward"
                    send_mqtt("Forward")
                elif is_close_to_zero(eixo_x) and is_close_to_zero(eixo_y):
                    send_mqtt("Stop")
                elif 0.95 <= eixo_y <= 1.05:  # Para "Back"
                    send_mqtt("Back")
                elif 0.95 <= eixo_x <= 1.05:  # Para "Right"
                    send_mqtt("Right")
                elif -1.05 <= eixo_x <= -0.95:  # Para "Left"
                    send_mqtt("Left")

            elif event.type == pygame.JOYBUTTONDOWN:
                # Atualizar variáveis de botões
                if event.button == 0:
                    print("Botão A pressionado")
                    send_mqtt('Luz_Baixa')

                elif event.button == 1:
                    print("Botão B pressionado")
                    send_mqtt('Seta_Direita')

                elif event.button == 2:
                    print("Botão X pressionado")
                    send_mqtt('Seta_Esquerda')

                elif event.button == 3:
                    print("Botão Y pressionado")
                    send_mqtt('Luz_Alta')

                elif event.button == 4:
                    print("Botão LB pressionado")
                    send_mqtt('4')

                elif event.button == 5:
                    print("Botão RB pressionado")
                    send_mqtt('Pisca_Alerta')

                elif event.button == 8:
                    print("Botão Left Stick pressionado")
                    send_mqtt('8')

                elif event.button == 9:
                    print("Botão Right Stick pressionado")
                    send_mqtt('9')

            elif event.type == pygame.JOYHATMOTION:
                direcao_hat = joysticks[event.joy].get_hat(0)
                print(f"Direcional (hat) movido: {direcao_hat}")

    pygame.quit()
    client.disconnect()

# Executar o código
main()
