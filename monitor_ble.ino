#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <NimBLEDevice.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN 26
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

#define BTN_TELA 33
#define BTN_RESET 25

#define LED_VERDE 5
#define LED_VERMELHO 18

#define LED_BI_VERDE 13
#define LED_BI_VERMELHO 19

#define SW1 16
#define SW2 4
#define SW3 2
#define SW4 15
//====================================================
// UUIDs BLE
//====================================================

#define ENV_SERVICE_UUID      "181A"

#define CONTROL_SERVICE_UUID  "8d8f0001-3b1a-4f91-bd2d-111111111111"

#define LED_CHAR_UUID         "8d8f0002-3b1a-4f91-bd2d-111111111111"

#define RGB_CHAR_UUID         "8d8f0003-3b1a-4f91-bd2d-111111111111"

#define RSSI_SERVICE_UUID     "8d8f0004-3b1a-4f91-bd2d-111111111111"

#define RSSI_CHAR_UUID        "8d8f0005-3b1a-4f91-bd2d-111111111111"

#define SENSOR_CHAR_UUID "8d8f1001-3b1a-4f91-bd2d-111111111111"

#define RESET_CHAR_UUID "8d8f0006-3b1a-4f91-bd2d-111111111111"

#define NOTIFY_COUNT_CHAR_UUID "8d8f0007-3b1a-4f91-bd2d-111111111111"

#define GRAPH_MODE_CHAR_UUID "8d8f0008-3b1a-4f91-bd2d-111111111111"

#define BICOLOR_CHAR_UUID "8d8f0009-3b1a-4f91-bd2d-111111111111"

int telaAtual = 0;

float temperatura = 0;
float umidade = 0;

float tempMin = 999;
float tempMax = -999;

float umidMin = 999;
float umidMax = -999;

bool estadoBotaoTelaAnterior = false;
bool estadoBotaoResetAnterior = false;

unsigned long ultimaLeitura = 0;
unsigned long ultimaTrocaTela = 0;
unsigned long pausaAte = 0;

NimBLECharacteristic* ledCharacteristic;

NimBLECharacteristic* resetCharacteristic;

NimBLECharacteristic* notifyCountCharacteristic;

NimBLECharacteristic* graphModeCharacteristic;

NimBLECharacteristic* bicolorCharacteristic;

uint8_t estadoLED = 0;

uint16_t atualConnHandle = BLE_HS_CONN_HANDLE_NONE;

bool controleBLEHabilitado = true;

bool estadoLedVerdeLocal = false;

bool estadoLedVermelhoLocal = false;

bool estadoSW4Anterior = false;

int rssiBLE = 0;

//====================================================
// BLE
//====================================================

NimBLEServer* pServer;

NimBLEService* envService;
NimBLEService* controlService;
NimBLEService* rssiService;

NimBLECharacteristic* sensorCharacteristic;
NimBLECharacteristic* rssiCharacteristic;

bool bleConectado = false;

uint16_t notificacoes = 0;

uint8_t modoGrafico = 0;   // 0 = Celsius | 1 = Fahrenheit

struct SensorPacket {

    float tempC;

    float tempF;

    float umidade;

};

void mostraTela();

class MyServerCallbacks : public NimBLEServerCallbacks {
   
    void onConnect(NimBLEServer* server,
                   NimBLEConnInfo& connInfo) override {

        bleConectado = true;
         atualConnHandle = connInfo.getConnHandle();
    
        // Intervalo mínimo = 50 ms (40 x 1,25 ms)
        // Intervalo máximo = 100 ms (80 x 1,25 ms)
        // Slave Latency = 0
        // Timeout = 2000 ms (200 x 10 ms)
        server->updateConnParams(
            connInfo.getConnHandle(),
            40,
            80,
            0,
            200
        );

        mostraTela();
    }

    void onDisconnect(NimBLEServer* server,
                      NimBLEConnInfo& connInfo,
                      int reason) override {

        bleConectado = false;
       atualConnHandle = BLE_HS_CONN_HANDLE_NONE;
        NimBLEDevice::startAdvertising();

        mostraTela();
    }

};

void atualizaHistorico() {

  if (temperatura < tempMin)
    tempMin = temperatura;

  if (temperatura > tempMax)
    tempMax = temperatura;

  if (umidade < umidMin)
    umidMin = umidade;

  if (umidade > umidMax)
    umidMax = umidade;
}

void mostraTela() {

  lcd.clear();

  switch (telaAtual) {

    case 0:

      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(temperatura, 1);
      lcd.print(" C");

      lcd.setCursor(0, 1);
      lcd.print("Umid:");
      lcd.print(umidade, 0);
      lcd.print("%");

      break;

    case 1:

      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print((temperatura * 1.8) + 32, 1);
      lcd.print(" F");

      lcd.setCursor(0, 1);
      lcd.print("Umid:");
      lcd.print(umidade, 0);
      lcd.print("%");

      break;

    case 2:

      lcd.setCursor(0, 0);
      lcd.print("TMin:");
      lcd.print(tempMin, 1);

      lcd.setCursor(0, 1);
      lcd.print("TMax:");
      lcd.print(tempMax, 1);

      break;

    case 3:

      lcd.setCursor(0, 0);
      lcd.print("UMin:");
      lcd.print(umidMin, 0);

      lcd.setCursor(0, 1);
      lcd.print("UMax:");
      lcd.print(umidMax, 0);

      break;

    case 4:

        lcd.setCursor(0,0);

        if(bleConectado)
            lcd.print("BLE:CONECTADO ");
        else
            lcd.print("BLE:ANUNCIANDO");

        lcd.setCursor(0,1);

        lcd.print("RSSI:");

        lcd.print(rssiBLE);

        lcd.print("dBm");
        break;
      }
}

class LedCallbacks : public NimBLECharacteristicCallbacks {

  void onWrite(NimBLECharacteristic *pCharacteristic,
               NimBLEConnInfo &connInfo) override {
  

  if (!controleBLEHabilitado) {

       Serial.println("Controle BLE bloqueado pelo SW1");

      return;
  }

  std::string value = pCharacteristic->getValue();

    if (value.length() == 1) {

      estadoLED = (uint8_t)value[0];

      Serial.print("Estado recebido: ");
      Serial.println(estadoLED);

      if (estadoLED == 0) {
        digitalWrite(LED_VERDE, LOW);
        digitalWrite(LED_VERMELHO, LOW);
      }

      if (estadoLED == 1) {
        digitalWrite(LED_VERDE, HIGH);
        digitalWrite(LED_VERMELHO, LOW);
      }

      if (estadoLED == 2) {
        digitalWrite(LED_VERDE, LOW);
        digitalWrite(LED_VERMELHO, HIGH);
      }

      if (estadoLED == 3) {
        digitalWrite(LED_VERDE, HIGH);
        digitalWrite(LED_VERMELHO, HIGH);
      }

      pCharacteristic->setValue((uint8_t*)&estadoLED, 1);
    }
  }
};

class BicolorCallbacks : public NimBLECharacteristicCallbacks {

  void onWrite(
      NimBLECharacteristic *pCharacteristic,
      NimBLEConnInfo &connInfo) override {

    std::string value = pCharacteristic->getValue();

    if (value.length() != 1) return;

    uint8_t estado = value[0];

    switch (estado) {

      case 0: // desligado
        digitalWrite(LED_BI_VERDE, LOW);
        digitalWrite(LED_BI_VERMELHO, LOW);
        break;

      case 1: // verde
        digitalWrite(LED_BI_VERDE, HIGH);
        digitalWrite(LED_BI_VERMELHO, LOW);
        break;

      case 2: // vermelho
        digitalWrite(LED_BI_VERDE, LOW);
        digitalWrite(LED_BI_VERMELHO, HIGH);
        break;

      case 3: // amarelo
        digitalWrite(LED_BI_VERDE, HIGH);
        digitalWrite(LED_BI_VERMELHO, HIGH);
        break;
    }

    Serial.print("Bicolor: ");
    Serial.println(estado);

    pCharacteristic->setValue(&estado, 1);
  }
};

class ResetCallbacks : public NimBLECharacteristicCallbacks {

  void onWrite(
      NimBLECharacteristic *pCharacteristic,
      NimBLEConnInfo &connInfo) override {

    tempMin = temperatura;
    tempMax = temperatura;

    umidMin = umidade;
    umidMax = umidade;

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("Historico");

    lcd.setCursor(0,1);
    lcd.print("Resetado!");

    delay(1000);

    mostraTela();

    Serial.println("===== RESET BLE =====");
    Serial.print("Temp atual: ");
    Serial.println(temperatura);

    Serial.print("TempMin: ");
    Serial.println(tempMin);

    Serial.print("TempMax: ");
    Serial.println(tempMax);

    Serial.print("UmidMin: ");
    Serial.println(umidMin);

    Serial.print("UmidMax: ");
    Serial.println(umidMax);

    mostraTela();
  }

};

void setup() {
Serial.begin(115200);
  Wire.begin(22, 23);

  lcd.init();
  lcd.backlight();

  dht.begin();

    //====================================================
    // BLE
    //====================================================

    NimBLEDevice::init("Monitor BLE");
    Serial.println("BLE iniciado");

    //==============================
    // Segurança BLE
    //==============================

    // Senha de pareamento
    NimBLEDevice::setSecurityPasskey(123456);

    // Exige autenticação + criptografia + proteção MITM
    NimBLEDevice::setSecurityAuth(
        true,   // Bonding
        true,   // MITM
        true    // Secure Connection
    );

    // ESP32 exibe a senha
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Serviço Environmental Sensing
    envService = pServer->createService("8d8f1000-3b1a-4f91-bd2d-111111111111");

    // Characteristic Dados Atuais
    sensorCharacteristic =
        envService->createCharacteristic(
            SENSOR_CHAR_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);

    notifyCountCharacteristic =
        envService->createCharacteristic(
            NOTIFY_COUNT_CHAR_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);
    
    graphModeCharacteristic =
    envService->createCharacteristic(
        GRAPH_MODE_CHAR_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::NOTIFY);
      
    graphModeCharacteristic->setValue(&modoGrafico, 1);
    
    envService->start();
    Serial.println("Environmental iniciado");
    controlService = pServer->createService(CONTROL_SERVICE_UUID);

    ledCharacteristic =
        controlService->createCharacteristic(
            LED_CHAR_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE);

    ledCharacteristic->setValue(&estadoLED, 1);
    ledCharacteristic->setCallbacks(new LedCallbacks());

    bicolorCharacteristic =
        controlService->createCharacteristic(
            BICOLOR_CHAR_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE);

    bicolorCharacteristic->setCallbacks(
    new BicolorCallbacks());

    resetCharacteristic =
    controlService->createCharacteristic(
        RESET_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);

    resetCharacteristic->setCallbacks(
    new ResetCallbacks());

    controlService->start();
    Serial.println("Control iniciado");

    // Advertising
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();

    advertising->setName("Monitor BLE");

    advertising->enableScanResponse(true);

    // Advertising Interval = 400 ms
    advertising->setMinInterval(640);
    advertising->setMaxInterval(640);

    advertising->addServiceUUID(CONTROL_SERVICE_UUID);
    advertising->addServiceUUID("8d8f1000-3b1a-4f91-bd2d-111111111111");

    advertising->start();

    Serial.print("Advertising: ");
    Serial.println(advertising->isAdvertising());

      pinMode(BTN_TELA, INPUT);
      pinMode(BTN_RESET, INPUT);

      pinMode(LED_VERDE, OUTPUT);
      pinMode(LED_VERMELHO, OUTPUT);

      pinMode(LED_BI_VERDE, OUTPUT);
      pinMode(LED_BI_VERMELHO, OUTPUT);

      digitalWrite(LED_BI_VERDE, LOW);
      digitalWrite(LED_BI_VERMELHO, LOW);

      pinMode(SW1, INPUT);
      pinMode(SW2, INPUT);
      pinMode(SW3, INPUT);
      pinMode(SW4, INPUT);

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Monitor BLE");

      lcd.setCursor(0, 1);
      lcd.print("Inicializando");

      delay(2000);

      mostraTela();
}

void loop() {
  if (millis() - ultimaLeitura >= 2000) {
    if (bleConectado) {
        //rssiBLE = NimBLEDevice::getPeerRSSI(atualConnHandle);
    }
    ultimaLeitura = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {

      temperatura = t;
      umidade = h;
      SensorPacket dados;

      dados.tempC = temperatura;
      dados.tempF = temperatura * 1.8 + 32;
      dados.umidade = umidade;

      sensorCharacteristic->setValue(
          (uint8_t*)&dados,
          sizeof(SensorPacket));

      if (bleConectado){
        sensorCharacteristic->notify();
      }

      notificacoes++; 

      Serial.print("Notificacoes = ");
      Serial.println(notificacoes);

      notifyCountCharacteristic->setValue(
          (uint8_t*)&notificacoes,
          sizeof(notificacoes));

      if (bleConectado) {
          notifyCountCharacteristic->notify();
      }

      notifyCountCharacteristic->setValue(
          (uint8_t*)&notificacoes,
          sizeof(notificacoes));

      if (bleConectado) {
          notifyCountCharacteristic->notify();
      }
      atualizaHistorico();

      mostraTela();
    }
  }

  bool leituraTela = digitalRead(BTN_TELA);

  if (leituraTela && !estadoBotaoTelaAnterior) {

    telaAtual++;

    if (telaAtual > 4)
      telaAtual = 0;

    // pausa o modo automático por 10 segundos
    pausaAte = millis() + 10000;

    mostraTela();
  }

  estadoBotaoTelaAnterior = leituraTela;

  bool leituraReset = digitalRead(BTN_RESET);

  if (leituraReset && !estadoBotaoResetAnterior) {

    tempMin = temperatura;
    tempMax = temperatura;

    umidMin = umidade;
    umidMax = umidade;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Historico");

    lcd.setCursor(0, 1);
    lcd.print("Resetado!");

    delay(1000);

    mostraTela();
  }

  estadoBotaoResetAnterior = leituraReset;

  bool leituraSW2 = digitalRead(SW2);

  if (leituraSW2 != estadoLedVerdeLocal) {

      estadoLedVerdeLocal = leituraSW2;

      digitalWrite(LED_VERDE, estadoLedVerdeLocal);

      Serial.print("SW2 -> LED Verde: ");
      Serial.println(estadoLedVerdeLocal ? "ON" : "OFF");
  }

  bool leituraSW3 = digitalRead(SW3);

  if (leituraSW3 != estadoLedVermelhoLocal) {

      estadoLedVermelhoLocal = leituraSW3;

      digitalWrite(LED_VERMELHO, estadoLedVermelhoLocal);

      Serial.print("SW3 -> LED Vermelho: ");
      Serial.println(estadoLedVermelhoLocal ? "ON" : "OFF");
  }

bool leituraSW4 = digitalRead(SW4);

if (leituraSW4 != estadoSW4Anterior) {

    estadoSW4Anterior = leituraSW4;

    modoGrafico = leituraSW4 ? 1 : 0;

    graphModeCharacteristic->setValue(&modoGrafico, 1);

    if (bleConectado) {
        graphModeCharacteristic->notify();
    }

    Serial.print("Modo do gráfico: ");

    if (modoGrafico == 0)
        Serial.println("Celsius");
    else
        Serial.println("Fahrenheit");
  }

  if (millis() > pausaAte) {

    if (millis() - ultimaTrocaTela >= 3000) {

      ultimaTrocaTela = millis();

      telaAtual++;

      if (telaAtual > 4)
        telaAtual = 0;

      mostraTela();
    }
  }
  controleBLEHabilitado = digitalRead(SW1);
}