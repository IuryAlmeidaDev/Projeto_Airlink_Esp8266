#include <ESP8266WiFi.h>         // Biblioteca correta para ESP8266
#include <WiFiClientSecure.h>    // Para conexões HTTPS
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>

const char* ssid = "Iury";         // Nome da sua rede Wi-Fi
const char* password = "123456789";       // Senha da sua rede Wi-Fi
const uint16_t TRANSMIT_PIN = 5;         // Pino de transmissão IR

const char* apiHost = "projeto-airlink-api.onrender.com"; // URL da API
const int apiPort = 443;                          // Porta HTTPS

const uint64_t IR_ON = 0xB2BF20;
const uint64_t IR_OFF = 0xB27BE0;

IRsend irsend(TRANSMIT_PIN);
bool isAirConditionerOn = false;

void setup() {
    Serial.begin(115200);
    irsend.begin();
    
    // Conectar ao Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Conectando ao WiFi...");
    
    // Verifica a conexão até conectar
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println("\nConectado ao WiFi");
    Serial.println(WiFi.localIP()); // Exibe o IP local do ESP8266
}

void loop() {
    checkApiAirConditionerState(); // Verifica o estado do ar-condicionado na API
    delay(2000);                    // Espera 5 segundos antes de checar novamente
}

// Função para checar o estado do ar-condicionado na API via HTTPS
void checkApiAirConditionerState() {
    WiFiClientSecure client;
    client.setInsecure(); // Usa o SSL sem verificação do certificado (não recomendado para produção)

    // Conecte-se ao host da API na porta 443 (HTTPS)
    if (client.connect(apiHost, apiPort)) {
        client.print(String("GET /ac/state HTTP/1.1\r\n") +
                     "Host: " + apiHost + "\r\n" +
                     "Connection: close\r\n\r\n");

        delay(500); // Aguarda a resposta
        String response = client.readString(); // Lê a resposta da API

        Serial.println("Resposta da API: " + response); // Mostra a resposta no Serial Monitor
        
        // Verifica o estado do ar-condicionado na resposta JSON
        if (response.indexOf("\"state\":true") >= 0) {
            if (!isAirConditionerOn) {
                sendIrCommand(IR_ON);  // Função para enviar o código IR para ligar
                Serial.println("Ar-condicionado ligado pela API");
                isAirConditionerOn = true;
            }
        } else if (response.indexOf("\"state\":false") >= 0) {
            if (isAirConditionerOn) {
                sendIrCommand(IR_OFF);  // Função para enviar o código IR para desligar
                Serial.println("Ar-condicionado desligado pela API");
                isAirConditionerOn = false;
            }
        }
        
        client.stop(); // Fecha a conexão
    } else {
        Serial.println("Falha na conexão com a API");
    }
}

// Função para enviar o comando IR várias vezes para garantir a execução
void sendIrCommand(uint64_t command) {
    for (int i = 0; i < 3; i++) {  // Envia o comando 3 vezes
        irsend.sendCOOLIX(command, 24);  // Envia o comando IR
        delay(2000);                      // Aguardar 500 ms entre envios
    }
}
