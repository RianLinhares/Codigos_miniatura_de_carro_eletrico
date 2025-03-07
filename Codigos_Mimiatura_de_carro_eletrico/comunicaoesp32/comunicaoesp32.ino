#include <WiFi.h>
#include <PubSubClient.h>
#include "comunicacaoCAN.h" // Inclui o arquivo de cabeçalho


#define WHEEL_DIAMETER 0.065
#define WHEEL_RADIUS (WHEEL_DIAMETER / 2)


// Configuração Wi-Fi
const char* ssid = "";
const char* password = "";

// Configuração MQTT
const char* mqtt_server = "test.mosquitto.org";  // Pode usar outro broker público ou privado
const int mqtt_port = 1883;
const char* mqtt_topic = "car/control";
const char* mqtt_topic2 = "car/status";
WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis de controle dos LEDs
int count_a = 0, count_b = 0, count_x = 0, count_y = 0, count_ax = 0, count_rb = 0; 
String last_command = ""; // Armazena o último comando recebido

bool isBacking = false; // Variável para rastrear o estado do comando "Back"
unsigned long lastUpdateTime = 0, lastUpdateTime_velaverage = 0, lastUpdateTime_rpm = 0; // Armazena o tempo da última atualização


// Função para conectar ao Wi-Fi
void setup_wifi() {
    Serial.print("Conectando ao Wi-Fi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
}

void enviarComandoMQTT(String comando) {
    Serial.println("Enviando comando: " + comando);
    client.publish(mqtt_topic2, comando.c_str());
}

// Callback que processa as mensagens MQTT recebidas
void callback(char* topic, byte* payload, unsigned int length) {
    String command = "";
    for (unsigned int i = 0; i < length; i++) {
        command += (char)payload[i];
    }
    command.trim();
    Serial.print("Comando recebido via MQTT: ");
    Serial.println(command);

    if (command == "Luz_Baixa" && count_y != 1) {
        count_a += 1;
        if (count_a == 1) {
            enviarMensagem2(1);
            enviarComandoMQTT("lhb");
            Serial.println("Luz baixa ligada");
        } else if (count_a == 2) {
            enviarMensagem2(0);
            enviarComandoMQTT("dlhb");
            Serial.println("Luz baixa desligada");
            count_a = 0;
        }
    } else if (command == "Luz_Baixa" && count_y == 1) {
        enviarComandoMQTT("Operação inválida");
        Serial.println("Operação inválida");
    }

    if (command == "Luz_Alta" && count_a == 1) {
        count_y += 1;
        if (count_y == 1) {
            enviarMensagem2(2);
            enviarComandoMQTT("dlhb");
            delay(500);
            enviarComandoMQTT("lha");
            Serial.println("Luz alta ligada - Luz baixa desligada");
        } else if (count_y == 2) {
            enviarMensagem2(3);
            enviarComandoMQTT("dlha");
            delay(500);
            enviarComandoMQTT("lhb");
            Serial.println("Luz alta desligada - Luz baixa ligada");
            count_y = 0;
        }
    }

    if (command == "Seta_Direita") {
        count_b += 1;
        if (count_rb > 0) {
            enviarComandoMQTT("Operação inválida");
            Serial.println("Operação inválida");
            count_b = 0;
        } else if (count_b == 1) {
            if (count_x > 0) {
                enviarMensagem2(13);
                //enviarComandoMQTT("Seta Esquerda Desligada");
                Serial.println("Seta Esquerda Desligada");
                count_x = 0;
            }
            enviarComandoMQTT("sdl"); // Seta Direita Ligada
            enviarMensagem2(10);
            Serial.println("Seta Direita Ligada");
        } else if (count_b == 2) {
            enviarComandoMQTT("sdd"); // Seta Direita Desligada
            enviarMensagem2(11);
            Serial.println("Seta Direita Desligada");
            count_b = 0;
        }
    }

    if (command == "Seta_Esquerda") {
        count_x += 1;
        if (count_rb > 0) {
            enviarComandoMQTT("Operação inválida");
            Serial.println("Operação inválida");
            count_x = 0;
        } else if (count_x == 1) {
            if (count_b > 0) {
                enviarMensagem2(11);
                //enviarComandoMQTT("Seta Direita Desligada automaticamente");
                Serial.println("Seta Direita Desligada automaticamente");
                count_b = 0;
            }
            enviarComandoMQTT("sel"); //Seta Esquerda Ligada
            enviarMensagem2(12);
            Serial.println("Seta Esquerda Ligada");
        } else if (count_x == 2) {
            enviarComandoMQTT("sed"); //Seta Esquerda Desligada
            enviarMensagem2(13);
            Serial.println("Seta Esquerda Desligada");
            count_x = 0;
        }
    }

    if (command == "Pisca_Alerta") {
        count_rb += 1;
        if (count_rb == 1) {
            enviarComandoMQTT("pal"); //Pisca Alerta Ligado
            enviarMensagem2(14);
            Serial.println("Pisca Alerta Ligado");
        } else if (count_rb == 2) {
            enviarComandoMQTT("pad"); //Pisca Alerta Desligado
            enviarMensagem2(15);
            Serial.println("Pisca Alerta Desligado");
            count_x = 0;
            count_b = 0;
            count_rb = 0;
        }
    }

    if (command == "Forward") {
        delay(200);
        enviarMensagem(9, 255);
        enviarMensagem2(9);
        Serial.println("Movendo para frente");
    } 

    if (command == "Stop") {
        delay(200);
        enviarMensagem(5, 0);
        enviarMensagem2(5);
        Serial.println("Parando");
        isBacking = false; // Desativa o estado de "Back"
        enviarComandoMQTT("parar");
        if (last_command == "Right" && count_b > 0) {
            enviarMensagem2(11);
            Serial.println("Seta Direita Desligada após Stop");
            count_b = 0;
        }
        if (last_command == "Left" && count_x > 0) {
            enviarMensagem2(13);
            Serial.println("Seta Esquerda Desligada após Stop");
            count_x = 0;
        }
        last_command = "";
    }

    if (command == "Back") {
        delay(200);
        enviarMensagem(8, 255);
        enviarMensagem2(8);
        Serial.println("Movendo para trás");
        isBacking = true; // Ativa o estado de "Back"
    }

    if (command == "Right") {
        delay(200);
        enviarMensagem(6, 255);
        enviarMensagem2(6);
        Serial.println("Virando para a direita");
        last_command = "Right";
    }

    if (command == "Left") {
        delay(200);
        enviarMensagem(7, 255);
        enviarMensagem2(7);
        Serial.println("Virando para a esquerda");
        last_command = "Left";
    }
}

// Função para conectar ao broker MQTT
void reconnect() {
    while (!client.connected()) {
        Serial.print("Conectando ao broker MQTT...");
        if (client.connect("ESP32_Car_Controller")) {
            Serial.println("Conectado!");
            client.subscribe(mqtt_topic);
        } else {
            Serial.print("Falha, rc=");
            Serial.print(client.state());
            Serial.println(" Tentando novamente em 5 segundos...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
  
    inicializarCAN();
}

void loop() { 
if (!client.connected()) { 
  reconnect(); 
  } 
  client.loop(); // Recebe mensagens CAN e exibe o valor da distância no serial

unsigned long currentTime_rpm = millis();
    if (currentTime_rpm - lastUpdateTime_rpm >= 1000) { // Atualiza a cada 1 segundo
        lastUpdateTime_rpm = currentTime_rpm;
        int idMensagem_rpm;
        byte dados_rpm[4];
        float rpm = receberMensagemCAN(&idMensagem_rpm, dados_rpm);
        float velAverage_ms = ((2.0 * PI * rpm) / 60.0) * WHEEL_RADIUS;
        float velAverage_kh = velAverage_ms * 3.6;
        if (idMensagem_rpm == 768){
          Serial.print("ESTAMOS AQUI RPM");
          Serial.println(" ");
          enviarComandoMQTT(String(rpm) + " RPM");
          enviarComandoMQTT(String(velAverage_kh) + " km/h");
        }
    }

if (isBacking) {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= 1000) { // Atualiza a cada 1 segundo
        lastUpdateTime = currentTime;

        int idMensagem;
        byte dados[4];
        Serial.println(idMensagem);
        
          // Recebe a mensagem CAN e captura o valor da distância
          
          float distancia = receberMensagemCAN(&idMensagem, dados);
          Serial.print("Distancia");
          Serial.println(distancia);
          if (idMensagem == 769){
          // Verifica se a distância é válida e envia via MQTT
          if (distancia != -1) {
                Serial.print("Distância recebida: ");
                Serial.print(distancia);
                Serial.println(" cm");
                if (distancia >= 1 && distancia <= 7){
                  enviarComandoMQTT("Perto");

                }
                else if (distancia > 7 && distancia <= 14){
                  enviarComandoMQTT("Media_distancia");
                }
                else if (distancia > 14 && distancia <= 37 ){
                  enviarComandoMQTT("Distante");
                }
            
          } else {
              Serial.println("Nenhuma mensagem válida recebida.");
          } 

        }
}
} }