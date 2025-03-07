# Projeto de Controle Automotivo via MQTT

Este projeto simula um sistema automotivo controlado por joysticks e comunicação MQTT. Ele é composto por várias ECUs embarcadas, conectadas via barramento CAN, e uma ECU remota para controle.

## Arquitetura do Sistema

- **ECU Lighting:** Controla os LEDs representando luzes e setas.
- **ECU Sensing:** Gerencia dados de sensores e envia informações via CAN.
- **ECU Powering:** Controla os motores e movimento.
- **ECU Communication Wireless:** Recebe comandos via MQTT e os retransmite via CAN.
- **Remote ECU Driver:** Interface para enviar comandos e visualizar dados.

## Requisitos

- Python 3.x
- Bibliotecas: `pygame`, `paho-mqtt`
- Microcontroladores: ESP32, Arduinos
- Módulos: MCP2515 para comunicação CAN

## Configuração

1. Configure a rede Wi-Fi no código ESP32.
2. Conecte os módulos MCP2515 aos Arduinos.
3. Ajuste o broker MQTT conforme necessário.

## Execução

1. Inicie o script Python envio_de_comando_esp32 para controle via joystick.
2. Execute o script Python cluster para visualizar o painel de instrumentos.
3. Execute o script Python sensor_de_re para ativar o sensor de ré.
4. As ECUs responderão aos comandos enviados.
5. A interface gráfica exibirá o estado das luzes e setas.

### Comandos para controlar a miniatura de carro elétrico
![Comando para controlar a miniatura de carro elétrico](https://raw.githubusercontent.com/RianLinhares/Codigos_miniatura_de_carro_eletrico/refs/heads/main/comando_carrinho.png)
