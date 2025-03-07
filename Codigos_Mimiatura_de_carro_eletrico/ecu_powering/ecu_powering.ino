#include <SPI.h>
#include <mcp2515.h>

// Configuração do MCP2515
const int CAN_CS_PIN = 10; // Pino CS do módulo MCP2515
MCP2515 mcp2515(CAN_CS_PIN);

// Pinos dos motores (conforme seu código original)
#define motor1_ENABLE 3 // LADO DIREITO
int motor1_IN1 = 4;
int motor1_IN2 = 5;

int motor2_IN1 = 7; //LADO ESQUERDO
int motor2_IN2 = 8;
int motor2_ENABLE = 9;

void setup() {
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  // Configuração dos motores
  pinMode(motor1_IN1, OUTPUT);
  pinMode(motor1_IN2, OUTPUT);
  pinMode(motor1_ENABLE, OUTPUT);

  pinMode(motor2_IN1, OUTPUT);
  pinMode(motor2_IN2, OUTPUT);
  pinMode(motor2_ENABLE, OUTPUT);
}

void loop() {
  // Buffer para receber mensagens
  struct can_frame canMsg;

  // Verifica se há mensagens CAN recebidas
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    // Verifica se o ID da mensagem é 0xAF
    if (canMsg.can_id == 0xAF) {
      Serial.print("Mensagem recebida com ID: 0x");
      Serial.println(canMsg.can_id, HEX);

      for (int i = 0; i < canMsg.can_dlc; i++) {  // Exibe os dados
        Serial.print(canMsg.data[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      int dado = canMsg.data[1]; // Lê o dado da mensagem

      // Chama funções com base no comando
      switch (canMsg.data[0]) {
        case 0x09:
          forward(dado);
          Serial.println(dado);
          Serial.println("FOWARD");
          break;
        case 0x08:
          back(dado);
          Serial.println("BACK");
          break;
        case 0x07:
          left(dado);
          Serial.println("LEFT");
          break;
        case 0x06:
          right(dado);
          Serial.println("RIGHT");
          break;
        case 0x05:
          Stop();
          Serial.println("STOP");
          break;
        default:
          Serial.println("Comando desconhecido.");
          break;
      }
    } 
  }
}

// Funções de controle dos motores
void forward(int speed) {
  analogWrite(motor1_ENABLE, speed);
  digitalWrite(motor1_IN1, HIGH);
  digitalWrite(motor1_IN2, LOW);

  analogWrite(motor2_ENABLE, speed);
  digitalWrite(motor2_IN1, HIGH);
  digitalWrite(motor2_IN2, LOW);
  Serial.println("Movendo para frente.");
}

void back(int speed) {
  analogWrite(motor1_ENABLE, speed);
  digitalWrite(motor1_IN1, LOW);
  digitalWrite(motor1_IN2, HIGH);

  analogWrite(motor2_ENABLE, speed);
  digitalWrite(motor2_IN1, LOW);
  digitalWrite(motor2_IN2, HIGH);
  Serial.println("Movendo para trás.");
}

void right(int speed) { // speed 1 lado direito, speed 2 lado esquerdo
  analogWrite(motor1_ENABLE, speed); 
  digitalWrite(motor1_IN1, LOW);
  digitalWrite(motor1_IN2, HIGH);

  analogWrite(motor2_ENABLE, speed);
  digitalWrite(motor2_IN1, HIGH);
  digitalWrite(motor2_IN2, LOW);
  Serial.println("Virando à direita.");
}

void left(int speed) {
  analogWrite(motor1_ENABLE, speed);
  digitalWrite(motor1_IN1, HIGH);
  digitalWrite(motor1_IN2, LOW);

  analogWrite(motor2_ENABLE, speed);
  digitalWrite(motor2_IN1, LOW);
  digitalWrite(motor2_IN2, HIGH);
  Serial.println("Virando à esquerda.");
}

void Stop() {
  analogWrite(motor1_ENABLE, 0);
  digitalWrite(motor1_IN1, LOW);
  digitalWrite(motor1_IN2, LOW);

  analogWrite(motor2_ENABLE, 0);
  digitalWrite(motor2_IN1, LOW);
  digitalWrite(motor2_IN2, LOW);
  Serial.println("Parando.");
}
