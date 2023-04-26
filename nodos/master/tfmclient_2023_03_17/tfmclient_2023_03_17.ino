#include <Wire.h>
#include "METI2C.h"  //TODO: valores negativos de temperatura
#include <MCP342x.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"
#include <Crypto.h>
#include <AES.h>
#include <CBC.h>
#include <RNG.h>
#include <SHA256.h>
#include <string.h>
#include <TransistorNoiseSource.h>
#include <base64.hpp>
#include <XBee.h>
#include <ArduinoJson.h>
#include "ESP32HTTPS.h"
#include "TFMUtils.h"
#include "TFMCryptoLibrary.h"
#include "TFMProtocol.h"

#include <Adafruit_ADS1X15.h>     //para sustuir al mcp3424 dañado

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <Preferences.h>  //https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences

#include <Adafruit_BMP280.h>
#define PA_TO_ATM 101325
#define PA_TO_HPA 0.01
#define BMP_CORRECTION 1.4138

#define SIZE_PARAM 8
#define TIME_LISTENING 50   //tiempo de escucha del xbee (50ms)

#define PULLDATA_POOL_TO_SEND 3   //valor mínimo del pulldatapool
#define MAX_PULLDATA_TO_SEND 3    //valor máximo del pulldatapool (3 como máximo para evitar canary trigger (buffer overflow))

#define REQUEST_TIME_RATE 43200
#define RESET_RATE 86400
#define REQUEST_GET_RATE 60
#define SEND_PULLDATA_RATE 20
#define SERVER_TEST_RATE 300   //si se pierde conexión con el servidor comprueba cada 5 minutos


#define CONFIGURATION_LF '\n'
#define CONFIGURATION_CR '\r'
#define CONFIGURATION_LENGTH_MIN 3
#define CONFIGURATION_PRELUDE 0x54  //"T"
#define CONFIGURATION_PRELUDE_BYTE 0
#define CONFIGURATION_CODE_BYTE 1
#define CONFIGURATION_PAYLOAD_BEGIN_BYTE 2


#define CONFIGURATION_CODE_SSID 'A'
#define CONFIGURATION_CODE_SSID_PASSWORD 'B'
#define CONFIGURATION_CODE_NETWORK_ID 'C'
#define CONFIGURATION_CODE_NETWORK_KEY 'D'
#define CONFIGURATION_CODE_SAMPLERATE 'E'
#define CONFIGURATION_CODE_NODENAME 'F'
#define CONFIGURATION_CODE_NODEPASS 'G'
#define CONFIGURATION_CODE_AUTHNAME 'H'
#define CONFIGURATION_CODE_AUTHPASS 'I'
#define CONFIGURATION_CODE_GET_OFFSET 'J'
#define CONFIGURATION_CODE_CLEAR_SD 'K'


#define PREFERENCES_FILE "myfile"
#define PARAMETERS_CONFIGURED "p_conf"  //booleano para saber si se ha configurado el nodo
#define DEFAULT_CONFIGURED false
#define PARAMETERS_SSID "p_ssid"
#define DEFAULT_SSID ""
#define PARAMETERS_SSID_PASSWORD "p_ssid_pass"
#define DEFAULT_SSID_PASSWORD ""
#define PARAMETERS_NODENAME "p_nodename"
#define DEFAULT_NODENAME ""
#define PARAMETERS_NODEPASS "p_nodepass"
#define DEFAULT_NODEPASS ""
#define PARAMETERS_NETWORK_ID "p_network_id"
#define DEFAULT_NETWORK_ID 0
#define PARAMETERS_AUTHNAME "p_authname"
#define DEFAULT_AUTHNAME ""
#define PARAMETERS_AUTHPASS "p_authpass"
#define DEFAULT_AUTHPASS ""
#define PARAMETERS_SAMPLERATE "p_samplerate"
#define DEFAULT_SAMPLERATE 60
#define PARAMETERS_OUTPUTVALUE "p_outputvalue"
#define DEFAULT_OUTPUTVALUE false
#define PARAMETERS_ADC1_OFFSET1 "p_adc1_offset1"
#define PARAMETERS_ADC1_OFFSET2 "p_adc1_offset2"
#define PARAMETERS_ADC1_OFFSET3 "p_adc1_offset3"
#define PARAMETERS_ADC1_OFFSET4 "p_adc1_offset4"
#define PARAMETERS_ADC2_OFFSET1 "p_adc2_offset1"
#define PARAMETERS_ADC2_OFFSET2 "p_adc2_offset2"
#define PARAMETERS_ADC2_OFFSET3 "p_adc2_offset3"
#define PARAMETERS_ADC2_OFFSET4 "p_adc2_offset4"
#define DEFAULT_OFFSET 0

#define MAX_TIMES_ERROR 5
//variable utilizada para parchear el fallo de la hora. Cuando se deja la noche encendido se queda pillado para las 12 de la noche (puede ser por el serverTime pero no tiene sentido)
uint8_t time_error = 0;

//PINESa
const uint8_t CS_SD = 5;
const uint8_t ENABLE_SENSORS = 12;
const uint8_t DALLAS_TEMP = 33;
const uint8_t XBEE_SLEEP_RQ = 32;
const uint8_t GENERAL_OUTPUT = 14;
const uint8_t NOISE_SOURCE = 35;
const uint8_t CONFIGURATION_PIN = 25;
const uint8_t INFORMATION_LED = 26;
const uint8_t ERROR_LED = 27;

//Object
uint8_t adc_voltage_address = 0x69;
MCP342x adc_voltage = MCP342x(adc_voltage_address);   //está dañado y no funciona correctamente. Se añade un ADS1015 para sustituirlo
uint8_t adc_current_address = 0x6A;
MCP342x adc_current = MCP342x(adc_current_address);

Adafruit_ADS1115 ads_voltage;

METi2c met(METI2C_MASTER, 0, METI2C_DEFAULT_ADDRESS, true);
SDManager mySDManager = SDManager(true);

Preferences preferences;

OneWire oneWireDallas(DALLAS_TEMP);
DallasTemperature dallasTemp(&oneWireDallas);
RTC_DS1307 rtc;
TFMCrypto crypto;

XBee xbee = XBee();

PullDataPool pullDataPool;

NodeInfo me(NODE_MASTER);

NodeInfoPool nodes;

int adc1_offset[4] = { 0, 0, 0, 0 };
int adc2_offset[4] = { 0, 0, 0, 0 };

//the rootCA certificate of the destination server
char* rootCA =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/\n"
  "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
  "DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow\n"
  "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
  "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB\n"
  "AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC\n"
  "ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL\n"
  "wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D\n"
  "LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK\n"
  "4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5\n"
  "bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y\n"
  "sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ\n"
  "Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4\n"
  "FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc\n"
  "SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql\n"
  "PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND\n"
  "TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n"
  "SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1\n"
  "c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx\n"
  "+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB\n"
  "ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu\n"
  "b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E\n"
  "U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu\n"
  "MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC\n"
  "5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW\n"
  "9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG\n"
  "WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O\n"
  "he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC\n"
  "Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5\n"
  "-----END CERTIFICATE-----\n";

char* server = "migue27au2.ddns.net";
HTTPS https(server, HTTPS_PORT, rootCA, false);  //server, port, rootCA, debug

Blink blink(INFORMATION_LED, ERROR_LED);

String restCreate = "/rest/create";
String restTime = "/rest/time";
String restTest = "/rest/test";
String restGet = "/rest/get";
String restInfo = "/rest/info";

bool PARAMS_VALID = false;

String PARAM_NONCE = "";
String PARAM_HASH = "";
String PARAM_DATA = "";
uint8_t sp_randomNumber[RNG_SIZE];
uint8_t sp_hashedPass[HASH_SIZE];
uint8_t sp_hash[HASH_SIZE];
uint8_t sp_verification[HASH_SIZE];

const int REQUEST_TIME = 1000;

//AT commands
uint8_t niCmd[] = { 'n', 'i' };
uint8_t idCmd[] = { 'i', 'd' };
uint8_t kyCmd[] = { 'k', 'y' };
uint8_t wrCmd[] = { 'w', 'r' };
uint8_t acCmd[] = { 'a', 'c' };
uint8_t shCmd[] = { 's', 'h' };
uint8_t slCmd[] = { 's', 'l' };
uint8_t opCmd[] = { 'o', 'p' };   //utilizado para verificar que el xbee funciona

bool xbee_available = false;
bool server_available = false;
bool setting_up_device = true;    //para comprobar si el dispositivo está iniciando y solo escuchar paquetes xbee de AT_CMD_RESPONSE

Adafruit_BMP280 bmp;  // I2C
Adafruit_Sensor* bmp_pressure = bmp.getPressureSensor();

void setup() {
  //This is the beggining of a great story.
  blink.info(INFO_STARTING);

  Serial.begin(9600);

  Serial2.begin(9600);
  xbee.begin(Serial2);

  Wire.begin();  // join i2c bus (address optional for master)

  Serial.println(F("START"));

  //setting pines
  pinMode(ENABLE_SENSORS, OUTPUT);
  pinMode(XBEE_SLEEP_RQ, OUTPUT);
  pinMode(CONFIGURATION_PIN, INPUT);
  pinMode(GENERAL_OUTPUT, OUTPUT);

  met.begin();

  i2cScan();

  //SD EN ESP32
  if (!SD.begin(CS_SD)) {
    Serial.println(F("Card Mount Failed"));
    blink.error(ERROR_SD_MOUNT);
    delay(250);
    ESP.restart();
  } else {
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
      Serial.println(F("No SD card attached"));
    }
    Serial.print(F("SD Card Type: "));
    if (cardType == CARD_MMC) {
      Serial.println(F("MMC"));
    } else if (cardType == CARD_SD) {
      Serial.println(F("SDSC"));
    } else if (cardType == CARD_SDHC) {
      Serial.println(F("SDHC"));
    } else {
      Serial.println(F("UNKNOWN"));
    }
  }

  mySDManager.begin(&SD);
  delay(100);

  //if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    delay(250);
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_NONE,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X4,     /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X4,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */
  dallasTemp.begin();

  // Reset devices
  MCP342x::generalCallReset();
  delay(1);  // MC342x needs 300us to settle, wait 1ms

  preferences.begin("myfile", false);

  //check XBEE
  sendAtCommand(opCmd);
  delay(100);
  listenXBee();
  if(xbee_available == false){
    sendAtCommand(opCmd);
    delay(100);
    listenXBee();
  }
  if(xbee_available == false){
    Serial.println(F("XBee is not available"));
    blink.error(ERROR_XBEE_NOT_RUNNING);
    ESP.restart();
  }

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    ESP.restart();
  }
  while (!rtc.isrunning()) {
    Serial.println(F("RTC is NOT running. Setting time."));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    blink.error(ERROR_RTC_NOT_RUNNING);
    delay(250);
    ESP.restart();
  }

  //check RTC
  DateTime now = rtc.now();
  long time = now.unixtime();
  delay(3000);
  now = rtc.now();

  if(time == now.unixtime()){
    Serial.println(F("RTC is NOT running successfuly!"));
    blink.error(ERROR_RTC_NOT_RUNNING);
    delay(250);
    ESP.restart();    
  }

  //se inicia el ads del voltage
  if (!ads_voltage.begin(0x49)) {
    Serial.println(F("Failed to initialize ADS."));
    blink.error(ERROR_ADS);
    delay(250);
    ESP.restart();
  }

  checkConfigurationMode();
  if (preferences.getBool(PARAMETERS_CONFIGURED, DEFAULT_CONFIGURED) == false) {
    Serial.println(F("Node require configuration"));
    while (true) {
      blink.error(ERROR_NODE_NOT_CONFIGURED);
    }
  } else {
    readParameters();
  }

  Serial.print(F("Attempting to connect to SSID: "));
  Serial.println(me.getSSID());
  WiFi.begin(me.getSSID().c_str(), me.getSSIDPassword().c_str());

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    /*
      0: WL_IDLE_STATUS when Wi-Fi is changing state
      1: WL_NO_SSID_AVAIL if the configured SSID cannot be reached
      3: WL_CONNECTED after connection is established
      4: WL_CONNECT_FAILED if the password is incorrect
      6: WL_DISCONNECTED if the module is not configured in station mode
    */
    Serial.print(WiFi.status());
    counter++;
    if (counter >= 60) {  //after 30 seconds timeout - reset board
      blink.error(ERROR_NO_WIFI);
      ESP.restart();
    }
  }
  Serial.println();

  
  
  //clear XBEE_Pool
  Serial.println("Clearing data from XBEE");
  for(uint8_t i = 0; i < 255; i++){
    listenXBee();
  }

  //device ended of setting up
  Serial.println("Setting up finish");
  setting_up_device = false;

  blink.info(INFO_CONNECTED_TO_WIFI);

  Serial.println();
  Serial.print(F("Connected to "));
  Serial.println(me.getSSID());
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  https.begin();

  
  //Comprobar conexión al servidor
  Serial.println(F("Checking if server is available..."));
  server_available = serverTest();
  if(server_available == false){
    Serial.println("Cannot connect to server");
    blink.error(ERROR_SERVER_NOT_AVAILABLE);
    delay(250);
    ESP.restart();
  }

  Serial.println("Can connect to server");

  Serial.println(F("Generating seed..."));
  if(crypto.pseudoRandomNumberGenerateSeed()){
    Serial.println(F("Seed generated"));
  } else {
    Serial.println(F("Not enogth noise"));
  }

  unsigned int timeResponse = 0;
  uint8_t timeCont = 0;
  while (timeResponse == 0 && timeCont < 2) {  //dos intentos para poner bien la hora
    timeCont++;
    //getTimeFromServer
    Serial.println(F("Requesting time..."));

    getSecureParams("[" + me.getNodeName() + "]");
    timeResponse = serverTime();
  }

  //envío la información de este nodo
  //securizar estos parámetros
  getSecureParams("[" + me.toJSON() + "]");
  serverInfo();  
}

void checkConfigurationMode() {
  //modo de configuración
  if (digitalRead(CONFIGURATION_PIN)) {
    Serial.println(F("CONFIGURATION MODE ON"));
    setting_up_device = true;
  }
  while (digitalRead(CONFIGURATION_PIN)) {
    digitalWrite(ERROR_LED, HIGH);  //console mode on
    configurationMode();

    if (digitalRead(CONFIGURATION_PIN) == false) {
      Serial.println(F("CONFIGURATION MODE OFF"));
      digitalWrite(ERROR_LED, LOW);  //console mode off
      readParameters();
      setting_up_device = false;
      ESP.restart();  //reinicio el ESP
    }
  }
}

unsigned long getSeconds(){
  return millis()/1000;
}

void loop() {
  
  checkConfigurationMode();

  //enchufo el led si es necesario.
  digitalWrite(GENERAL_OUTPUT, me.getOutputValue());

  unsigned long time_in_seconds = getSeconds();

  bool must_server_get = false, must_server_time = false, must_read_data = false, must_server_create = false;

  //si el pulldataPool no está lleno y el SD tiene pulldatas. Lo relleno con los pulldatas
  while (pullDataPool.isFull() == false && mySDManager.isEmpty() == false){
    Serial.println(F("Getting pulldata from SD"));   
    PullData pulldata_from_sd;
    pulldata_from_sd.fromSD(mySDManager.getLastPullDataFromSD());
    //delay(10); //delay para no saturar

    //verificación rápida del pulldata (no está vacío). la verificación de integridad por corrupción de datos se realiza en el servidor
    if(pulldata_from_sd.verifyFromSD() == true){
      pullDataPool.push(pulldata_from_sd);      
    }
    mySDManager.deleteLastPullDataFromSD();
    //delay(50); //delay para no saturar
  }

  unsigned long milliseconds = millis();    

  //cuando se desborda el valor de millis() approximately 50 days o cuando el tiempo de vida es mayor al máximo
  if(crypto.getSeedLifetime() > milliseconds || (milliseconds - crypto.getSeedLifetime()) > MAX_SEED_LIFETIME){
    Serial.println(F("Generating new seed..."));
    if(crypto.pseudoRandomNumberGenerateSeed()){
      Serial.println(F("Seed generated"));
    } else {
      Serial.println(F("Not enogth noise"));
    }
  }
  
  if (time_in_seconds % RESET_RATE == 0){
    Serial.println(F("Reset ESP32"));
    blink.error(INFO_REBOOTING_ESP32);
  
    Serial.println(F("Saving pulldataPool in SD"));
    //vacío el pulldataPool en la SD
    while (pullDataPool.isEmpty() == false){
      PullData pulldata_to_sd = pullDataPool.pop();
      delay(10); //delay para no saturar
      mySDManager.newPullDataInSD(pulldata_to_sd.toSD());
    }
    
    Serial.println(F("ESP32 RESET"));
    ESP.restart();  //reinicio el ESP32 para volver a coger señal de reloj
  }

  if (time_in_seconds % REQUEST_GET_RATE == 0) {
    must_server_get = true;
  }

  if (time_in_seconds % me.getSampleRate() == 0) {
    must_read_data = true;
  }

  if (time_in_seconds % REQUEST_TIME_RATE == 0) {
    must_server_time = true;
  }
  if(pullDataPool.count() >= PULLDATA_POOL_TO_SEND && time_in_seconds % SEND_PULLDATA_RATE == 0) {
    must_server_create = true;
  }

  if(must_read_data){
    Serial.println(F("READING SENSORS..."));
    readData();
  }

  if(server_available == false && time_in_seconds % SERVER_TEST_RATE == 0){
    //Comprobar conexión al servidor
    Serial.println(F("Checking if server is available..."));
    server_available = serverTest();
  }

  //acciones a realizar contra el servidor
  if(server_available){
    if(must_server_time){
      unsigned int timeResponse = 0;
      uint8_t timeCont = 0;
      //cinco intentos como mucho
      while (timeResponse == 0 && timeCont < 5) {
        timeCont++;
        //getTimeFromServer
        Serial.println(F("Requesting time..."));

        getSecureParams("[" + me.getNodeName() + "]");
        timeResponse = serverTime();
      }
    }
    if(must_server_get){
      Serial.println(F("Requesting get..."));

      String nodenames_json = "[" + me.getNodeNameJSON();

      for(uint8_t node_index = 0; node_index < nodes.count(); node_index++){
        nodenames_json += "," + nodes.nodeInfoAt(node_index).getNodeNameJSON();
      }
      nodenames_json += "]";

      Serial.print("node names json: ");
      Serial.println(nodenames_json);

      getSecureParams(nodenames_json);
      serverGet();
    }
    
    if(must_server_create){
      Serial.println(F("Server create..."));     

      getSecureParams(getPullDataPool());
      unsigned int responseCode = serverCreate();

      //Si hay errores (400) en el envío de datos no borro el pulldataPool para un posterior envío
      if (responseCode == 0 || responseCode == HTTP_RESPONSE_BAD_REQUEST) {
        Serial.print(F("Error sending pulldatapool. Pulldatapool size: "));
        Serial.println(pullDataPool.count());
      } else {
        //el resto de códigos están gestionados por el servidor y por tanto debería limpiarse la pila de datos
        pullDataPoolSendedOk();
        Serial.println(F("PullDataPool sended, deleting items..."));
      }
    }
  }
  
  //Finalmente consulta el XBee
  listenXBee();

  Serial.printf("%d\r" ,time_in_seconds);
}

//TODO: problema, a la hora de no enviar paquetes se desconecta del wifi y se devuelve 1. Puede ser problema del ap
//cuando se reinicia vuelve a conectarse
void connectToWiFi() {
  uint8_t counter1 = 0;
  uint8_t counter2 = 0;
  while (WiFi.status() != WL_CONNECTED && counter1 < 5) {
    WiFi.disconnect();
    delay(5000);
    WiFi.disconnect();
    delay(5000);
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(me.getSSID());
    WiFi.begin(me.getSSID().c_str(), me.getSSIDPassword().c_str());
    counter2 = 0;
    while (WiFi.status() != WL_CONNECTED && counter2 < 60) {
      delay(500);
      /*
        0: WL_IDLE_STATUS when Wi-Fi is changing state
        1: WL_NO_SSID_AVAIL if the configured SSID cannot be reached
        3: WL_CONNECTED after connection is established
        4: WL_CONNECT_FAILED if the password is incorrect
        6: WL_DISCONNECTED if the module is not configured in station mode
      */
      Serial.print(WiFi.status());
      counter2++;
    }
    Serial.println();
    counter1++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    blink.info(INFO_CONNECTED_TO_WIFI);
  } else {
    blink.error(ERROR_NO_WIFI);

    Serial.println(F("Saving pulldataPool in SD"));
    //vacío el pulldataPool en la SD
    while (pullDataPool.isEmpty() == false){
      PullData pulldata_to_sd = pullDataPool.pop();
      delay(10); //delay para no saturar
      mySDManager.newPullDataInSD(pulldata_to_sd.toSD());
    }
    
    Serial.println(F("ESP32 RESET"));

    ESP.restart();
  }
}

bool checkRTC(long time){
  //cuando hayan ocurrido más de 5 errores seguidos
  if(time_error >= MAX_TIMES_ERROR){
    Serial.println(F("Too many time errors."));
    rtc.begin();  //Inicializamos el RTC
    delay(10);
    //si el RTC no está funcionando o si se juntan más de 10 errores de tiempo 
    if (!rtc.isrunning() || time_error >= (2*MAX_TIMES_ERROR)) {
      Serial.println(F("RTC is NOT running!"));
      blink.error(ERROR_RTC_NOT_RUNNING);
    
      Serial.println(F("Saving pulldataPool in SD"));
      //vacío el pulldataPool en la SD
      while (pullDataPool.isEmpty() == false){
        PullData pulldata_to_sd = pullDataPool.pop();
        delay(10); //delay para no saturar
        mySDManager.newPullDataInSD(pulldata_to_sd.toSD());
      }
      
      Serial.println(F("ESP32 RESET"));
      ESP.restart();  //reinicio el ESP32 para volver a coger señal de reloj
    } else {
      Serial.println(F("RTC is running"));
    } 
  }

  //si hay error en el time
  if(time < 1628500000 || time > 2543941748){
    Serial.printf("Error in time: %d\n", time);
    time_error++;
    return false;
  } else{
    time_error = 0;
    return true;
  }
}

void i2cScan() {
  int nDevices = 0;

  Serial.println(F("Scanning..."));

  for (byte address = 1; address < 127; ++address) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16) {
        Serial.print(F("0"));
      }
      Serial.print(address, HEX);
      Serial.println(F("  !"));

      ++nDevices;
    } else if (error == 4) {
      Serial.print(F("Unknown error at address 0x"));
      if (address < 16) {
        Serial.print(F("0"));
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println(F("No I2C devices found\n"));
    delay(250);
    ESP.restart();
  } else {
    Serial.println(F("done\n"));
  }

  //si no se encuentran todos los dispositivos
  if (nDevices < 5) {
    blink.error(ERROR_I2C_DEVICES);
  }
}

void getSecureParams(String data) {
  crypto.pseudoRandomNumberGenerator(sp_randomNumber, RNG_SIZE);

  PARAM_NONCE = hexToString(sp_randomNumber, RNG_SIZE, false);

  crypto.sha256Hash(me.getAuthPass().c_str(), me.getAuthPass().length(), sp_hashedPass, HASH_SIZE);

  //Serial.print(F("PASS: "));
  //Serial.println(authPassCharArray);

  //Serial.print(F("HASHED PASS: "));
  String hashedPassString = hexToString(sp_hashedPass, HASH_SIZE, true);
  //Serial.println(hashedPassString);

  String text = hashedPassString + PARAM_NONCE;

  //Serial.print(F("text: "));
  //for(int i = 0; i < textLength; i++){
  //  Serial.print((char)text[i]);
  // }
  //Serial.println();


  crypto.sha256Hash(text.c_str(), text.length(), sp_hash, HASH_SIZE);
  PARAM_HASH = hexToString(sp_hash, HASH_SIZE, true);

  PARAM_DATA = crypto.encryptAesCBC(hashedPassString, PARAM_NONCE, data);
  PARAMS_VALID = true;

  text = "";
  hashedPassString = "";
  data = "";
  //Serial.println();
  //Serial.print(F("PARAM_AUTH_NAME: "));
  //Serial.println(me.getAuthName());
  //Serial.print(F("DATA: "));
  //Serial.println(data);
  //Serial.print(F("PARAM_NONCE: "));
  //Serial.println(PARAM_NONCE);
  //Serial.print(F("PARAM_HASH: "));
  //Serial.println(PARAM_HASH);
  //Serial.print(F("text: "));
  //Serial.println(text);
  //Serial.print(F("textLength: "));
  //Serial.println(textLength);
}


void readPressure() {
  //TODO: añadir un limitador
  bool value = true;

  uint8_t tries = 0;

  //solución temporal para evitar los 731 hpa, si es menor a 800 vuelve a pedirlo
  met.setPressure(0.0);
  while((met.getPressure() < 800 || met.getPressure() > 1100) && tries < 5){
    for (int i = 0; i < 3; i++) {
      value = !value;
      if (value) {
        met.enableSensors();
      } else {
        met.disableSensors();
      }
      bmp.setSampling(Adafruit_BMP280::MODE_FORCED,   /* Operating Mode. */
                      Adafruit_BMP280::SAMPLING_NONE,   /* Temp. oversampling */
                      Adafruit_BMP280::SAMPLING_X4,   /* Pressure oversampling */
                      Adafruit_BMP280::FILTER_X4,    /* Filtering. */
                      Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */
      Serial.print(value);
      Serial.print(F(" "));
      met.setPressure(bmp.readPressure() * PA_TO_HPA);
      
      delay(100);
    }
    met.disableSensors();
  }
}

void readData() {
  blink.info(INFO_READING_DATA);

  bool metEnable = false;
  float batTemperature = 0.0, pvTemperature = 0.0, pressure = 0.0;

  if (met.checkConnection()) {
    metEnable = true;
    Serial.println(F("Met is on"));
  } else {
    Serial.println(F("Met don't response ok, trying again..."));
    if (met.checkConnection()) {
      metEnable = true;
      Serial.println(F("Met is on"));

      blink.error(ERROR_MET_NOT_AVAILABLE);
    } else {
      Serial.println(F("Met is off"));
    }
  }

  //enabling sensors
  if (metEnable) {
    met.enableSensors();
  }
  digitalWrite(ENABLE_SENSORS, HIGH);
  dallasTemp.requestTemperatures();

  Serial.println(F("Reading current..."));
  long value_current[4] = {0,0,0,0};

  for (int i = 0; i < 4; i++) {
    uint8_t err = 0;
    MCP342x::Config status;

    switch (i) {
      case 0:
        err = adc_current.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[0], status);
        break;
      case 1:
        err = adc_current.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[1], status);
        break;
      case 2:
        err = adc_current.convertAndRead(MCP342x::channel3, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[2], status);
        break;
      case 3:
        err = adc_current.convertAndRead(MCP342x::channel4, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[3], status);
        break;
    }

    if (err) {
      Serial.print(F("Convert error: "));
      Serial.println(err);
      blink.error(ERROR_CONVERSION_CURRENT);
    }

    delay(20);
  }

  

  Serial.println(F("Reading voltage..."));
  int16_t value_voltage[4] = {0,0,0,0};
  /* EL MCP3424 del voltage está estropeado
  long value_voltage[4];
  for (int i = 0; i < 4; i++) {
    uint8_t err = 0;
    MCP342x::Config status;
    switch (i) {
      case 0:
        err = adc_voltage.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[0], status);
        break;
      case 1:
        err = adc_voltage.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[1], status);
        break;
      case 2:
        err = adc_voltage.convertAndRead(MCP342x::channel3, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[2], status);
        break;
      case 3:
        err = adc_voltage.convertAndRead(MCP342x::channel4, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[3], status);
        break;
    }

    if (err) {
      Serial.print(F("Convert error: "));
      Serial.println(err);
      blink.error(ERROR_CONVERSION_VOLTAGE);
    }

    delay(20);
  } */

  //hay mucho ruido en la medición de los ads, lo que hago es coger la media de 5 mediciones  
  for (uint8_t meassure_num = 0; meassure_num < 5; meassure_num++){
    for (uint8_t i = 0; i < 4; i++) {
      value_voltage[i] += ((int)ads_voltage.readADC_SingleEnded(i)/5);
      delay(10);
    }
  }

  //getting metdata and disabling sensors...
  digitalWrite(ENABLE_SENSORS, LOW);
  if (metEnable) {
    met.clear();
    met.getMeasures();
    delay(100);
    //met.disableSensors();
    //delay(100);
    met.resetWindAndRain();
    delay(100);
    readPressure();
    //met.setPressure(bmp.readPressure() * PA_TO_HPA);

  }

  PullData pd;

  pd.setBatteryTemperature(dallasTemp.getTempCByIndex(0));
  pd.setPhotovoltaicTemperature(dallasTemp.getTempCByIndex(1));

  pd.setRain(met.getRain());
  pd.setWind(met.getWind());
  pd.setTemperature(met.getTemperature());
  pd.setHumidity(met.getHumidity());
  pd.setPressure(met.getPressure());  //cambiado al master

  pd.setLoadCurrent((int)value_current[0] - adc1_offset[0]);
  pd.setBatteryCurrent((int)value_current[1] - adc1_offset[1]);
  pd.setPhotovoltaicCurrent((int)value_current[2] - adc1_offset[2]);

  pd.setIrradiance((int)value_current[3] - adc1_offset[3]);

  //pd.setLoadVoltage((int)value_voltage[0] - adc2_offset[0]);
  //pd.setBatteryVoltage((int)value_voltage[1] - adc2_offset[1]);
  //pd.setPhotovoltaicVoltage((int)value_voltage[2] - adc2_offset[2]);
  pd.setLoadVoltage((int)value_voltage[0] - adc2_offset[0]);
  pd.setBatteryVoltage((int)value_voltage[1] - adc2_offset[1]);
  pd.setPhotovoltaicVoltage((int)value_voltage[2] - adc2_offset[2]);

  pd.setOutput(me.getOutputValue());

  Serial.print(F("Rain: "));
  Serial.println(pd.getRain(), 5);
  Serial.print(F("Wind: "));
  Serial.println(pd.getWind(), 5);
  Serial.print(F("Temperature: "));
  Serial.println(pd.getTemperature(), 5);
  Serial.print(F("Humidity: "));
  Serial.println(pd.getHumidity(), 5);
  Serial.print(F("Pressure: "));
  Serial.println(pd.getPressure(), 5);
  Serial.print(F("Irradiance: "));
  Serial.println(pd.getIrradiance(), 5);
  Serial.println();

  Serial.print(F("batTemperature: "));
  Serial.println(pd.getBatteryTemperature(), 5);
  Serial.print(F("pvTemperature: "));
  Serial.println(pd.getPhotovoltaicTemperature(), 5);
  Serial.println();

  Serial.printf("Value current %d: %d \t offset: %d\tload\n", 0, value_current[0],  value_current[0]-adc1_offset[0]);
  Serial.printf("Value current %d: %d \t offset: %d\tbattery\n", 1, value_current[1],  value_current[1]-adc1_offset[1]);
  Serial.printf("Value current %d: %d \t offset: %d\tphotovoltaic\n", 2, value_current[2],  value_current[2]-adc1_offset[2]);
  Serial.printf("Value current %d: %d \t offset: %d\tirradiance\n", 3, value_current[3],  value_current[3]-adc1_offset[3]);
  
  Serial.print(F("loadCurrent: "));
  Serial.println(pd.getLoadCurrent(), 5);
  Serial.print(F("batteryCurrent: "));
  Serial.println(pd.getBatteryCurrent(), 5);
  Serial.print(F("photovoltaicCurrent: "));
  Serial.println(pd.getPhotovoltaicCurrent(), 5);

  Serial.printf("Value voltage %d: %d \t offset: %d\tload\n", 0, value_voltage[0],  value_voltage[0]-adc2_offset[0]);
  Serial.printf("Value voltage %d: %d \t offset: %d\tbattery\n", 1, value_voltage[1],  value_voltage[1]-adc2_offset[1]);
  Serial.printf("Value voltage %d: %d \t offset: %d\tphotovoltaic\n", 2, value_voltage[2],  value_voltage[2]-adc2_offset[2]);
  Serial.printf("Value voltage %d: %d \t offset: %d\n", 3, value_voltage[3],  value_voltage[3]-adc2_offset[3]);

  Serial.print(F("loadVoltage: "));
  Serial.println(pd.getLoadVoltage(), 5);
  Serial.print(F("batteryVoltage: "));
  Serial.println(pd.getBatteryVoltage(), 5);
  Serial.print(F("photovoltaicVoltage: "));
  Serial.println(pd.getPhotovoltaicVoltage(), 5);
  Serial.println();

  Serial.print(F("output: "));
  Serial.println(pd.getOutput());
  Serial.println();

  if(addSecureParametersToPullData(&pd, me)){
    if(pullDataPool.push(pd) == false){
      Serial.println(F("PullDataPool is full. Sending pulldata to SD"));
      //blink.error(ERROR_PULLDATAPOOL_FULL);
      blink.info(INFO_PULLDATA_TO_SD);
    
      mySDManager.newPullDataInSD(pd.toSD());
    } else {
      Serial.println(F("PullData added"));
    }
  } else {
    Serial.println(F("Error setting secure parameters to pulldata"));
  }
}

bool addSecureParametersToPullData(PullData* pd, NodeInfo nodeInfo) {
  delay(20);
  DateTime now = rtc.now();
  long time = now.unixtime();
  if(checkRTC(time) == false){
    delay(100);
    now = rtc.now();    
    time = now.unixtime();
  }
  if(checkRTC(time)){
    
    crypto.pseudoRandomNumberGenerator(sp_randomNumber, RNG_SIZE);
    String hexNonce = hexToString(sp_randomNumber, RNG_SIZE, false);
    
    crypto.sha256Hash(nodeInfo.getNodePass().c_str(), nodeInfo.getNodePass().length(), sp_hashedPass, HASH_SIZE);
    String hashedPassString = hexToString(sp_hashedPass, HASH_SIZE, true);
    
    String text = hashedPassString + hexNonce;
    crypto.sha256Hash(text.c_str(), text.length(), sp_hash, HASH_SIZE);

    String hexHash = hexToString(sp_hash, HASH_SIZE, true);

    pd->setTime(time);

    pd->setNodeName(nodeInfo.getNodeName());
    pd->setHexNonce(hexNonce);
    pd->setHexHash(hexHash);


    Serial.print(F("time: "));
    Serial.println(pd->getTime());
    Serial.print(F("nodeName: "));
    Serial.println(nodeInfo.getNodeName());
    Serial.print(F("hexNonce: "));
    Serial.println(hexNonce);
    Serial.print(F("hexHash: "));
    Serial.println(hexHash);

    Serial.println("debug 1.2"); //delay(10);
    //TODO: mete excepcion: Guru Meditation Error: Core  1 panic'ed (StoreProhibited). Exception was unhandled.
    String json = "";
    pd->toJSONPointer(&json);  //before verification
    //Serial.print(F("Before Verification JSON -> "));
    
    Serial.println("debug 1.3"); //delay(10);
    crypto.sha256Hash(json.c_str(), json.length(), sp_verification, HASH_SIZE);
    Serial.println("debug 1.4"); //delay(10);
    String hexVerification = hexToString(sp_verification, HASH_SIZE, true);
    Serial.println("debug 1.5"); //delay(10);
    pd->setVerification(hexVerification);
    
    //after verification
    //Serial.print(F("After Verification json -> "));
    //Serial.println(pd->toJSON());

    hexVerification = "";
    json = "";
    hexHash = "";
    hexNonce = "";
    hashedPassString = "";

    //delete [] pd;
    
    return true;
  } else {
    Serial.println(F("Time error"));

    return false;
  }
}

//TODO: Obten datos de la SD
String getPullDataPool() {
  String json = "[";

  for (int i = 0; i < MAX_PULLDATA_TO_SEND; i++) {
    if (!pullDataPool.isEmpty()) {
      json += pullDataPool.pullDataAt(i).toJSON();
      if (i < MAX_PULLDATA_TO_SEND - 1) {
        json += ",";
      }
    } else {
      break;
    }
  }

  json += "]";

  //Serial.print(F("pullDataPool: "));
  //Serial.println(json);

  return json;
}

void pullDataPoolSendedOk() {
  Serial.print(F("Size before deleting: "));
  Serial.println(pullDataPool.count());

  for (int i = 0; i < MAX_PULLDATA_TO_SEND; i++) {
    if (!pullDataPool.isEmpty()) {
      //Serial.print(F("Deleting pulldata: "));
      //Serial.println(pullDataPool.popFirst().toJSON());
      pullDataPool.popFirst();
    } else {
      break;
    }
  }

  Serial.print(F("Size after deleting: "));
  Serial.println(pullDataPool.count());
}

String buildPayload() {
  String payload = "";
  payload += "{";
  payload += "\"authName\":\"";
  payload += me.getAuthName();
  payload += "\",\"nonce\":\"";
  payload += PARAM_NONCE;
  payload += "\",\"hash\":\"";
  payload += PARAM_HASH;
  payload += "\",\"data\":\"";
  payload += PARAM_DATA;
  payload += "\"}";

  return payload;
}

//----------------------- XBEE -----------------------------------------
//------------------------LISTEN XBEE-----------------------------------
int listenXBee() {
  //Serial.println(F("XBEE> Listening..."));
  if (xbee.readPacket(TIME_LISTENING)) {

    blink.info(INFO_ZIGBEE_PACKET_RECEIVED);
    Serial.print(F("Got a response. ApiId = "));
    Serial.println(xbee.getResponse().getApiId(), HEX);
    //si el dispositivo se está iniciando escuchar solo AT RESPONSES
    if(setting_up_device && xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE){
        blink.info(INFO_ZB_ATCMD_RESPONSE);
        receiveAtCommand();
    } else{
      switch (xbee.getResponse().getApiId()) {
        case AT_COMMAND_RESPONSE:
          blink.info(INFO_ZB_ATCMD_RESPONSE);
          receiveAtCommand();
          break;
        case ZB_RX_RESPONSE:
          /*if (readingParameters) {
            Serial.println(F("Wrong response"));
            return 2;
          }*/
          blink.info(INFO_ZB_RX_RESPONSE);
          receiveRXResponse();
          break;
        case ZB_TX_STATUS_RESPONSE:
          //TODO: gestionar paquetes de estados (Ej. no existe dirección)
          /*if (readingParameters) {
            Serial.println(F("Wrong response"));
            return 2;
          }*/
          blink.info(INFO_ZB_TXSTAT_RESPONSE);
          receiveTxStatusResponse();
          break;
        case MODEM_STATUS_RESPONSE:
          /*if (readingParameters) {
            Serial.println(F("Wrong response"));
            return 2;
          }*/
          blink.info(INFO_ZB_MODEMSTAT_RESPONSE);
          receiveModemStatus();
          break;
      }
    }
    return 1;
  } else if (xbee.getResponse().isError()) {
    //TODO: gestionar errores
    Serial.print(F("Error reading packet.  Error code: "));
    Serial.println(xbee.getResponse().getErrorCode());
    blink.error(ERROR_ZIGBEE_ERROR);

    return -1;
  } else {
    return 0;
  }
}

//----------------SEND FRAME------------------------
void sendFrame(Frame f, NodeInfo n) {
  XBeeAddress64 addr64 = XBeeAddress64(n.getZBAddressHigh(), n.getZBAddressLow());

  Serial.print("Frame to: ");
  Serial.print(addr64.getMsb(), HEX);
  Serial.println(addr64.getLsb(), HEX);

  uint8_t payload[f.createFrameLength()];
  f.createFrame(payload);
  
  //ZBTxRequest(const XBeeAddress64 &addr64, uint16_t addr16, uint8_t broadcastRadius, uint8_t option, uint8_t *payload, uint8_t payloadLength, uint8_t frameId);
                                            //maximos saltos              no retries                                        no transmit status
  //ZBTxRequest zbTx = ZBTxRequest(addr64, 0, ZB_BROADCAST_RADIUS_MAX_HOPS, 0, payload, sizeof(payload), NO_RESPONSE_FRAME_ID);
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload), NO_RESPONSE_FRAME_ID);

  xbee.send(zbTx);

  //Serial.println(F("Listen for the TX status..."));
  //listenXBee();
}


//----------------RESPONSES-------------------------
ModemStatusResponse msr = ModemStatusResponse();
void receiveModemStatus() {
  
  xbee.getResponse().getModemStatusResponse(msr);

  Serial.print(F("Modem Status Response. Status = "));
  Serial.println(msr.getStatus());
}

ZBTxStatusResponse txStatus = ZBTxStatusResponse();
void receiveTxStatusResponse() {
  xbee.getResponse().getZBTxStatusResponse(txStatus);

  // get the delivery status, the fifth byte
  Serial.print(F("Tx Status Response. Status = "));
  Serial.println(txStatus.getDeliveryStatus());
  if (txStatus.getDeliveryStatus() == SUCCESS) {
    Serial.println(F("Success"));
    //txStatusIsOk = true;
  } else {
    Serial.println(F("Remote XBee didn't receive the packet"));
  }
}

ZBRxResponse rx = ZBRxResponse();
Frame frameReceived;
void receiveRXResponse() {
  xbee.getResponse().getZBRxResponse(rx);
  Serial.println(F("ZB> RX Response"));

  Serial.print(F("ZB> Remote Address: "));
  Serial.print(rx.getRemoteAddress64().getMsb(), HEX);
  Serial.println(rx.getRemoteAddress64().getLsb(), HEX);

  if(me.getZBAddressHigh() == rx.getRemoteAddress64().getMsb() && me.getZBAddressLow() == rx.getRemoteAddress64().getLsb()){
    Serial.println(F("ZB> Received own packet. Escaping..."));
    
  } else {
    
    Serial.print(F("ZB> Length: "));
    Serial.println(rx.getDataLength());

    uint8_t dataReceived[rx.getDataLength()];

    Serial.print(F("ZB> Data: "));
    for (int i = 0; i < rx.getDataLength(); i++) {
      dataReceived[i] = rx.getData(i);
      Serial.print(dataReceived[i]);
      Serial.print(F(" "));
    }
    Serial.println(F(""));

    if (frameReceived.validate(dataReceived, rx.getDataLength())) {
      Serial.println(F("ZB> Frame received"));

      frameReceived.clear();
      frameReceived = Frame(dataReceived, rx.getDataLength());

      Serial.print(F("ZB> Frame code: "));
      Serial.println(frameReceived.getCode());


      //checking if node is added
      int8_t nodeIndex = nodes.checkIfAdded(rx.getRemoteAddress64().getMsb(), rx.getRemoteAddress64().getLsb());
      Serial.print(F("ZB> node found: "));
      Serial.println(nodeIndex);
      if (nodeIndex != NODE_NOT_FOUND) {
        Serial.println(F("ZB> Node Authenticated"));
        NodeInfo remoteNode = nodes.nodeInfoAt(nodeIndex);

        switch (frameReceived.getCode()) {
          case CODE_SAMPLE:
            {
              blink.info(INFO_FRAME_CODE_SAMPLE_RECEIVED);

              PullData pd(0, frameReceived);

              Serial.printf("debug 1\n\r"); //delay(10);
              if(addSecureParametersToPullData(&pd, remoteNode)){
                Serial.printf("debug 2\n\r"); //delay(10);
                if (pullDataPool.push(pd) == false) {
                  Serial.println(F("PullDataPool is full. Sending pulldata to SD"));
                  blink.error(ERROR_PULLDATAPOOL_FULL);
                  blink.info(INFO_PULLDATA_TO_SD);
                  mySDManager.newPullDataInSD(pd.toSD());
                } else {
                  Serial.println(F("PullData added"));
                }
                Serial.println(F("ZB> Sending ACK"));
                Frame frameAck(CODE_SAMPLE_ACK, frameReceived.getId());
                sendFrame(frameAck, remoteNode);
              } else {
                Serial.println(F("Error setting secure parameters to pulldata"));
              }
            }
            break;
          case CODE_RESPONSE_PARAMETERS:
            {
              blink.info(INFO_FRAME_CODE_RESPONSE_PARAMETERS_RECEIVED);

              Serial.println(F("ZB> received response parameters"));
              remoteNode.setParametersByFrame(frameReceived);
              Serial.print(F("ZB> new parameters: "));
              Serial.println(remoteNode.getSampleRate());
              Serial.println(remoteNode.getOutputValue());

              Serial.println(F("ZB> Sending parameters to server"));


              //TODO: enviar de forma periódica
              //enviar un nodeInfoJSON
              //securizar estos parámetros
              getSecureParams("[" + remoteNode.toJSON() + "]");
              //comprobar error
              serverInfo();
            }
            break;
          case CODE_RESPONSE_SAMPLERATE:
            {
              blink.info(INFO_FRAME_CODE_RESPONSE_SAMPLERATE_RECEIVED);

              Serial.println(F("ZB> received response samplerate"));
              remoteNode.setParametersByFrame(frameReceived);
              Serial.print(F("ZB> new SampleRate: "));
              Serial.println(remoteNode.getSampleRate());
            }
            break;
          case CODE_RESPONSE_OUTPUT:
            {
              blink.info(INFO_FRAME_CODE_RESPONSE_OUTPUT_RECEIVED);

              Serial.println(F("ZB> received response output"));
              remoteNode.setParametersByFrame(frameReceived);
              Serial.print(F("ZB> new outputValue: "));
              Serial.println(remoteNode.getOutputValue());
            }
            break;
          //con cualquier otro código indica que está autenticado (para los heartbeat)
          default:
            {
              Frame frameAuthenticated(CODE_AUTHENTICATED);
              sendFrame(frameAuthenticated, remoteNode);
            }
          break;
        }

      } else {
        //if not added, send request auth
        NodeInfo strangerNode(rx.getRemoteAddress64().getMsb(), rx.getRemoteAddress64().getLsb());

        switch (frameReceived.getCode()) {
          case CODE_RESPONSE_AUTH:
            {
              blink.info(INFO_FRAME_CODE_RESPONSE_AUTH_RECEIVED);

              Serial.println(F("ZB> received response auth"));
              strangerNode.setParametersByFrame(frameReceived);

              Serial.println(F("ZB> StrangerNode checking if node is valid"));
              if (nodes.checkIfValidToBeAdded(strangerNode) == true) {
                Serial.println(F("ZB> FirstValidation Ok"));

                Serial.println(F("ZB> Checking if node is valid in server"));
                getSecureParams("[" + strangerNode.toJSON() + "]");

                unsigned int responseCode = serverInfo();
                if (responseCode == HTTP_RESPONSE_OK) {
                  Serial.println(F("ZB> StrangerNode added to nodes"));
                  nodes.push(strangerNode);
                  
                  Frame frameAuthenticated(CODE_AUTHENTICATED);
                  sendFrame(frameAuthenticated, strangerNode);

                  blink.info(INFO_NODE_ADDED_TO_NODES);
                  
                  Serial.println(F("ZB> Requesting parameters"));
                  Frame frameRequestParameters(CODE_REQUEST_PARAMETERS);
                  sendFrame(frameRequestParameters, strangerNode);
                } else if (responseCode == HTTP_RESPONSE_FORBIDDEN) {
                  Serial.println(F("ZB> StrangerNode is a malicious node"));
                  blink.error(ERROR_NODE_CANNOT_BE_ADDED_TO_NODES_BECAUSE_ANOTHER_NODE_WITH_SAME_NODENAME_EXISTS);
                } else {
                  Serial.println(F("ZB> Cannot add node to nodes"));
                  blink.error(ERROR_NODE_CANNOT_BE_ADDED_TO_NODES);
                }
              } else {
                Serial.println(F("ZB> Node with same nodeName was added previously."));
                Serial.println(F("ZB> Cannot add node to nodes"));
                blink.error(ERROR_NODE_CANNOT_BE_ADDED_TO_NODES_BECAUSE_ANOTHER_NODE_WITH_SAME_NODENAME_EXISTS);
              }
            }
            break;
          default:
            {
              blink.info(INFO_FRAME_NODE_NO_AUTHENTICATED);

              Serial.println(F("ZB> Node No Authenticated"));
              Serial.println(F("ZB> Sending RequestAuth"));
              Frame frameRequestAuth(CODE_REQUEST_AUTH);
              sendFrame(frameRequestAuth, strangerNode);
            }
            break;
        }
      }
    } else {
      blink.error(ERROR_FRAME_VALIDATION);

      Serial.print(F("ZB> FRAME VALIDATION ERROR: "));
      Serial.println(frameReceived.validate(dataReceived, rx.getDataLength()));
    }
  }
  delay(200); //delay para no saturar
}

//---------------------- AT COMMANDS -----------------------------------
void sendAtCommand(uint8_t cmd[]) {
  Serial.println(F("Sending At command..."));
  AtCommandRequest atRequest = AtCommandRequest(cmd);

  xbee.send(atRequest);
  delay(100);
  Serial.println(F("Waiting AT Command response"));
  listenXBee();
}

void sendAtCommand(uint8_t cmd[], uint8_t cmdValue[], int cmdLength) {
  Serial.println(F("Sending At command..."));
  Serial.print(F("cmd: "));
  for (int i = 0; i < 2; i++) {
    Serial.print((char)cmd[i]);
  }
  Serial.println(F(""));
  Serial.print(F("valueLength: "));
  Serial.println(cmdLength);
  Serial.print(F("value: "));
  for (int i = 0; i < cmdLength; i++) {
    Serial.print(cmdValue[i], HEX);
  }
  Serial.println(F(""));


  AtCommandRequest atRequest = AtCommandRequest(cmd, cmdValue, cmdLength);

  xbee.send(atRequest);
  delay(100);
  Serial.println(F("Waiting AT Command response"));
  listenXBee();
}

void receiveAtCommand() {
  AtCommandResponse atResponse = AtCommandResponse();
  xbee.getResponse().getAtCommandResponse(atResponse);
  Serial.println(F("AT Command Response"));
  if (atResponse.isOk()) {
    Serial.println(F("Status OK"));
    Serial.print(F("Command: "));
    Serial.print((char)atResponse.getCommand()[0]);
    Serial.println((char)atResponse.getCommand()[1]);
    Serial.print(F("ValueLength: "));
    Serial.println(atResponse.getValueLength());
    Serial.print(F("Value: "));
    for (int i = 0; i < atResponse.getValueLength(); i++) {
      Serial.print(atResponse.getValue()[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println(F(""));

    if (atResponse.getCommand()[0] == niCmd[0] && atResponse.getCommand()[1] == niCmd[1] && atResponse.getValueLength() > 0) {

      char alias[SIZE_PARAM];
      Serial.print(F("NI is: "));
      for (int i = 0; i < SIZE_PARAM; i++) {
        alias[i] = (char)atResponse.getValue()[i];
        Serial.print(alias[i]);
      }
      Serial.println(F(""));

      //me.setNodeAuth(alias);
    } else if (atResponse.getCommand()[0] == shCmd[0] && atResponse.getCommand()[1] == shCmd[1] && atResponse.getValueLength() > 0) {
      long msb = 0;

      msb += atResponse.getValue()[0] * pow(2, 24);
      msb += atResponse.getValue()[1] * pow(2, 16);
      msb += atResponse.getValue()[2] * pow(2, 8);
      msb += atResponse.getValue()[3] << 0;

      me.setZBAddressHigh(msb);

      Serial.print(F("SH is: "));
      Serial.println(me.getZBAddressHigh(), HEX);
    } else if (atResponse.getCommand()[0] == slCmd[0] && atResponse.getCommand()[1] == slCmd[1] && atResponse.getValueLength() > 0) {
      long lsb = 0;

      lsb += atResponse.getValue()[0] * pow(2, 24);
      lsb += atResponse.getValue()[1] * pow(2, 16);
      lsb += atResponse.getValue()[2] * pow(2, 8);
      lsb += atResponse.getValue()[3] << 0;

      me.setZBAddressLow(lsb);

      Serial.print(F("SL is: "));
      Serial.println(me.getZBAddressLow(), HEX);
    } else if (atResponse.getCommand()[0] == idCmd[0] && atResponse.getCommand()[1] == idCmd[1] && atResponse.getValueLength() > 0) {
      long id = 0;
      for (int i = 0; i < atResponse.getValueLength(); i++) {
        id += int((atResponse.getValue()[atResponse.getValueLength() - 1 - i] * pow(2, i * 8)) + 0.5);
      }
      //XBEE_ID = id;
      Serial.print(F("ID is: "));
      Serial.println(id, HEX);
    } else if(atResponse.getCommand()[0] == opCmd[0] && atResponse.getCommand()[1] == opCmd[1]){
      Serial.println(F("XBEE is alive"));
      xbee_available = true;
    }
  } else {
    Serial.print(F("Command return error code: "));
    Serial.println(atResponse.getStatus(), HEX);
  }
}


//---------------------------SERVER COMMUNICATIONS ----------------------------------
String payload_to_server = "";
HTTPSRequest httpsRequest = HTTPSRequest();
HTTPSResponse httpsResponse = HTTPSResponse();
unsigned int serverTime() {

  unsigned int responseCode = 0;

  if(server_available){
    Serial.println(F("REST> time"));
    blink.info(INFO_SERVER_TIME);

    payload_to_server = buildPayload();

    //Serial.print(F("Payload: "));
    //Serial.println(payload);
    Serial.print(F("REST> Payload length: "));
    Serial.println(payload_to_server.length());

    if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status
      httpsRequest = HTTPSRequest(HTTP_POST, restTime, payload_to_server);
      httpsRequest.setHeader("User-Agent", "masterNode");
      httpsRequest.setHeader("Content-Type", "application/json");

      httpsResponse = https.sendRequest(httpsRequest);

      responseCode = httpsResponse.getResponseCode();

      if (responseCode == HTTP_RESPONSE_OK) {
        String timeFromServer = httpsResponse.getPayload();

        //Serial.print(F("REST> timeFromServer: "));
        //Serial.println(timeFromServer);

        boolean error = false;

        long newTime = atol(timeFromServer.c_str());
        if (error == false) {
          DateTime now = DateTime(newTime);

          Serial.print(F("REST> Time Received: "));
          Serial.print(now.year());
          Serial.print(F("/"));
          Serial.print(now.month());
          Serial.print(F("/"));
          Serial.print(now.day());
          Serial.print(F(" "));
          Serial.print(now.hour());
          Serial.print(F(":"));
          Serial.print(now.minute());
          Serial.print(F(":"));
          Serial.println(now.second());

          rtc.adjust(now.unixtime());

          DateTime checkTime = rtc.now();

          //Serial.print(F("It is ok? = "));
          //Serial.println(checkTime.unixtime());

          if (checkTime.unixtime() < newTime - 10 || checkTime.unixtime() > newTime + 10) {  //está mal
            blink.error(ERROR_SERVER_TIME_SYNC);
            return 0;
          }
          Serial.println(F("REST> TIME SET OK"));

        } else {
          blink.error(ERROR_SERVER_TIME_SYNC);
        }
      } else {
        Serial.println(F("REST> ERROR"));
        blink.error(ERROR_SERVER_RESPONSE_CODE);
        blink.error(ERROR_SERVER_TIME_SYNC);
      }

    } else {
      Serial.println(F("REST> Not connect to WiFi."));
      blink.error(ERROR_NO_WIFI);
      connectToWiFi();
    }

    //un código de respuesta 0 significará que el servidor no está disponible
    if(responseCode == 0){
      server_available = false;
    }
  } else {
    Serial.println(F("REST> Server is not available"));
  }

  return responseCode;
}

unsigned int serverCreate() {

  unsigned int responseCode = 0;

  if(server_available){
    Serial.println(F("REST> create"));
    blink.info(INFO_SERVER_CREATE);

    payload_to_server = buildPayload();

    //Serial.print(F("Payload: "));
    //Serial.println(payload);
    Serial.print(F("REST> Payload length: "));
    Serial.println(payload_to_server.length());

    if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status
      httpsRequest = HTTPSRequest(HTTP_POST, restCreate, payload_to_server);
      httpsRequest.setHeader("User-Agent", "masterNode");
      httpsRequest.setHeader("Content-Type", "application/json");
      httpsResponse = https.sendRequest(httpsRequest);

      responseCode = httpsResponse.getResponseCode();
      Serial.print(F("REST> Response Code: "));
      Serial.println(responseCode);
      Serial.println(F("REST> Payload:"));
      Serial.println(httpsResponse.getPayload());

      switch (responseCode) {
        case HTTP_RESPONSE_OK:      
          Serial.println(F("REST> Data sended"));
          blink.info(INFO_SERVER_CREATE_DATA_SENDED);
          break;
        case HTTP_RESPONSE_CONFLICT:
          Serial.println(F("REST> Some of the payload send exists in the server"));
          blink.info(ERROR_SERVER_CREATE_CONFLICT);
          break;
        case HTTP_RESPONSE_PARTIAL_CONTENT:
          Serial.println(F("REST> Some of the payload sent is not valid"));
          blink.info(ERROR_SERVER_CREATE_PARTIAL);
          break;   
        default:
          Serial.println(F("REST> Error"));
          blink.error(ERROR_SERVER_RESPONSE_CODE);
          break;
      }

    } else {
      Serial.println(F("REST> Not connected to WiFi."));
      blink.error(ERROR_NO_WIFI);
      connectToWiFi();
    }

    //un código de respuesta 0 significará que el servidor no está disponible
    if(responseCode == 0){
      server_available = false;
    }
  } else {
    Serial.println(F("REST> Server is not available"));
  }

  return responseCode;
}


unsigned int serverInfo() {
  unsigned int responseCode = 0;

  if(server_available){
    Serial.println(F("REST> info"));
    blink.info(INFO_SERVER_INFO);

    payload_to_server = buildPayload();

    //Serial.print(F("Payload: "));
    //Serial.println(payload);
    Serial.print(F("REST> Payload length: "));
    Serial.println(payload_to_server.length());

    if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status
      httpsRequest = HTTPSRequest(HTTP_POST, restInfo, payload_to_server);
      httpsRequest.setHeader("User-Agent", "masterNode");
      httpsRequest.setHeader("Content-Type", "application/json");
      httpsResponse = https.sendRequest(httpsRequest);

      responseCode = httpsResponse.getResponseCode();
      Serial.print(F("REST> Response Code: "));
      Serial.println(responseCode);
      Serial.println(F("REST> Payload:"));
      Serial.println(httpsResponse.getPayload());

      switch (responseCode) {
        case HTTP_RESPONSE_OK:
          Serial.println(F("REST> Data sended"));
          blink.info(INFO_SERVER_INFO_DATA_SENDED);
          break;
        default:
          Serial.println(F("REST> Error"));
          blink.error(ERROR_SERVER_RESPONSE_CODE);
          break;
      }

    } else {
      Serial.println(F("REST> Not connected to WiFi."));
      blink.error(ERROR_NO_WIFI);
      connectToWiFi();
    }

    //un código de respuesta 0 significará que el servidor no está disponible
    if(responseCode == 0){
      server_available = false;
    }
  } else {
    Serial.println(F("REST> Server is not available"));
  }

  return responseCode;
}

unsigned int serverGet() {
  unsigned int responseCode = 0;

  if(server_available){
    Serial.println(F("REST> get"));
    blink.info(INFO_SERVER_GET);

    payload_to_server = buildPayload();

    //Serial.print(F("Payload: "));
    //Serial.println(payload);
    Serial.print(F("REST> Payload length: "));
    Serial.println(payload_to_server.length());

    if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status
      httpsRequest = HTTPSRequest(HTTP_POST, restGet, payload_to_server);
      httpsRequest.setHeader("User-Agent", "masterNode");
      httpsRequest.setHeader("Content-Type", "application/json");

      httpsResponse = https.sendRequest(httpsRequest);

      responseCode = httpsResponse.getResponseCode();

      Serial.print(F("REST> Responde Code: "));
      Serial.println(responseCode);

      if (httpsResponse.getResponseCode() == HTTP_RESPONSE_OK) {
        String responsePayload = httpsResponse.getPayload();

        Serial.print(F("REST> response: "));
        Serial.println(responsePayload);

        //obtengo el JSON y lo deserializo
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, responsePayload);

        JsonArray array = doc.as<JsonArray>();
        Serial.print(F("REST> Received objects: "));
        Serial.println(array.size());

        for(JsonVariant v : array){
          JsonObject obj = v.as<JsonObject>();

          String nodeName = obj[String("n")];
          bool outputValue = obj[String("o")];
          long sampleRate = obj[String("s")];

          Serial.print(F("REST> nodeName: "));
          Serial.println(nodeName);
          Serial.print(F("REST> outputValue: "));
          Serial.println(outputValue);
          Serial.print(F("REST> sampleRate: "));
          Serial.println(sampleRate);

          if (me.getNodeName().equals(nodeName)) {
            Serial.println(F("REST> Updating my values"));
            
            if(sampleRate != me.getSampleRate()){
              me.setSampleRate(sampleRate);
              preferences.putInt(PARAMETERS_SAMPLERATE, sampleRate);
            }
            if(outputValue != me.getOutputValue()){
              me.setOutputValue(outputValue);
              
              if(outputValue == true){
                preferences.putInt(PARAMETERS_OUTPUTVALUE, 1);
              } else {
                preferences.putInt(PARAMETERS_OUTPUTVALUE, 0);
              }
            }
            Serial.println(F("Values updated"));

            blink.info(INFO_SERVER_GET_MASTER_VALUES);
          } else {
            //si no soy yo
            int8_t nodeIndex = nodes.checkIfAdded(nodeName);
            Serial.print(F("ZB> node found: "));
            Serial.println(nodeIndex);
            if (nodeIndex != NODE_NOT_FOUND) {
              NodeInfo node = nodes.nodeInfoAt(nodeIndex);  //obtengo el nodo
              bool values_changed = false;
              if(sampleRate != node.getSampleRate()){
                node.setSampleRate(sampleRate);
                values_changed = true;
              }
              if(outputValue != node.getOutputValue()){
                node.setOutputValue(outputValue);
                values_changed = true;
              }

              if(values_changed){
                //nodes.updateNodeInfo(nodeIndex, node);    //no lo actualizo. le mando un frame_request_parameters y lo gestiono ahí
                blink.info(INFO_SERVER_GET_SLAVE_VALUES);                
                
                Serial.println(F("ZB> Set new parameters"));
                Frame newValues;
                newValues.createSetParameters(node.getSampleRate(), node.getOutputValue());
                sendFrame(newValues, node);
                
                Serial.println(F("ZB> Requesting parameters"));
                Frame frameRequestParameters(CODE_REQUEST_PARAMETERS);
                sendFrame(frameRequestParameters, node);

                //escucho ahora para evitar que se junten 20 resposne parameters de 20 nodos distintos
                delay(100);
                listenXBee();   //escuchamos la respuesta para actualizar los datos
              }

              Serial.println(F("ZB> Remote node updated"));
            }
          }
        } //end json array for
      } else {
        Serial.println(F("REST> ERROR"));
        blink.error(ERROR_SERVER_RESPONSE_CODE);
      }

    } else {
      Serial.println(F("REST> Not connect to WiFi."));
      blink.error(ERROR_NO_WIFI);
      connectToWiFi();
    }
    
    //un código de respuesta 0 significará que el servidor no está disponible
    if(responseCode == 0){
      server_available = false;
    }
  } else {
    Serial.println(F("REST> Server is not available"));
  }

  return responseCode;
}

bool serverTest() {
  WiFiClient client;
  client.setTimeout(10000); // Establecer un tiempo de espera de 5 segundos
  if ((WiFi.status() == WL_CONNECTED)) {
    if (client.connect(server, HTTPS_PORT)) {
      Serial.println("REST> SERVER OK");
      client.stop(); // cerrar la conexión
      return true;
    } else {
      Serial.println("REST> SERVER NOT AVAILABLE");
      return false;
    }
  } else {
    Serial.println(F("REST> Not connect to WiFi."));
    blink.error(ERROR_NO_WIFI);
    connectToWiFi();
    return false;
  }
}

/*
 * #define CONFIGURATION_CODE_BYTE 1
 * #define CONFIGURATION_CODE_SSID 'A'
 * #define CONFIGURATION_CODE_SSID_PASSWORD 'B'
 * #define CONFIGURATION_CODE_NETWORK_ID 'C'
 * #define CONFIGURATION_CODE_NETWORK_KEY 'D'
 * #define CONFIGURATION_CODE_SAMPLERATE 'E'
 * #define CONFIGURATION_CODE_NODENAME 'F'
 * #define CONFIGURATION_CODE_NODEPASS 'G'
 * #define CONFIGURATION_CODE_AUTHNAME 'H'
 * #define CONFIGURATION_CODE_AUTHPASS 'I'
 * #define CONFIGURATION_CODE_GET_OFFSET 'J'
 */

//-------------------------Parte de configuración-------------------------------
void configurationMode() {
  delay(100);
  //Serial.println(F("CONSOLE_ENABLE"));

  String consoleRead;
  if (Serial.available()) {
    while (Serial.available()) {
      char readByte = Serial.read();
      if (readByte != '\n' && readByte != CONFIGURATION_CR && readByte != CONFIGURATION_LF) {
        consoleRead += readByte;
      }
      delay(100);
    }

    Serial.print(F("Data: "));
    Serial.println(consoleRead);

    //formato del paquete [prelude(0x54), code[1Byte], data[16Bytes] (los 0 no forman parte del contenido (el contenido va en los bits mas significantes))]
    if (consoleRead.length() >= CONFIGURATION_LENGTH_MIN && consoleRead.charAt(CONFIGURATION_PRELUDE_BYTE) == CONFIGURATION_PRELUDE) {
      String payload = consoleRead.substring(CONFIGURATION_PAYLOAD_BEGIN_BYTE);
      switch (consoleRead.charAt(CONFIGURATION_CODE_BYTE)) {
        case CONFIGURATION_CODE_SSID:
          {
            Serial.print(F("CONF> new SSID: "));
            preferences.putString(PARAMETERS_SSID, payload);
            Serial.println(payload);
          }
          break;

        case CONFIGURATION_CODE_SSID_PASSWORD:
          {
            Serial.print(F("CONF> new SSID password: "));
            preferences.putString(PARAMETERS_SSID_PASSWORD, payload);
            Serial.println(payload);
          }
          break;

        case CONFIGURATION_CODE_NETWORK_ID:
          {
            int newNetworkId = payload.toInt();
            if (newNetworkId > 0) {
              Serial.print(F("CONF> new network id: "));
              Serial.println(newNetworkId);
              preferences.putInt(PARAMETERS_NETWORK_ID, newNetworkId);
              XBEEchangeId((long)newNetworkId);
            } else {
              Serial.println(F("network id must be a integer and must be greater than 0"));
            }
          }
          break;

        case CONFIGURATION_CODE_NETWORK_KEY:
          {
            int newNetworkKey = payload.toInt();
            if (newNetworkKey > 0) {
              Serial.print(F("CONF> new network key: "));
              Serial.println(newNetworkKey);
              XBEEchangeKey((long)XBEEchangeKey);
            } else {
              Serial.println(F("key must be a integer and must be greater than 0"));
            }
          }
          break;

        case CONFIGURATION_CODE_SAMPLERATE:
          {
            int newSampleRate = payload.toInt();
            if (newSampleRate >= MIN_SAMPLE_RATE) {
              Serial.print(F("CONF> new samplerate: "));
              Serial.println(newSampleRate);
              preferences.putInt(PARAMETERS_SAMPLERATE, newSampleRate);
            } else {
              Serial.print(F("samplerate must be a integer and must be greater than "));
              Serial.println(MIN_SAMPLE_RATE - 1);
            }
          }
          break;

        case CONFIGURATION_CODE_NODENAME:
          {
            if (payload.length() > 0 && payload.length() <= NODENAME_SIZE) {
              Serial.print(F("CONF> new nodename: "));
              Serial.println(payload);
              preferences.putString(PARAMETERS_NODENAME, payload);
            } else {
              Serial.print(F("Nodename length must be greater than 0 and less than "));
              Serial.println(NODENAME_SIZE);
            }
          }
          break;

        case CONFIGURATION_CODE_NODEPASS:
          {
            if (payload.length() > 0 && payload.length() <= NODEPASS_SIZE) {
              Serial.print(F("CONF> new nodepass: "));
              Serial.println(payload);
              preferences.putString(PARAMETERS_NODEPASS, payload);
            } else {
              Serial.print(F("Nodepass length must be greater than 0 and less than "));
              Serial.println(NODEPASS_SIZE);
            }
          }
          break;

        case CONFIGURATION_CODE_AUTHNAME:
          {
            if (payload.length() > 0) {
              Serial.print(F("CONF> new authname: "));
              Serial.println(payload);
              preferences.putString(PARAMETERS_AUTHNAME, payload);
            } else {
              Serial.println(F("authName length must be greater than 0"));
            }
          }
          break;

        case CONFIGURATION_CODE_AUTHPASS:
          {
            if (payload.length() > 0) {
              Serial.print(F("CONF> new authpass: "));
              Serial.println(payload);
              preferences.putString(PARAMETERS_AUTHPASS, payload);
            } else {
              Serial.println(F("authName length must be greater than 0"));
            }
          }
          break;

        case CONFIGURATION_CODE_GET_OFFSET:
          {
            digitalWrite(ENABLE_SENSORS, HIGH);
            delay(100);
            
            Serial.println(F("Reading current..."));
            long measures_current[4] = {0,0,0,0};

            //5 mediciones y hago media
            for(int meassureX = 0; meassureX < 5; meassureX++){
              Serial.printf("Measure %d\n\r", meassureX);


              // Reset devices
              MCP342x::generalCallReset();
              delay(1);  // MC342x needs 300us to settle, wait 1ms
            
              long value_current[4] = {0,0,0,0};
              for (int i = 0; i < 4; i++) {
                uint8_t err = 0;
                MCP342x::Config status;
                switch (i) {
                  case 0:
                    err = adc_current.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[0], status);
                    break;
                  case 1:
                    err = adc_current.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[1], status);
                    break;
                  case 2:
                    err = adc_current.convertAndRead(MCP342x::channel3, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[2], status);
                    break;
                  case 3:
                    err = adc_current.convertAndRead(MCP342x::channel4, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_current[3], status);
                    break;
                }
                if (err) {
                  Serial.print(F("Convert error: "));
                  Serial.println(err);
                  blink.error(ERROR_CONVERSION_CURRENT);
                }

                delay(500);
                Serial.printf("Offset current %d: %d\n", i, value_current[i]);
            
              }

              measures_current[0] += value_current[0];
              measures_current[1] += value_current[1];
              measures_current[2] += value_current[2];
              measures_current[3] += value_current[3];
            }

            Serial.println(F("Reading voltage..."));
            long measures_voltage[4] = {0,0,0,0};

            // Reset devices
            /*MCP342x::generalCallReset();
            delay(1);  // MC342x needs 300us to settle, wait 1ms

            for (int i = 0; i < 4; i++) {
              uint8_t err = 0;
              MCP342x::Config status;
              switch (i) {
                case 0:
                  err = adc_voltage.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[0], status);
                  break;
                case 1:
                  err = adc_voltage.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[1], status);
                  break;
                case 2:
                  err = adc_voltage.convertAndRead(MCP342x::channel3, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[2], status);
                  break;
                case 3:
                  err = adc_voltage.convertAndRead(MCP342x::channel4, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value_voltage[3], status);
                  break;
              }

              if (err) {
                Serial.print(F("Convert error: "));
                Serial.println(err);
                blink.error(ERROR_CONVERSION_VOLTAGE);
              }
              delay(500);
            }*/

            //5 mediciones y hago media
            for(int meassureX = 0; meassureX < 5; meassureX++){
              Serial.printf("Measure %d\n\r", meassureX);

              long value_voltage[4] = {0,0,0,0};
              for (uint8_t i = 0; i < 4; i++) {
                value_voltage[i] = ads_voltage.readADC_SingleEnded(i);
                Serial.printf("Offset voltage %d: %d\n", i, value_voltage[i]);
                delay(500);
              }
              measures_voltage[0] += value_voltage[0];
              measures_voltage[1] += value_voltage[1];
              measures_voltage[2] += value_voltage[2];
              measures_voltage[3] += value_voltage[3];              
            }

            measures_current[0] = int(measures_current[0]/5);
            measures_current[1] = int(measures_current[1]/5);
            measures_current[2] = int(measures_current[2]/5);
            measures_current[3] = int(measures_current[3]/5);

            measures_voltage[0] = int(measures_voltage[0]/5);
            measures_voltage[1] = int(measures_voltage[1]/5);
            measures_voltage[2] = int(measures_voltage[2]/5);
            measures_voltage[3] = int(measures_voltage[3]/5);

            preferences.putInt(PARAMETERS_ADC1_OFFSET1, measures_current[0]);
            preferences.putInt(PARAMETERS_ADC1_OFFSET2, measures_current[1]);
            preferences.putInt(PARAMETERS_ADC1_OFFSET3, measures_current[2]);
            preferences.putInt(PARAMETERS_ADC1_OFFSET4, measures_current[3]);

            preferences.putInt(PARAMETERS_ADC2_OFFSET1, measures_voltage[0]);
            preferences.putInt(PARAMETERS_ADC2_OFFSET2, measures_voltage[1]);
            preferences.putInt(PARAMETERS_ADC2_OFFSET3, measures_voltage[2]);
            preferences.putInt(PARAMETERS_ADC2_OFFSET4, measures_voltage[3]);

            Serial.printf("offset adc1: %d\t%d\t%d\t%d\n\r", measures_current[0], measures_current[1], measures_current[2], measures_current[3]);
            Serial.printf("offset adc2: %d\t%d\t%d\t%d\n\r", measures_voltage[0], measures_voltage[1], measures_voltage[2], measures_voltage[3]);
            
            digitalWrite(ENABLE_SENSORS, LOW);

          }
          break;
          case CONFIGURATION_CODE_CLEAR_SD:
            Serial.println(F("CONF> deleting files from SD"));
            mySDManager.clearPullDatasFromSD();
          break;
      }
      preferences.putBool(PARAMETERS_CONFIGURED, true);
    } else {
      Serial.println(F("Bad Format"));
      Serial.println(consoleRead);
    }
    delay(1000);
  }
}

void readParameters() {
  me.setSSID(preferences.getString(PARAMETERS_SSID, DEFAULT_SSID));
  me.setSSIDPassword(preferences.getString(PARAMETERS_SSID_PASSWORD, DEFAULT_SSID_PASSWORD));
  me.setNodeName(preferences.getString(PARAMETERS_NODENAME, DEFAULT_NODENAME));
  me.setNodePass(preferences.getString(PARAMETERS_NODEPASS, DEFAULT_NODEPASS));
  me.setNetworkId(preferences.getInt(PARAMETERS_NETWORK_ID, DEFAULT_NETWORK_ID));
  me.setAuthName(preferences.getString(PARAMETERS_AUTHNAME, DEFAULT_AUTHNAME));
  me.setAuthPass(preferences.getString(PARAMETERS_AUTHPASS, DEFAULT_AUTHPASS));
  me.setSampleRate(preferences.getInt(PARAMETERS_SAMPLERATE, DEFAULT_SAMPLERATE));
  me.setOutputValue(preferences.getInt(PARAMETERS_OUTPUTVALUE, DEFAULT_OUTPUTVALUE));

  adc1_offset[0] = preferences.getInt(PARAMETERS_ADC1_OFFSET1, DEFAULT_OFFSET);
  adc1_offset[1] = preferences.getInt(PARAMETERS_ADC1_OFFSET2, DEFAULT_OFFSET);
  adc1_offset[2] = preferences.getInt(PARAMETERS_ADC1_OFFSET3, DEFAULT_OFFSET);
  adc1_offset[3] = preferences.getInt(PARAMETERS_ADC1_OFFSET4, DEFAULT_OFFSET);

  adc2_offset[0] = preferences.getInt(PARAMETERS_ADC2_OFFSET1, DEFAULT_OFFSET);
  adc2_offset[1] = preferences.getInt(PARAMETERS_ADC2_OFFSET2, DEFAULT_OFFSET);
  adc2_offset[2] = preferences.getInt(PARAMETERS_ADC2_OFFSET3, DEFAULT_OFFSET);
  adc2_offset[3] = preferences.getInt(PARAMETERS_ADC2_OFFSET4, DEFAULT_OFFSET);

  Serial.println(F("Parameters"));
  Serial.print(F("SSID: "));
  Serial.println(me.getSSID());
  Serial.print(F("SSID password: "));
  Serial.println(me.getSSIDPassword());
  Serial.print(F("NodeName: "));
  Serial.println(me.getNodeName());
  Serial.print(F("NodePass: "));
  Serial.println(me.getNodePass());
  Serial.print(F("AuthName: "));
  Serial.println(me.getAuthName());
  Serial.print(F("AuthPass: "));
  Serial.println(me.getAuthPass());
  Serial.print(F("NetworkID: "));
  Serial.println(me.getNetworkId());
  Serial.print(F("SampleRate: "));
  Serial.println(me.getSampleRate());
  Serial.print(F("Output value: "));
  Serial.println(me.getOutputValue());

  Serial.print(F("offset adc1: "));
  Serial.print(adc1_offset[0]);
  Serial.print(F(" "));
  Serial.print(adc1_offset[1]);
  Serial.print(F(" "));
  Serial.print(adc1_offset[2]);
  Serial.print(F(" "));
  Serial.println(adc1_offset[3]);
  Serial.print(F("offset adc2: "));
  Serial.print(adc2_offset[0]);
  Serial.print(F(" "));
  Serial.print(adc2_offset[1]);
  Serial.print(F(" "));
  Serial.print(adc2_offset[2]);
  Serial.print(F(" "));
  Serial.println(adc2_offset[3]);

  if (me.getSSID().equals(DEFAULT_SSID) == false && me.getSSIDPassword().equals(DEFAULT_SSID_PASSWORD) == false
      && me.getNodeName().equals(DEFAULT_NODENAME) == false && me.getNodePass().equals(DEFAULT_NODEPASS) == false
      && me.getNetworkId() > 0 && me.getSampleRate() >= MIN_SAMPLE_RATE
      && me.getAuthName().equals(DEFAULT_AUTHNAME) == false && me.getAuthPass().equals(DEFAULT_AUTHPASS) == false) {

    //GETTING XBEE PARAMETERS
    sendAtCommand(shCmd);  //TODO: modo de leer parámetros, no escuchar otros mensajes
    listenXBee();
    sendAtCommand(slCmd);
    listenXBee();
  } else {
    Serial.println(F("Invalid Parameters"));
    while (true) {
      blink.error(ERROR_NODE_INVALID_PARAMETERS);
    }
  }
}

void XBEEchangeId(long newId) {
  Serial.print(F("Change ID: "));
  Serial.println(newId);

  uint8_t payload[4];
  payload[0] = newId >> 24 & 0xff;
  payload[1] = newId >> 16 & 0xff;
  payload[2] = newId >> 8 & 0xff;
  payload[3] = newId & 0xff;

  Serial.print(F("Payload: "));
  for (int i = 0; i < 4; i++) {
    Serial.print(payload[i], HEX);
  }
  Serial.println(F(""));

  sendAtCommand(idCmd, payload, sizeof(payload));
  delay(100);
  listenXBee();
  sendAtCommand(wrCmd);
  delay(100);
  listenXBee();
  sendAtCommand(acCmd);
  delay(100);
  listenXBee();
}

void XBEEchangeKey(long newKy) {
  Serial.print(F("Change KY: "));
  Serial.println(newKy);

  uint8_t payload[4];
  payload[0] = newKy >> 24 & 0xff;
  payload[1] = newKy >> 16 & 0xff;
  payload[2] = newKy >> 8 & 0xff;
  payload[3] = newKy & 0xff;

  Serial.print(F("Payload: "));
  for (int i = 0; i < 4; i++) {
    Serial.print(payload[i], HEX);
  }
  Serial.println(F(""));

  sendAtCommand(kyCmd, payload, sizeof(payload));
  delay(100);
  listenXBee();
  sendAtCommand(wrCmd);
  delay(100);
  listenXBee();
  sendAtCommand(acCmd);
  delay(100);
  listenXBee();
}