#include <SPI.h>
#include <mcp2515.h>

// Configuração do MCP2515
const int CAN_CS_PIN = 10; // Pino CS do módulo MCP2515
MCP2515 mcp2515(CAN_CS_PIN);

// Pinos dos LEDs
int seta_direita = 2;
int luz_posicao_d = 3, luz_re_d = 4, luz_alta = 5;
int luz_baixa = 6, seta_esquerda = 7, luz_re_e = 8;
int luz_posicao_e = 9;

// Variáveis de estado
bool piscar_seta_direita = false;
bool piscar_seta_esquerda = false;

void setup() {
  Serial.begin(115200);

  // Inicializa o MCP2515
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  // Configuração dos pinos dos LEDs
  pinMode(luz_alta, OUTPUT);
  pinMode(seta_direita, OUTPUT);
  pinMode(luz_baixa, OUTPUT);
  pinMode(luz_posicao_d, OUTPUT);
  pinMode(luz_re_d, OUTPUT);
  pinMode(seta_esquerda, OUTPUT);
  pinMode(luz_re_e, OUTPUT);
  pinMode(luz_posicao_e, OUTPUT);

  digitalWrite(luz_alta, LOW); // LEDs desligados inicialmente
  digitalWrite(seta_direita, LOW);
  digitalWrite(luz_baixa, LOW);
  digitalWrite(luz_posicao_d, LOW);
  digitalWrite(luz_re_d, LOW);
  digitalWrite(seta_esquerda, LOW);
  digitalWrite(luz_re_e, LOW);
  digitalWrite(luz_posicao_e, LOW);

  Serial.println("Sistema iniciado. Aguardando mensagens CAN...");

}

void loop() {
  // Buffer para receber mensagens
  struct can_frame canMsg;

  // Verifica se há mensagens CAN recebidas
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    // Verifica se o ID da mensagem é 0x22F
    if (canMsg.can_id == 0x22F || canMsg.can_id == 0x22E ) {
      Serial.print("Mensagem recebida com ID: 0x");
      Serial.println(canMsg.can_id, HEX);

      // Exibe os dados da mensagem recebida
      for (int i = 0; i < canMsg.can_dlc; i++) {
        Serial.print(canMsg.data[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      // Lê o comando do joystick
      switch (canMsg.data[0]) {

        case 0x01:
          digitalWrite(luz_baixa, HIGH);
          digitalWrite(luz_posicao_d, HIGH);
          digitalWrite(luz_posicao_e, HIGH);
          Serial.println("Comando recebido: Liga a Luz baixa e Luz de Posição.");
          break;

        case 0x00:
          digitalWrite(luz_baixa, LOW);
          digitalWrite(luz_posicao_d, LOW);
          digitalWrite(luz_posicao_e, LOW);
          Serial.println("Comando recebido: Desliga a Luz baixa.");
          break;

        case 0x02:
          digitalWrite(luz_alta, HIGH);
          digitalWrite(luz_baixa, LOW);
          Serial.println("Comando recebido: Liga a Luz alta.");
          Serial.println("Comando recebido: Desliga a Luz baixa.");
          break;

        case 0x03:
          digitalWrite(luz_baixa, HIGH);
          digitalWrite(luz_alta, LOW);
          Serial.println("Comando recebido: Liga a Luz baixa.");
          Serial.println("Comando recebido: Desliga a Luz alta.");
          break;

        case 0x08:
          digitalWrite(luz_re_d, HIGH);
          digitalWrite(luz_re_e, HIGH);
          Serial.println("Comando recebido: Liga a Luz de Ré");
          break;

        case 0x05:
        case 0x06:
        case 0x07:
        case 0x09:
          digitalWrite(luz_re_d, LOW);
          digitalWrite(luz_re_e, LOW);
          break;

        case 0x0A: // Liga a Seta Direita
          Serial.println("Comando recebido: Liga a Seta Direita.");
          piscar_seta_direita = true; // Ativa o estado de piscar
          break;

        case 0x0B: // Desliga a Seta Direita
          Serial.println("Comando recebido: Desliga a Seta Direita.");
          piscar_seta_direita = false; // Desativa o estado de piscar
          digitalWrite(seta_direita, LOW); // Garante que o LED esteja desligado
          break;

        case 0x0C: // Liga a Seta Esquerda
          Serial.println("Comando recebido: Liga a Seta Esquerda.");
          piscar_seta_esquerda = true; // Ativa o estado de piscar
          break;

        case 0x0D: // Desliga a Seta Esquerda
          Serial.println("Comando recebido: Desliga a Seta Esquerda.");
          piscar_seta_esquerda = false; // Desativa o estado de piscar
          digitalWrite(seta_esquerda, LOW); // Garante que o LED esteja desligado
          break;

        case 0x0E:
          digitalWrite(seta_direita, LOW);
          digitalWrite(seta_esquerda, LOW);
          delay(500);
          Serial.println("Comando recebido: Liga Pisca Alerta");
          piscar_seta_direita = true; // Ativa o estado de piscar
          piscar_seta_esquerda = true;
          break;

        case 0x0F:
          Serial.println("Comando recebido: Desliga Pisca Alerta");
          piscar_seta_direita = false; // Desativa o estado de piscar
          digitalWrite(seta_direita, LOW); // Garante que o LED esteja desligado
          piscar_seta_esquerda = false; // Desativa o estado de piscar
          digitalWrite(seta_esquerda, LOW); // Garante que o LED esteja desligado
          break;

        default: // Comando desconhecido
          Serial.println("Comando desconhecido recebido.");
          break;
      }
    }
  }

  // Controle de piscar para a seta direita
  static unsigned long previousMillis_direita = 0;
  const unsigned long interval = 500; // Intervalo de 500ms
  if (piscar_seta_direita) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis_direita >= interval) {
      previousMillis_direita = currentMillis;
      // Alterna o estado do LED
      digitalWrite(seta_direita, !digitalRead(seta_direita));
    }
  }

  // Controle de piscar para a seta esquerda
  static unsigned long previousMillis_esquerda = 0;
  if (piscar_seta_esquerda) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis_esquerda >= interval) {
      previousMillis_esquerda = currentMillis;
      // Alterna o estado do LED
      digitalWrite(seta_esquerda, !digitalRead(seta_esquerda));
    }
  }
}
