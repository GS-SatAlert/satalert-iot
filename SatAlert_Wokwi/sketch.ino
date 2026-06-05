#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "6264bb59992c4a7ea9f762277e2f87a1.s1.eu.hivemq.cloud"; 
const int   mqtt_port = 8883; 
const char* mqtt_user = "SatAlertAdmin";
const char* mqtt_pass = "Satalert123abc.";

const char* topico_telemetria = "satalert/telemetria"; 
const char* topico_alertas = "satalert/alertas";
const char* topico_status = "satalert/status";

WiFiClientSecure espClient;
PubSubClient client(espClient);

#define PINO_DHT 15
#define TIPO_DHT DHT22
DHT dht(PINO_DHT, TIPO_DHT);

#define PINO_FUMO 34  
#define PINO_LED 2
#define PINO_BUZZER 4

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long tempoAnteriorTelemetria = 0;
unsigned long tempoAnteriorStatus = 0;

void setup_wifi() {
  lcd.clear();
  lcd.print("Conectando WiFi");
  Serial.print("Conectando ao Wi-Fi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Conectado!");
  Serial.println("\nWi-Fi Conectado!");
  delay(1000);
}

void reconnect() {
  while (!client.connected()) {
    lcd.clear();
    lcd.print("Conectando MQTT");
    Serial.print("Tentando conexao MQTT...");
    String clientId = "ESP32SatAlert-" + String(random(0, 1000));
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      lcd.clear();
      lcd.print("MQTT Conectado!");
      Serial.println("\nConectado!");
      delay(1000);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando em 5s...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  espClient.setInsecure();
  
  pinMode(PINO_LED, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
  dht.begin();
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  Serial.println("Sistema iniciado.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnteriorTelemetria >= 2000) {
    tempoAnteriorTelemetria = tempoAtual;

    float temperatura = dht.readTemperature();
    int valorFumoBruto = analogRead(PINO_FUMO);
    int nivelFumo = map(valorFumoBruto, 0, 4095, 0, 100); 

    Serial.print("Temperatura: "); Serial.print(temperatura);
    Serial.print(" | Nível de fumaça: "); Serial.println(nivelFumo);

    if (isnan(temperatura)) return;

    bool focoIncendio = (temperatura > 45 || nivelFumo > 60);
    
    digitalWrite(PINO_LED, focoIncendio ? HIGH : LOW);
    if(focoIncendio) tone(PINO_BUZZER, 100); else noTone(PINO_BUZZER);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(focoIncendio ? "ALERTA CRITICO!!" : "Estado: NORMAL");
    lcd.setCursor(0, 1);
    lcd.print(focoIncendio ? "FOGO DETECTADO!" : "Temp: " + String(temperatura, 1) + "C");

    StaticJsonDocument<200> docTelemetria;
    docTelemetria["temperatura"] = temperatura;
    docTelemetria["nivel_fumaca"] = nivelFumo;
    char buffer[200];
    serializeJson(docTelemetria, buffer);
    client.publish(topico_telemetria, buffer);

    StaticJsonDocument<200> docAlerta;
    docAlerta["foco_incendio"] = focoIncendio; 
    docAlerta["risco"] = focoIncendio ? "CRITICO - EVACUAR" : "NORMAL";

    char bufferAlerta[200];
    serializeJson(docAlerta, bufferAlerta);
    client.publish(topico_alertas, bufferAlerta);
  }

  if (tempoAtual - tempoAnteriorStatus >= 10000) {
    tempoAnteriorStatus = tempoAtual;
    StaticJsonDocument<200> docStatus;
    docStatus["dispositivo"] = "SatAlert_GroundStation_01";
    docStatus["status_conexao"] = "online";
    docStatus["uptime_seg"] = millis() / 1000;
    
    char bufferStatus[200];
    serializeJson(docStatus, bufferStatus);
    client.publish(topico_status, bufferStatus);
  }
}