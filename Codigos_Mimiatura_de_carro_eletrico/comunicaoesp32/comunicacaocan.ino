#include "comunicacaoCAN.h"

// Defina os pinos e o MCP2515
const int CS_PIN = 5;
MCP2515 mcp2515(CS_PIN);

void inicializarCAN() {
    Serial.begin(115200);

    // Inicializa o SPI com os pinos especificados
    SPI.begin(18, 19, 23, CS_PIN);

    // Reset do MCP2515
    mcp2515.reset();

    // Configura o bitrate e o cristal
    if (mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ) != MCP2515::ERROR_OK) {
        Serial.println("Erro: Configuração de bitrate falhou. Verifique o cristal e o circuito.");
        while (1);
    }

    // Coloca o MCP2515 em modo normal
    if (mcp2515.setNormalMode() != MCP2515::ERROR_OK) {
        Serial.println("Erro: MCP2515 não está no modo normal. Verifique as conexões SPI.");
        while (1);
    }

    Serial.println("MCP2515 inicializado com sucesso e em modo normal.");
}

void enviarMensagem(int comando, int dado) {
    uint16_t id = 0xAF;

    // Cria uma estrutura de mensagem CAN
    struct can_frame canMsg;
    canMsg.can_id = id;     // Define o ID da ECU powering
    canMsg.can_dlc = 2;     // Define o comprimento da mensagem (1 byte)
    canMsg.data[0] = comando;
    canMsg.data[1] = dado;
    
    // Envia a mensagem
    if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK) {
        Serial.print("Mensagem enviada com ID: 0x");
        Serial.println(id, HEX);
        Serial.print("Enviando dados: ");
        for (int i = 0; i < 2; i++) {
            Serial.print(canMsg.data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("Falha ao enviar mensagem.");
    }
}

void enviarMensagem2(uint16_t comando) {
    uint16_t id = 0x22F;
    // Cria uma estrutura de mensagem CAN
    struct can_frame canMsg;
    canMsg.can_id = id;     // Define o ID da ECU lighting
    canMsg.can_dlc = 1;     // Define o comprimento da mensagem (1 byte)
    canMsg.data[0] = comando;
    // Envia a mensagem
    if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK) {
        Serial.print("Mensagem enviada com ID: 0x");
        Serial.println(canMsg.can_id);
        Serial.print("Enviando dados: ");
        for (int i = 0; i < 2; i++) {
            Serial.print(canMsg.data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("Falha ao enviar mensagem.");
    }
}

float receberMensagemCAN(int* idMensagem, byte* dados) { struct can_frame canMsg;


// Verifica se há uma mensagem CAN recebida
if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    *idMensagem = canMsg.can_id;  // Atribui o ID da mensagem

    // Copia os dados recebidos para o array 'dados'
    for (int i = 0; i < canMsg.can_dlc; i++) {
        dados[i] = canMsg.data[i];
    }

    Serial.print("Mensagem recebida com ID: 0x");
    Serial.println(canMsg.can_id, HEX);
    Serial.print("Dados recebidos: ");
    for (int i = 0; i < canMsg.can_dlc; i++) {
        Serial.print(canMsg.data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Processar mensagem do sensor ultrassônico
    if (canMsg.can_id == 0x300 || canMsg.can_id == 0x301 || canMsg.can_id == 0x302 ) { // ID do sensor ultrassônico
  
        // Converte os 4 bytes recebidos em um valor float (distância em cm)
        float medida;
        memcpy(&medida, canMsg.data, sizeof(medida));
        
        return medida; // Retorna a medida recebida
    }
}

    Serial.println("Nenhuma mensagem válida recebida.");
    return 0; // Retorna -1 caso não haja mensagem ou não seja do ID esperado
}


