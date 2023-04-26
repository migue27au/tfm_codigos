#ifndef TFMPROTOCOL_H
#define TFMPROTOCOL_H

#include "Arduino.h"
#include <string.h>


//------------------------------------------------
//--------------------FRAME-----------------------
//------------------------------------------------

#define MAX_LENGTH 30

#define FRAME_PRELUDE_SIZE 1
#define FRAME_PRELUDE_BYTE 0
#define FRAME_PRELUDE 27
#define CODE_SIZE 1
#define CODE_BYTE 1
#define ID_SIZE 1
#define ID_BYTE 2


//CODES
#define CODE_HEARTBEAT 100              //envía un código de heartbeat al master para indicar que existe

#define CODE_SAMPLE 101					//envía datos de sensores.
#define CODE_SAMPLE_ACK 102				//acuse de recibo a CODE_SAMPLE indicando el identificador del frame que se recibió

#define CODE_AUTHENTICATED 200          //el servidor indica al slave que está autenticado
#define CODE_REQUEST_AUTH 201			//solicita nodeinfo.alias y nodeinfo.nodePass
#define CODE_RESPONSE_AUTH 202			//devuelve nodeinfo.alias y nodeinfo.nodePass. Respuesta a CODE_REQUEST_AUTH

#define CODE_SET_SAMPLERATE 203         //indica un nuevo valor para nodeinfo.samplerate
#define CODE_REQUEST_SAMPLERATE 204		//solicita nodeinfo.samplerate
#define CODE_RESPONSE_SAMPLERATE 205 	//devuelve nodeinfo.samplerate respuesta a CODE_REQUEST_SAMPLERATE

#define CODE_SET_OUTPUT 206				//indica un nuevo valor para nodeinfo.outputvalue
#define CODE_REQUEST_OUTPUT 207		    //solicita nodeinfo.outputvalue
#define CODE_RESPONSE_OUTPUT 208		//devuelve nodeinfo.outputvalue. en respuesat a CODE_SET_OUTPUT y CODE_REQUEST_OUTPUT

#define CODE_REQUEST_PARAMETERS 209     //d1
#define CODE_RESPONSE_PARAMETERS 210
#define CODE_SET_PARAMETERS 211         //d3

#define FRAME_VALIDATION_OK 1
#define FRAME_VALIDATION_SIZE_ERROR 2
#define FRAME_VALIDATION_PRELUDE_ERROR 3
#define FRAME_NOT_RECOGNISED 4

#define CODE_SAMPLE_DATA_LENGTH 17
#define CODE_RESPONSE_AUTH_DATA_LENGTH 16
#define CODE_RESPONSE_SAMPLERATE_DATA_LENGTH 2
#define CODE_RESPONSE_OUTPUT_DATA_LENGTH 1
#define CODE_RESPONSE_PARAMETERS_DATA_LENGTH 3
#define CODE_SET_SAMPLERATE_DATA_LENGTH 0
#define CODE_REQUEST_SAMPLERATE_DATA_LENGTH 0
#define CODE_SET_OUTPUT_DATA_LENGTH 0
#define CODE_REQUEST_OUTPUT_DATA_LENGTH 0
#define CODE_REQUEST_PARAMETERS_DATA_LENGTH 0
#define CODE_SAMPLE_ACK_DATA_LENGTH 0


class Frame{
public:
	/*Constructors*/
	Frame(uint8_t payload[], uint8_t payloadLength, uint8_t frameCode, uint8_t frameId = 0);
	Frame(uint8_t frameCode, uint8_t frameId = 0);
	Frame();

	//constructor used when you receive a packet
	Frame(uint8_t frame[], uint8_t frameLength);
	Frame(String frame);

	~Frame();

	/*Getters & Setters*/
	void setCode(uint8_t codeX);
	uint8_t getCode();
	void setLength(uint8_t lengthX);
	uint8_t getLength();
	void setId(uint8_t idX);
	uint8_t getId();
  	void setData(uint8_t value, uint8_t index);
  	void setData(uint8_t payload[], uint8_t lengthX);
  	void getData(uint8_t array[]);
  	uint8_t getData(uint8_t index);

    void clear();
  	void clearData();

  	bool equalsTo(Frame f);

  	uint8_t createFrameLength();
  	void createFrame(uint8_t dataArray[]);

	uint8_t headerLength();
	
	String toString();
	String dataToString();

    uint8_t validate(uint8_t frame[], uint8_t frameLength);

	//automatic frames
	void createSampleFrame(int loadCurrent, int batteryCurrent, int photovoltaicCurrent, int loadVoltage, int batteryVoltage, int photovoltaicVoltage, float batteryTemperature, float photovoltaicTemperature, bool output, uint8_t idX = 0);
	void createResponseAuthFrame(String alias, String nodePass, uint8_t idX = 0);
	void createResponseSampleRate(uint16_t sampleRate, uint8_t idX = 0);
	void createResponseOutput(bool outputValue, uint8_t idX = 0);
    void createSetParameters(uint16_t sampleRate, bool outputValue, uint8_t idX = 0);
    void createResponseParameters(uint16_t sampleRate, bool outputValue, uint8_t idX = 0);
private:
	uint8_t data[MAX_LENGTH];
	uint8_t length;

	uint8_t code;
	uint8_t id;
};

#ifndef ARDUINO_ARCH_ESP32      //si no es un esp32

//------------------------------------------------
//--------------------FRAMEPOOL-------------------
//------------------------------------------------


#define FRAME_NOT_FOUND -1
#define SIZE_FRAME_POOL 10

class FramePool{
public:
    FramePool();
    ~FramePool();

    bool push(Frame f); //return true if ok, false if pool is full
    Frame peek();
    Frame pop();
    Frame frameAt(uint8_t index);
    uint8_t count();
    bool isFull();
    bool isEmpty();

    Frame get(uint8_t index);
    void remove(uint8_t index);

    int8_t checkIfAdded(Frame f);

protected:
    uint8_t top = 0;
    uint8_t bottom = 0;
    struct framePool{
        bool haveFrame = false;
        Frame frame;
    } framePool[SIZE_FRAME_POOL];
};

#endif //#ifndef ARDUINO_ARCH_ESP32


//------------------------------------------------
//--------------------PULLDATA--------------------
//------------------------------------------------

#ifdef ARDUINO_ARCH_ESP32

#define RESOLUTION_VOLTAGE 2.048
#define RESOLUTION_MAX_VALUE 32767

#define SIZE_PULLDATA_POOL 15

class PullData{
public:
  /*Constructors*/
  PullData();
  PullData(String authNameX, String hexNonceX, String hexHashX, long timeX, float loadCurrentX, float batteryCurrentX, float photovoltaicCurrentX, float loadVoltageX, float batteryVoltageX, float photovoltaicVoltageX, float batteryTemperatureX, float photovoltaicTemperatureX, float irradianceX, float windX, float rainX, float temperatureX, float humidityX, float pressureX, bool outputX);
  
  PullData(long timeX, Frame f, String authNameX = "", String hexNonceX = "", String hexHashX = "");
  ~PullData();


  /*Getters & Setters*/
  long getTime();
  void setTime(long timeX);
  float getLoadCurrent();
  void setLoadCurrent(float loadCurrentX);
  void setLoadCurrent(int loadCurrentX, long resolution = RESOLUTION_MAX_VALUE, float resolution_voltage = RESOLUTION_VOLTAGE);
  float getBatteryCurrent();
  void setBatteryCurrent(float batteryCurrentX);
  void setBatteryCurrent(int batteryCurrentX, long resolution = RESOLUTION_MAX_VALUE, float resolution_voltage = RESOLUTION_VOLTAGE);
  float getPhotovoltaicCurrent();
  void setPhotovoltaicCurrent(float photovoltaicCurrentX);
  void setPhotovoltaicCurrent(int photovoltaicCurrentX, long resolution = RESOLUTION_MAX_VALUE, float resolution_voltage = RESOLUTION_VOLTAGE);
  float getLoadVoltage();
  void setLoadVoltage(float loadVoltageX);
  void setLoadVoltage(int loadVoltageX, long resolution = RESOLUTION_MAX_VALUE, float resolution_voltage = RESOLUTION_VOLTAGE);
  float getBatteryVoltage();
  void setBatteryVoltage(float batteryVoltageX);
  void setBatteryVoltage(int batteryVoltageX, long resolution = RESOLUTION_MAX_VALUE, float resolution_voltage = RESOLUTION_VOLTAGE);
  float getPhotovoltaicVoltage();
  void setPhotovoltaicVoltage(float photovoltaicVoltageX);
  void setPhotovoltaicVoltage(int photovoltaicVoltageX, long resolution = RESOLUTION_MAX_VALUE, float resolution_voltage = RESOLUTION_VOLTAGE);
  float getBatteryTemperature();
  void setBatteryTemperature(float batteryTemperatureX);
  float getPhotovoltaicTemperature();
  void setPhotovoltaicTemperature(float photovoltaicTemperatureX);
  float getIrradiance();
  void setIrradiance(float irradianceX);
  void setIrradiance(int irradianceX, long resolution = RESOLUTION_MAX_VALUE, float resolution_voltage = RESOLUTION_VOLTAGE);
  float getWind();
  void setWind(float windX);
  float getRain();
  void setRain(float rainX);
  float getTemperature();
  void setTemperature(float temperatureX);
  float getHumidity();
  void setHumidity(float humidityX);
  float getPressure();
  void setPressure(float pressureX);
  bool getOutput();
  void setOutput(bool outputX);
  String getNodeName();
  void setNodeName(String nodeNameX);
  String getHexNonce();
  void setHexNonce(String hexNonceX);
  String getHexHash();
  void setHexHash(String hexHashX);
  String getVerification();
  void setVerification(String verificationX);

  void setCorrections();

  void clear();

  String toJSON();
  void toJSONPointer(String *json);
  void fromJSON(String json);

  String toSD();
  void fromSD(String sd_string);
  bool verifyFromSD();


private:
  //time
  long time;
  //current values
  float loadCurrent;
  float batteryCurrent;
  float photovoltaicCurrent;
  //voltage values
  float loadVoltage;
  float batteryVoltage;
  float photovoltaicVoltage;
  //temperature values
  float batteryTemperature;
  float photovoltaicTemperature;
  //meteorologic values
  float irradiance;
  float wind;
  float rain;
  float temperature;
  float humidity;
  float pressure;
  //output values
  bool output;
  //node values
  String nodeName = "";
  String hexHash = "";
  String hexNonce = "";

  String verification = "";
};

//------------------------------------------------
//-----------------PULLDATAPOOL-------------------
//------------------------------------------------

class PullDataPool{
public:
    PullDataPool();
    ~PullDataPool();

    bool push(PullData pullData); //return true if ok, false if pool is full
    PullData peek();
    PullData pop();
    PullData popFirst();
    PullData pullDataAt(uint16_t index);
    
    void remove(uint16_t index);

    uint16_t count();


    bool isFull();
    bool isEmpty();

protected:
    struct pullDataPool{
        bool havePullData = false;
        PullData pullData;
    } pullDataPool[SIZE_PULLDATA_POOL];
};


#endif

//------------------------------------------------
//-----------------NODEINFO-----------------------
//------------------------------------------------

#define NODENAME_SIZE 8
#define NODEPASS_SIZE 8
#define NODE_MASTER false
#define NODE_SLAVE true
#define MIN_SAMPLE_RATE 20

class NodeInfo{
public:
    NodeInfo(bool isSlaveX = NODE_SLAVE);
    NodeInfo(long ZBAddressHighX, long ZBAddressLowX, bool isSlaveX = NODE_SLAVE);
    NodeInfo(String json);
    ~NodeInfo();

    //GETTERS & SETTERS
    void setZBAddressHigh(long ZBAddressHighX);
    long getZBAddressHigh();
    
    void setZBAddressLow(long ZBAddressLowX);
    long getZBAddressLow();

    void setSampleRate(uint16_t sampleRateX);
    uint16_t getSampleRate();

    void setNodeName(String nodeNameX);
    void getNodeName(char nodeNameX[]);
    String getNodeName();

    void setNodePass(String nodePassX);
    String getNodePass();

    void setNetworkId(unsigned int networkIdX);
    unsigned int getNetworkId();

    void setAuthName(String authNameX);
    String getAuthName();
    void setAuthPass(String authPassX);
    String getAuthPass();

    void setIsSlave(bool isSlaveX);
    bool getIsSlave();

    void setOutputValue(int outputValueX);
    void setOutputValue(bool outputValueX);
    bool getOutputValue();

    void setSSID(String ssidX);
    String getSSID();

    void setSSIDPassword(String ssidPaswordX);
    String getSSIDPassword();

    void setValidated(bool validatedX);
    bool getValidated();

    void setServer(char* serverX);
    char* getServer();

    void clear();
    bool equalsTo(NodeInfo n);
    bool simpleEqualsTo(NodeInfo n);    //comprueba la dirección y el nodename

    bool checkAddress(long msb, long lsb);
    bool checkAddress(NodeInfo n);
    bool checkNodeName(NodeInfo n);
    
    void setParametersByFrame(Frame f);

    String toJSON();
    String getNodeNameJSON();
private:
    long ZBAddressHigh = 0;
    long ZBAddressLow = 0;

    uint16_t sampleRate = MIN_SAMPLE_RATE;

    String nodeName = "";
    String nodePass = "";

    uint16_t networkId = 0;

    String authName = "";
    String authPass = "";

    String ssid;
    String ssidPassword;
    char* server;

    bool isSlave = NODE_SLAVE;

    bool outputValue = false;

    bool validated = false;     //usado por los slave para saber si han sido validados por el servidor

};


#ifdef ARDUINO_ARCH_ESP32

//------------------------------------------------
//-----------------NODEINFOPOOL-------------------
//------------------------------------------------


#define SIZE_NODEINFO_POOL 30

#define ADDRESSES_EQUALS 2
#define NODE_EQUALS 1
#define NODE_NOT_FOUND -1

class NodeInfoPool{
public:
    NodeInfoPool();
    ~NodeInfoPool();

    bool push(NodeInfo nodeInfo); //return true if ok, false if pool is full
    NodeInfo peek();
    NodeInfo pop();
    NodeInfo nodeInfoAt(uint8_t index);
    uint8_t count();
    bool isFull();
    bool isEmpty();

    void updateNodeInfo(uint8_t index, NodeInfo updatedNode);

    void remove(uint8_t index);

    bool checkIfValidToBeAdded(NodeInfo n); //check if a node was added with the same nodename. If yes return true
    int8_t checkIfAdded(NodeInfo n);
    int8_t checkIfAdded(long msb, long lsb);
    int8_t checkIfAdded(String nodeNameX);

protected:
    struct nodeInfoPool{
        bool haveNodeInfo = false;
        NodeInfo nodeInfo;
    } nodeInfoPool[SIZE_NODEINFO_POOL];
};

#endif //ARDUINO_ARCH_ESP32

#endif  /* TFMPROTOCOL_H */