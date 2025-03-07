#include <SPI.h>
#include <mcp2515.h>
#include <HCSR04.h>
#include <PinChangeInterrupt.h>

// Pinos do MCP2515
#define CAN_CS 10
#define CAN_INT 2

// Inicializa o sensor ultrassônico
const byte triggerPin = 6;
const byte echoPin = 7;
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

// Inicializa o módulo MCP2515
MCP2515 mcp2515(CAN_CS);

// Definições para encoders
#define TEETH_COUNT 20
#define WHEEL_DIAMETER 0.065
#define WHEEL_RADIUS (WHEEL_DIAMETER / 2)
#define ENCODER_PIN_1 2
#define ENCODER_PIN_2 3
#define ENCODER_PIN_3 4 
#define ENCODER_PIN_4 5

// Variáveis voláteis para contagem de pulsos
volatile unsigned int pulseCount1 = 0, pulseCount2 = 0, pulseCount3 = 0, pulseCount4 = 0;

// Variáveis para armazenar o RPM e velocidade
float rpmAverage = 0.0, velAverage_ms = 0.0, velAverage_kh = 0.0;
float dist_cm;
unsigned long lastTime = 0;

// Funções de interrupção para encoders
void countPulse1() { pulseCount1++; }
void countPulse2() { pulseCount2++; }
void countPulse3() { pulseCount3++; }
void countPulse4() { pulseCount4++; }

void setup() {
    Serial.begin(115200);
    pinMode(CAN_INT, INPUT);
    
    // Inicializa CAN
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();
    Serial.println("CAN iniciado!");
    
    // Configura os encoders
    pinMode(ENCODER_PIN_1, INPUT_PULLUP);
    pinMode(ENCODER_PIN_2, INPUT_PULLUP);
    pinMode(ENCODER_PIN_3, INPUT_PULLUP);
    pinMode(ENCODER_PIN_4, INPUT_PULLUP);
    
    attachPinChangeInterrupt(digitalPinToPCINT(ENCODER_PIN_1), countPulse1, RISING);
    attachPinChangeInterrupt(digitalPinToPCINT(ENCODER_PIN_2), countPulse2, RISING);
    attachPinChangeInterrupt(digitalPinToPCINT(ENCODER_PIN_3), countPulse3, RISING);
    attachPinChangeInterrupt(digitalPinToPCINT(ENCODER_PIN_4), countPulse4, RISING);
    
    lastTime = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Mede a distância
    dist_cm = distanceSensor.measureDistanceCm();
    Serial.print("Distância: ");
    Serial.print(dist_cm);
    Serial.println(" cm");
    
    if (currentTime - lastTime >= 1000) {
        noInterrupts();
        unsigned int pulses1 = pulseCount1, pulses2 = pulseCount2, pulses3 = pulseCount3, pulses4 = pulseCount4;
        pulseCount1 = pulseCount2 = pulseCount3 = pulseCount4 = 0;
        interrupts();
        
        float rpm1 = ((float)pulses1 / TEETH_COUNT) * 60.0;
        float rpm2 = ((float)pulses2 / TEETH_COUNT) * 60.0;
        float rpm3 = ((float)pulses3 / TEETH_COUNT) * 60.0;
        float rpm4 = ((float)pulses4 / TEETH_COUNT) * 60.0;
        rpmAverage = (rpm1 + rpm2 + rpm3 + rpm4) / 4.0;
        velAverage_ms = ((2.0 * PI * rpmAverage) / 60.0) * WHEEL_RADIUS;
        velAverage_kh = velAverage_ms * 3.6;
        
        Serial.print("RPM1: ");
        Serial.print(rpm1);
        Serial.println(" ");
        Serial.print("RPM2: ");
        Serial.print(rpm2);
        Serial.println(" ");
        Serial.print("RPM3: ");
        Serial.print(rpm3);
        Serial.println(" ");
        Serial.print("RPM4: ");
        Serial.print(rpm4);
        Serial.println(" ");
        Serial.print("RPM Médio: "); Serial.print(rpmAverage);
        Serial.print(" | Velocidade Média: "); Serial.print(velAverage_ms, 3);
        Serial.print(" m/s | "); 
        Serial.print(velAverage_kh, 3); 
        Serial.println(" km/h");
        
        lastTime = currentTime;
    }
    
    // Envia dados via CAN
    struct can_frame canMsg;
    canMsg.can_dlc = 4;
    
    memcpy(canMsg.data, &dist_cm, 4);
    canMsg.can_id = 0x301;
    mcp2515.sendMessage(&canMsg);
    delay(1000);
    memcpy(canMsg.data, &rpmAverage, 4);
    canMsg.can_id = 0x300;
    mcp2515.sendMessage(&canMsg);
    delay(1000);
    //memcpy(canMsg.data, &velAverage_kh, 4);
    //canMsg.can_id = 0x300;
    //mcp2515.sendMessage(&canMsg);
    //delay(1000);
}
