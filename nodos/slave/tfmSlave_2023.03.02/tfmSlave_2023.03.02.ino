#include <OneWire.h>
#include <DallasTemperature.h>
#include <string.h>
#include <XBee.h>
#include "TFMUtils.h"
#include "TFMProtocol.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>

#define CR 13
#define LF 10
#define SIZE_PARAM 8
#define TIME_LISTENING 50


#define PARAMETERS_CONFIGURED_YES 12345

#define PARAMETERS_CONFIGURED 0
#define PARAMETERS_NODENAME 16
#define PARAMETERS_NODEPASS 32
#define PARAMETERS_OUTPUTVALUE 48
#define PARAMETERS_SAMPLERATE 64
#define PARAMETERS_NETWORK_ID 80

#define PARAMETERS_ADC1_OFFSET1 96
#define PARAMETERS_ADC1_OFFSET2 112
#define PARAMETERS_ADC1_OFFSET3 128
#define PARAMETERS_ADC1_OFFSET4 144
#define PARAMETERS_ADC2_OFFSET1 160
#define PARAMETERS_ADC2_OFFSET2 176
#define PARAMETERS_ADC2_OFFSET3 192
#define PARAMETERS_ADC2_OFFSET4 208

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

#define DEFAULT_NODENAME ""
#define DEFAULT_NODEPASS ""

#define TIME_HEARTBEAT 300    //cada 5 min heartbeat

const uint8_t DALLAS_TEMP = 3;
const uint8_t GENERAL_OUTPUT = 5;
const uint8_t ENABLE_SENSORS = 7;
const int XBEE_RX = 8;
const int XBEE_TX = 9;
const uint8_t ERROR_LED = 10;
const uint8_t INFORMATION_LED = 11;
const uint8_t CONFIGURATION_PIN = 12;

SoftwareSerial XBeeSerial(XBEE_RX, XBEE_TX);
XBee xbee = XBee();

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

OneWire oneWireDallas(DALLAS_TEMP);
DallasTemperature dallasTemp(&oneWireDallas);

Blink blink(INFORMATION_LED, ERROR_LED);

FramePool framePool;
NodeInfo me;
NodeInfo master(0x0,0x0);

int adc1_offset[4] = { 0, 0, 0, 0 };
int adc2_offset[4] = { 0, 0, 0, 0 };

//utilizado para asignar ids a los frames
uint8_t frameCounter = 0;

void setup() {
  // put your setup code here, to run once:
  //This is the beggining of a great story.
  blink.info(INFO_STARTING);

  //pinMode(XBEE_RX, INPUT);
  //pinMode(XBEE_TX, OUTPUT);

  Serial.begin(9600);
  XBeeSerial.begin(9600);
  
  //Serial1.begin(9600);
  //xbee.setSerial(Serial1);
  
  xbee.setSerial(XBeeSerial);

  Serial.println(F("START"));

  pinMode(ENABLE_SENSORS, OUTPUT);
  pinMode(CONFIGURATION_PIN, INPUT_PULLUP);
  pinMode(GENERAL_OUTPUT, OUTPUT);


  sendAtCommand(opCmd);
  while(xbee_available == false){
    Serial.println(F("XBee is not available"));
    blink.error(ERROR_XBEE_NOT_RUNNING);
    listenXBee();
    delay(1000);
    sendAtCommand(opCmd);
    delay(1000);
  }
  

  checkConfigurationMode();
  //comprueba que el nodo haya sido configurado
  int nodeConfigured = 0;
  EEPROM.get(PARAMETERS_CONFIGURED, nodeConfigured);
  if (nodeConfigured != PARAMETERS_CONFIGURED_YES) {
    Serial.println(F("Node require configuration"));
    while (true) {
      blink.error(ERROR_NODE_NOT_CONFIGURED);
    }
  } else {
    readParameters();
  }
  
  //nada más encender envía un heartbeat
  Serial.println(F("Sending heartbeat"));
  Frame frameHeartbeat(CODE_HEARTBEAT);
  sendFrame(frameHeartbeat, master);
}

void checkConfigurationMode() {
  //modo de configuración
  if (!digitalRead(CONFIGURATION_PIN)) {
    Serial.println(F("CONFIGURATION MODE ON"));
  }
  while (!digitalRead(CONFIGURATION_PIN)) {
    digitalWrite(ERROR_LED, HIGH);  //console mode on
    configurationMode();

    if (!digitalRead(CONFIGURATION_PIN) == false) {
      Serial.println(F("CONFIGURATION MODE OFF"));
      digitalWrite(ERROR_LED, LOW);  //console mode off
      readParameters();
    }
  }
}

void loop() {

  unsigned long time_in_seconds = getSeconds();

  checkConfigurationMode();
  //enchufo el led si es necesario.
  digitalWrite(GENERAL_OUTPUT, me.getOutputValue());
  
  //el frameCounter no debe pasar de 256(valor maximo)
  if(frameCounter >= 250) //en 250 lo reinicio
    frameCounter = 0;

  //cada hora envía un hearbeat y te marcas como no validado. Utilizado para determinar si el master me sigue teniendo autenticado
  if(time_in_seconds%TIME_HEARTBEAT == 0){
    Serial.println(F("Deauthenticated myself"));

    me.setValidated(false);
    delay(100);
    Serial.println(F("Sending heartbeat"));
    Frame frameHeartbeat(CODE_HEARTBEAT);
    sendFrame(frameHeartbeat, master);
  }

  //si estoy validado
  if(me.getValidated()){
    //si es la hora de tomar datos

    if(time_in_seconds%me.getSampleRate() == 0){
      Serial.println(F("READING DATA"));
      readData();
    }


    //envía los datos que estén en el framepool (si los hay) además envíalos el doble de rapido que los tomas
    if (!framePool.isEmpty() && time_in_seconds%((int)me.getSampleRate()/2) == 0) {
      Serial.print(F("There are "));
      Serial.print(framePool.count());
      Serial.println(F(" frames in framePool"));

      Frame frameData = framePool.peek();
      sendFrame(frameData, master);
      delay(1000);    //espera un segundo entre envíos para evitar enviar varias frames seguidas
    }
  }

  listenXBee();
  
  
  Serial.println(time_in_seconds);
}



//----------------------- XBEE -----------------------------------------
//------------------------LISTEN XBEE-----------------------------------
int listenXBee() {
  if (xbee.readPacket(TIME_LISTENING)) {

    blink.info(INFO_ZIGBEE_PACKET_RECEIVED);
    Serial.print(F("Got a response. ApiId = "));
    Serial.println(xbee.getResponse().getApiId(), HEX);

    switch (xbee.getResponse().getApiId()) {
      case AT_COMMAND_RESPONSE:
        blink.info(INFO_ZB_ATCMD_RESPONSE);
        receiveAtCommand();
        break;
      case ZB_RX_RESPONSE:
        blink.info(INFO_ZB_RX_RESPONSE);
        receiveRXResponse();
        break;
      case ZB_TX_STATUS_RESPONSE:
        blink.info(INFO_ZB_TXSTAT_RESPONSE);
        receiveTxStatusResponse();
        break;
      case MODEM_STATUS_RESPONSE:
        blink.info(INFO_ZB_MODEMSTAT_RESPONSE);
        receiveModemStatus();
        break;
    }
    Serial.println(F("------------------------------"));
    delay(500);
    return 1;
  } else if (xbee.getResponse().isError()) {
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

  uint8_t payload[f.createFrameLength()];
  f.createFrame(payload);

  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));

  xbee.send(zbTx);

  Serial.println(F("Listen for the TX status..."));
  listenXBee();
}

bool readData() {
  blink.info(INFO_READING_DATA);
  digitalWrite(ENABLE_SENSORS, HIGH);

  float batTemperature = 0.0, pvTemperature = 0.0;
  int currentLoad = 0.0, currentBattery = 0.0, currentPhotovoltaic = 0.0;
  int voltageLoad = 0.0, voltageBattery = 0.0, voltagePhotovoltaic = 0.0;

  
  dallasTemp.requestTemperatures();

  batTemperature = dallasTemp.getTempCByIndex(0);
  pvTemperature = dallasTemp.getTempCByIndex(1);

  currentLoad = analogRead(0);
  currentBattery = analogRead(1);
  currentPhotovoltaic = analogRead(2);

  voltageLoad = analogRead(4);
  voltageBattery = analogRead(5);
  voltagePhotovoltaic = analogRead(6);

  Serial.print("batTemperature: ");
  Serial.println(batTemperature, 5);
  Serial.print("pvTemperature: ");
  Serial.println(pvTemperature, 5);

  Serial.print("loadCurrent: ");
  Serial.println(currentLoad, 5);
  Serial.print("batteryCurrent: ");
  Serial.println(currentBattery, 5);
  Serial.print("photovoltaicCurrent: ");
  Serial.println(currentPhotovoltaic, 5);

  Serial.print("loadVoltage: ");
  Serial.println(voltageLoad, 5);
  Serial.print("batteryVoltage: ");
  Serial.println(voltageBattery, 5);
  Serial.print("photovoltaicVoltage: ");
  Serial.println(voltagePhotovoltaic, 5);

  digitalWrite(ENABLE_SENSORS, LOW);

  Frame sampleFrame;
  sampleFrame.createSampleFrame(currentLoad, currentBattery, currentPhotovoltaic, voltageLoad, voltageBattery, voltagePhotovoltaic, batTemperature, pvTemperature, frameCounter++);
  
  if(framePool.push(sampleFrame)){
    Serial.println(F("sampleFrame pushed to framePool"));
    return true;
  } else {
    Serial.println(F("Framepool if full"));
    return false;
  }
}

//----------------RESPONSES-------------------------
void receiveModemStatus() {
  ModemStatusResponse msr = ModemStatusResponse();
  xbee.getResponse().getModemStatusResponse(msr);

  Serial.print(F("Modem Status Response. Status = "));
  Serial.println(msr.getStatus());
}

void receiveTxStatusResponse() {
  ZBTxStatusResponse txStatus = ZBTxStatusResponse();
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

void receiveRXResponse() {
  ZBRxResponse rx = ZBRxResponse();
  xbee.getResponse().getZBRxResponse(rx);
  Serial.println(F("RX Response"));

  Serial.print(F("Remote Address: "));
  Serial.print(rx.getRemoteAddress64().getMsb(), HEX);
  Serial.println(rx.getRemoteAddress64().getLsb(), HEX);

  Serial.print(F("Length: "));
  Serial.println(rx.getDataLength());

  uint8_t dataReceived[rx.getDataLength()];

  Serial.print(F("Data: "));
  for (int i = 0; i < rx.getDataLength(); i++) {
    dataReceived[i] = rx.getData(i);
    Serial.print(dataReceived[i]);
    Serial.print(F(" "));
  }
  Serial.println(F(""));

  Frame frameReceived;
  uint8_t validation = frameReceived.validate(dataReceived, rx.getDataLength());

  Serial.print(F("ZB> Validation: "));
  Serial.println(validation);

  if (validation) {
    Serial.println("ZB> Frame received");

    frameReceived = Frame(dataReceived, rx.getDataLength());

    Serial.print(F("ZB> Frame code: "));
    Serial.println(frameReceived.getCode());
    Serial.print(F("ZB> Frame id: "));
    Serial.println(frameReceived.getId());

    switch (frameReceived.getCode()) {
      //RESPUESTA EXITOSA DE ENVÍO DE DATOS. DEBERÍA ELIMINAR EL FRAME ENVIADO (ID DE FRAME RECIBIDO CORRESPONDE AL ID DEL FRAME ENVIADO)
      case CODE_SAMPLE_ACK:
        {
          Serial.println(F("Received sample ack"));          //TODO: comprobar id del frame
          
          //itero en la framePool en busca del frame con el mismo id que el recibido y lo elimino
          uint8_t framePoolIndex = 0;
          bool frame_deleted = false;
          while(framePoolIndex < framePool.count()){
            if(framePool.get(framePoolIndex).getId() == frameReceived.getId()){
              Serial.print(F("Removing frame: "));
              Serial.println(framePoolIndex);
              framePool.remove(framePoolIndex);
              frame_deleted = true;
              break;
            } else {
              framePoolIndex++;
            }
          }
          
          //si el framePoolIndex es igual al tamaño de framePool significará que no hay ningún framePool con ese id. Si esto ocurre elimino el más antiguo (pos = 0)
          if(frame_deleted == false){
            framePool.remove(0);
            Serial.println(F("Frame not detected, removing first frame from framePool"));
          } else {
            Serial.println(F("Frame removed from framePool"));
          }
        }
        break;
      //SOLICITUD DE ENVÍO DE FRAME DE AUTENTICACIÓN AL COORDINATOR
      case CODE_REQUEST_AUTH:
        {
          Serial.println(F("Received request auth"));

          me.setValidated(false); //el master no me tiene validado. Luego me pongo como que no lo estoy
          
          Frame authResponse;
          authResponse.createResponseAuthFrame(me.getNodeName(), me.getNodePass(), frameCounter++);

          Serial.println(F("Sending responseAuth"));
          sendFrame(authResponse, master);
        }
        break;
      case CODE_AUTHENTICATED:
        {
          Serial.println(F("Received authenticated"));

          me.setValidated(true); //el master me confirma que estoy validado
        }
        break;
      //SOLICITUD DE ENVIO DE SAMPLERATE
      case CODE_REQUEST_SAMPLERATE:
        {
          Serial.println(F("Received request samplerate"));

          Frame samplerateResponse;
          samplerateResponse.createResponseSampleRate(me.getSampleRate(), frameCounter++);
          sendFrame(samplerateResponse, master);
        }
        break;
       //NUEVO VALOR DE SAMPLERATE
        case CODE_SET_SAMPLERATE:
        {
          Serial.println(F("Received set samplerate"));
          //TODO: REDUCIR EL TIEMPO CUANDO SE SATURA EL PULLDATAPOOL
        }
        break;
      //SOLICITUD DE ENVIO DE OUTPUT
      case CODE_REQUEST_OUTPUT:
        {
          Serial.println(F("Received request output"));

          Frame outputResponse;
          outputResponse.createResponseOutput(me.getOutputValue(), frameCounter++);
          sendFrame(outputResponse, master);

        }
        break;
      //NUEVO VALOR DE OUTPUT
      case CODE_SET_OUTPUT:
        {
          Serial.println(F("Received set output"));
          //TODO
        }
        break;
      //SOLICITUD DE ENVÍO DE PARÁMETROS
      case CODE_REQUEST_PARAMETERS:
        {
          Serial.println(F("Received request parameters"));

          Frame parametersResponse;
          parametersResponse.createResponseParameters(me.getSampleRate(), me.getOutputValue(), frameCounter++);
          sendFrame(parametersResponse, master);

        }
        break;
      case CODE_SET_PARAMETERS:
        {
          Serial.println(F("Received set parameters"));

          me.setParametersByFrame(frameReceived);

          setEEPROMSampleRate(me.getSampleRate());
          setEEPROMOutput(me.getOutputValue());


        }
        break;
    }

  } else {
    blink.error(ERROR_FRAME_VALIDATION);

    Serial.print(F("ZB> FRAME VALIDATION ERROR: "));
    Serial.println(frameReceived.validate(dataReceived, rx.getDataLength()));
  }
}

//---------------------- AT COMMANDS -----------------------------------
void sendAtCommand(uint8_t cmd[]) {
  Serial.println(F("Sending At command..."));
  delay(100);
  AtCommandRequest atRequest = AtCommandRequest(cmd);

  xbee.send(atRequest);

  delay(100);
  Serial.println(F("Waiting AT Command response"));
  listenXBee();
}

void sendAtCommand(uint8_t cmd[], uint8_t cmdValue[], int cmdLength) {
  Serial.println(F("Sending At command..."));
  delay(100);
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
      Serial.println("");

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
        case CONFIGURATION_CODE_NETWORK_ID:
          {
            int newNetworkId = payload.toInt();
            if (newNetworkId > 0) {
              Serial.print(F("CONF> new network id: "));
              Serial.println(newNetworkId);
              setEEPROMNetworkId(newNetworkId);
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
            uint16_t newSampleRate = payload.toInt();
            if (newSampleRate >= MIN_SAMPLE_RATE) {
              Serial.print(F("CONF> new samplerate: "));
              Serial.println(newSampleRate);
              setEEPROMSampleRate(newSampleRate);
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
              setEEPROMNodeName(payload);
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
              setEEPROMNodePass(payload);
            } else {
              Serial.print(F("Nodepass length must be greater than 0 and less than "));
              Serial.println(NODEPASS_SIZE);
            }
          }
          break;

        case CONFIGURATION_CODE_GET_OFFSET:
          {
            digitalWrite(ENABLE_SENSORS, HIGH);
            delay(100);
            
            Serial.println(F("Reading current..."));
            long measures_current[4] = {0,0,0,0};

            for(int meassureX = 0; meassureX < 5; meassureX++){
              long value_current[4] = {0,0,0,0};
              for (uint8_t i = 0; i < 4; i++) {
                value_current[i] = analogRead(i);
                delay(500);
              }
              measures_current[0] += value_current[0];
              measures_current[1] += value_current[1];
              measures_current[2] += value_current[2];
              measures_current[3] += value_current[3];              
            }

            Serial.println(F("Reading voltage..."));
            long measures_voltage[4] = {0,0,0,0};

            for(int meassureX = 0; meassureX < 5; meassureX++){

              long value_voltage[4] = {0,0,0,0};
              for (uint8_t i = 0; i < 4; i++) {
                value_voltage[i] = analogRead(i+4);
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

            setEEPROMAdc1Offset1(measures_current[0]);
            setEEPROMAdc1Offset2(measures_current[1]);
            setEEPROMAdc1Offset3(measures_current[2]);
            setEEPROMAdc1Offset4(measures_current[3]);

            setEEPROMAdc2Offset1(measures_voltage[0]);
            setEEPROMAdc2Offset2(measures_voltage[1]);
            setEEPROMAdc2Offset3(measures_voltage[2]);
            setEEPROMAdc2Offset4(measures_voltage[3]);

            digitalWrite(ENABLE_SENSORS, LOW);

          }
          break;
      }
      EEPROM.put(PARAMETERS_CONFIGURED, PARAMETERS_CONFIGURED_YES);
    } else {
      Serial.println(F("Bad Format"));
      Serial.println(consoleRead);
    }
    delay(1000);
  }
}


void setEEPROMNodeName(String newNodeName){
  for(uint8_t i = 0; i < SIZE_PARAM; i++){
    EEPROM.put(PARAMETERS_NODENAME + i, newNodeName.charAt(i));
  }
}

String getEEPROMNodeName(){
  String s = "";
  for(uint8_t i = 0; i < SIZE_PARAM; i++){
    char c = '0';
    EEPROM.get(PARAMETERS_NODENAME + i, c);
    s += c;    
  }
  return s;
}

void setEEPROMNodePass(String newNodePass){
  for(uint8_t i = 0; i < SIZE_PARAM; i++){
    EEPROM.put(PARAMETERS_NODEPASS + i, newNodePass.charAt(i));
  }
}

String getEEPROMNodePass(){
  String s = "";
  for(uint8_t i = 0; i < SIZE_PARAM; i++){
    char c = '0';
    EEPROM.get(PARAMETERS_NODEPASS + i, c);
    s += c;    
  }
  return s;
}

void setEEPROMOutput(bool newOutput){
  EEPROM.put(PARAMETERS_OUTPUTVALUE, newOutput);
}

bool getEEPROMOutput(){
  bool output = false;
  EEPROM.get(PARAMETERS_OUTPUTVALUE, output);
  return output;
}

void setEEPROMSampleRate(uint16_t newSampleRate){
  EEPROM.put(PARAMETERS_SAMPLERATE, newSampleRate);
}

uint16_t getEEPROMSampleRate(){
  uint16_t sampleRate = MIN_SAMPLE_RATE;
  EEPROM.get(PARAMETERS_SAMPLERATE, sampleRate);
  return sampleRate;
}

void setEEPROMNetworkId(long newNetworkId){
  EEPROM.put(PARAMETERS_NETWORK_ID, newNetworkId);
}

long getEEPROMNetworkId(){
  long networkId = 0;
  EEPROM.get(PARAMETERS_NETWORK_ID, networkId);
  return networkId;
}

void setEEPROMAdc1Offset1(int offset){
  EEPROM.put(PARAMETERS_ADC1_OFFSET1, offset);
}

int getEEPROMAdc1Offset1(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC1_OFFSET1, offset);
  return offset;
}

void setEEPROMAdc1Offset2(int offset){
  EEPROM.put(PARAMETERS_ADC1_OFFSET2, offset);
}

int getEEPROMAdc1Offset2(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC1_OFFSET2, offset);
  return offset;
}

void setEEPROMAdc1Offset3(int offset){
  EEPROM.put(PARAMETERS_ADC1_OFFSET3, offset);
}

int getEEPROMAdc1Offset3(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC1_OFFSET3, offset);
  return offset;
}

void setEEPROMAdc1Offset4(int offset){
  EEPROM.put(PARAMETERS_ADC1_OFFSET4, offset);
}

int getEEPROMAdc1Offset4(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC1_OFFSET4, offset);
  return offset;
}

void setEEPROMAdc2Offset1(int offset){
  EEPROM.put(PARAMETERS_ADC2_OFFSET1, offset);
}

int getEEPROMAdc2Offset1(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC2_OFFSET1, offset);
  return offset;
}

void setEEPROMAdc2Offset2(int offset){
  EEPROM.put(PARAMETERS_ADC2_OFFSET2, offset);
}

int getEEPROMAdc2Offset2(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC2_OFFSET2, offset);
  return offset;
}

void setEEPROMAdc2Offset3(int offset){
  EEPROM.put(PARAMETERS_ADC2_OFFSET3, offset);
}

int getEEPROMAdc2Offset3(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC2_OFFSET3, offset);
  return offset;
}

void setEEPROMAdc2Offset4(int offset){
  EEPROM.put(PARAMETERS_ADC2_OFFSET4, offset);
}

int getEEPROMAdc2Offset4(){
  int offset = 0;
  EEPROM.get(PARAMETERS_ADC2_OFFSET4, offset);
  return offset;
}


void readParameters() {
  me.setNodeName(getEEPROMNodeName());
  me.setNodePass(getEEPROMNodePass());
  me.setNetworkId(getEEPROMNetworkId());
  me.setSampleRate(getEEPROMSampleRate());
  me.setOutputValue(getEEPROMOutput());

  adc1_offset[0] = getEEPROMAdc1Offset1();
  adc1_offset[1] = getEEPROMAdc1Offset2();
  adc1_offset[2] = getEEPROMAdc1Offset3();
  adc1_offset[3] = getEEPROMAdc1Offset4();

  adc2_offset[0] = getEEPROMAdc2Offset1();
  adc2_offset[1] = getEEPROMAdc2Offset2();
  adc2_offset[2] = getEEPROMAdc2Offset3();
  adc2_offset[3] = getEEPROMAdc2Offset4();

  Serial.println(F("Parameters"));
  Serial.print(F("NodeName: "));
  Serial.println(me.getNodeName());
  Serial.print(F("NodePass: "));
  Serial.println(me.getNodePass());
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
  Serial.print(" ");
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

  if (me.getNodeName().equals(DEFAULT_NODENAME) == false && me.getNodePass().equals(DEFAULT_NODEPASS) == false
      && me.getNetworkId() > 0 && me.getSampleRate() >= MIN_SAMPLE_RATE) {

    //GETTING XBEE PARAMETERS
    sendAtCommand(shCmd);  //TODO: modo de leer parámetros, no escuchar otros mensajes
    sendAtCommand(slCmd);
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
  delay(500);
  sendAtCommand(wrCmd);
  delay(500);
  sendAtCommand(acCmd);
  delay(500);
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
  delay(500);
  sendAtCommand(wrCmd);
  delay(500);
  sendAtCommand(acCmd);
  delay(500);
}


//----------------UTILIDADES-------------------
unsigned long getSeconds(){
  return millis()/1000;
}
