#include "TFMProtocol.h"


float bytesToFloat(uint8_t a[]){
  
  float f = 0;

  uint16_t i_uint16 = a[0] | (a[1] << 8);
  uint16_t d_uint16 = a[2] | (a[3] << 8);

  int i = (int)i_uint16;
  int d = (int)d_uint16;

  float i_float = ((float)i+0.0);
  float d_float = ((float)d+0.0)/10000;

  f = (float)i_float + (float)d_float;

  return f;
}

void floatTo4Bytes(float f, uint8_t a[]){
  int i = (int)(f);
  int d = (int)((f-i)*10000);

  uint16_t i_uint16 = (uint16_t)i;
  uint16_t d_uint16 = (uint16_t)d;

  uint8_t a1[2]={ i & 0xff, i >> 8 };
  uint8_t a2[2]={ d & 0xff, d >> 8 };

  a[0] = a1[0];
  a[1] = a1[1];
  a[2] = a2[0];
  a[3] = a2[1];
}



/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------FRAME-------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/

//Constructors
Frame::Frame(uint8_t payload[], uint8_t payloadLength, uint8_t frameCode, uint8_t frameId){
  clearData();

  setData(payload, payloadLength);
  code = frameCode;
  id = frameId;
}

Frame::Frame(uint8_t frameCode, uint8_t frameId){
  clearData();

  code = frameCode;
  id = frameId;
  length = 0;
}

Frame::Frame(){
	clearData();
}

Frame::Frame(uint8_t frame[], uint8_t frameLength){
  clearData();

  	if(frame[FRAME_PRELUDE_BYTE] == FRAME_PRELUDE){
		code = frame[CODE_BYTE];
		id = frame[ID_BYTE];
		if(frameLength > headerLength()){
			length = frameLength - headerLength();
			for(uint8_t i = 0; i < length; i++){
				setData(frame[headerLength()+i], i);
			}
		} else{
			length = 0;
		}
	}
}

Frame::Frame(String frame){
  clearData();

  	if((uint8_t)frame.charAt(FRAME_PRELUDE_BYTE) == FRAME_PRELUDE){
		code = (uint8_t)frame.charAt(CODE_BYTE);
		id = (uint8_t)frame.charAt(ID_BYTE);

		if(frame.length() > headerLength()){
			length = frame.length() - headerLength();
			uint8_t datos[length];

			for(uint8_t i = 0; i < length; i++){
				setData((uint8_t)frame.charAt(i+headerLength()),i);
			}
		} else{
			length = 0;
		}
	}
}

Frame::~Frame(){
	clearData();
}

/*Automatic frames*/
void Frame::createSampleFrame(int loadCurrent, int batteryCurrent, int photovoltaicCurrent, int loadVoltage, int batteryVoltage, int photovoltaicVoltage, float batteryTemperature, float photovoltaicTemperature, bool output, uint8_t idX){
	clearData();
	code = CODE_SAMPLE;
	id = idX;

	length = 17;

	uint16_t loadCurrent_uint16 = (uint16_t)loadCurrent;
	uint16_t batteryCurrent_uint16 = (uint16_t)batteryCurrent;
	uint16_t photovoltaicCurrent_uint16 = (uint16_t)photovoltaicCurrent;
	uint16_t loadVoltage_uint16 = (uint16_t)loadVoltage;
	uint16_t batteryVoltage_uint16 = (uint16_t)batteryVoltage;
	uint16_t photovoltaicVoltage_uint16 = (uint16_t)photovoltaicVoltage;

	uint8_t lc[2]={ loadCurrent_uint16 & 0xff, loadCurrent_uint16 >> 8 };
	uint8_t bc[2]={ batteryCurrent_uint16 & 0xff, batteryCurrent_uint16 >> 8 };
	uint8_t pc[2]={ photovoltaicCurrent_uint16 & 0xff, photovoltaicCurrent_uint16 >> 8 };
	uint8_t lv[2]={ loadVoltage_uint16 & 0xff, loadVoltage_uint16 >> 8 };
	uint8_t bv[2]={ batteryVoltage_uint16 & 0xff, batteryVoltage_uint16 >> 8 };
	uint8_t pv[2]={ photovoltaicVoltage_uint16 & 0xff, photovoltaicVoltage_uint16 >> 8 };

	int iBatteryTemperature = (int)(batteryTemperature);
	int dBatteryTemperature = (int)((batteryTemperature-iBatteryTemperature)*100);
  	
	int iPhotovoltaicTemperature = (int)(photovoltaicTemperature);
	int dPhotovoltaicTemperature = (int)((photovoltaicTemperature-iPhotovoltaicTemperature)*100);

	uint8_t bt[2] = { (uint8_t)iBatteryTemperature , (uint8_t) dBatteryTemperature };
	uint8_t pt[2] = { (uint8_t)iPhotovoltaicTemperature , (uint8_t) dPhotovoltaicTemperature };

	data[0] = lc[0];
	data[1] = lc[1];
	data[2] = bc[0];
	data[3] = bc[1];
	data[4] = pc[0];
	data[5] = pc[1];
	data[6] = lv[0];
	data[7] = lv[1];
	data[8] = bv[0];
	data[9] = bv[1];
	data[10] = pv[0];
	data[11] = pv[1];
	data[12] = bt[0];
	data[13] = bt[1];
	data[14] = pt[0];
	data[15] = pt[1];
	data[16] = output;
}

void Frame::createResponseAuthFrame(String nodeName, String nodePass, uint8_t idX){
	clearData();
	code = CODE_RESPONSE_AUTH;
	id = idX;

	length = NODENAME_SIZE + NODEPASS_SIZE;

	for(uint8_t i = 0; i < NODENAME_SIZE; i++){
		data[i] = (uint8_t)nodeName.charAt(i);
	}
	for(uint8_t i = 0; i < NODEPASS_SIZE; i++){
		data[NODENAME_SIZE+i] = (uint8_t)nodePass.charAt(i);
	}
}

void Frame::createResponseSampleRate(uint16_t sampleRate, uint8_t idX){
	clearData();
	code = CODE_RESPONSE_SAMPLERATE;
	id = idX;

	length = 2;

	uint8_t a[2]={ sampleRate & 0xff, sampleRate >> 8 };

	data[0] = a[1];
	data[1] = a[0];
}

void Frame::createResponseOutput(bool outputValue, uint8_t idX){
	clearData();
	code = CODE_RESPONSE_OUTPUT;
	length = 1;
	data[0] = outputValue;
}

void Frame::createResponseParameters(uint16_t sampleRate, bool outputValue, uint8_t idX){
	clearData();
	code = CODE_RESPONSE_PARAMETERS;
	id = idX;

	length = 3;

	uint8_t a[2]={ sampleRate & 0xff, sampleRate >> 8 & 0xff};

	//se guarda invertido
	data[0] = a[1];
	data[1] = a[0];

	data[2] = outputValue;
}

void Frame::createSetParameters(uint16_t sampleRate, bool outputValue, uint8_t idX){
	clearData();
	code = CODE_SET_PARAMETERS;
	id = idX;

	length = 3;

	uint8_t a[2]={ sampleRate & 0xff, sampleRate >> 8 & 0xff};

	//se envía invertido
	data[0] = a[1];
	data[1] = a[0];

	data[2] = outputValue;
}

/*Getters & Setters*/
void Frame::setCode(uint8_t codeX){
	code = codeX;
}
uint8_t Frame::getCode(){
	return code;
}
void Frame::setLength(uint8_t lengthX){
	length = lengthX;
}
uint8_t Frame::getLength(){
	return length;
}
void Frame::setId(uint8_t idX){
	id = idX;
}
uint8_t Frame::getId(){
	return id;
}
void Frame::setData(uint8_t value, uint8_t index){
	if(index >= 0 && index < MAX_LENGTH){
		data[index] = value;
	}
}

void Frame::setData(uint8_t payload[], uint8_t lengthX){
	if(lengthX >= 0 && lengthX < MAX_LENGTH){
		length = lengthX;
		for(uint8_t i = 0; i < length; i++){
			data[i] = payload[i];  
		}
	}	
}

void Frame::getData(uint8_t array[]){
	for(uint8_t i = 0; i < length; i++){
		array[i] = data[i];
	}
}
uint8_t Frame::getData(uint8_t index){
	return data[index];
}


void Frame::clear(){
	code = 0;
	id = 0;
	length = 0;
	for(uint8_t i = 0; i < MAX_LENGTH; i++){
		data[i] = 0;
	}
}

void Frame::clearData(){
	for(uint8_t i = 0; i < MAX_LENGTH; i++){
		data[i] = 0;
	}
}


bool Frame::equalsTo(Frame f){
  if(code == f.getCode() && length == f.getLength()){
    for(uint8_t i = 0; i < length; i++){
      if(data[i] != f.getData(i)){
        return false;
      }
    }
    return true;
  } else{
    return false;
  }
}

uint8_t Frame::validate(uint8_t frame[], uint8_t frameLength){
	clearData();
	if(frameLength >= headerLength()){
		if(frame[FRAME_PRELUDE_BYTE] == FRAME_PRELUDE){
			code = frame[CODE_BYTE];
			id = frame[ID_BYTE];

			uint8_t frameDataLength = frameLength - headerLength();

			if(code == CODE_SAMPLE && frameDataLength != CODE_SAMPLE_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_RESPONSE_AUTH && frameDataLength != CODE_RESPONSE_AUTH_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_RESPONSE_SAMPLERATE && frameDataLength != CODE_RESPONSE_SAMPLERATE_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_RESPONSE_OUTPUT && frameDataLength != CODE_RESPONSE_OUTPUT_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_RESPONSE_PARAMETERS && frameDataLength != CODE_RESPONSE_PARAMETERS_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_SET_SAMPLERATE && frameDataLength != CODE_SET_SAMPLERATE_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_REQUEST_SAMPLERATE && frameDataLength != CODE_REQUEST_SAMPLERATE_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_SET_OUTPUT && frameDataLength != CODE_SET_OUTPUT_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_REQUEST_OUTPUT && frameDataLength != CODE_REQUEST_OUTPUT_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_REQUEST_PARAMETERS && frameDataLength != CODE_REQUEST_PARAMETERS_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			} else if(code == CODE_SAMPLE_ACK && frameDataLength != CODE_SAMPLE_ACK_DATA_LENGTH){
				return FRAME_VALIDATION_SIZE_ERROR;
			}

			if(frameLength > headerLength()){
				length = frameLength - headerLength();
				for(uint8_t i = 0; i < length; i++){
					setData(frame[headerLength()+i], i);
				}
			} else{
				length = 0;
			}

			return FRAME_VALIDATION_OK;
		} else {
			return FRAME_VALIDATION_PRELUDE_ERROR;
		}
	} else {
		return FRAME_VALIDATION_SIZE_ERROR;
	}
}

void Frame::createFrame(uint8_t frame[]){
  frame[FRAME_PRELUDE_BYTE] = FRAME_PRELUDE;
  frame[CODE_BYTE] = code;
  frame[ID_BYTE] = id;
  for(uint8_t i = headerLength(); i < createFrameLength(); i++){
    frame[i] = data[i-headerLength()];
  }
}

uint8_t Frame::createFrameLength(){
  return headerLength() + length;
}

uint8_t Frame::headerLength(){
  return (FRAME_PRELUDE_SIZE + CODE_SIZE + ID_SIZE);
}


String Frame::toString(){
  String frame;
  uint8_t dataArray[createFrameLength()];
  createFrame(dataArray);
  for(uint8_t i = 0; i < createFrameLength(); i++){
      frame += dataArray[i];
  }
  return frame;
}

String Frame::dataToString(){
  String frame = "";
  for(uint8_t i = 0; i < length; i++){
    frame += (String)data[i];
  }
  return frame;
}

#ifndef ARDUINO_ARCH_ESP32		//SI NO ES ESP32


/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------FramePool----------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/


FramePool::FramePool(){
  for(uint8_t i = 0; i < SIZE_FRAME_POOL; i++){
    framePool[i].haveFrame = false;
  }
}

FramePool::~FramePool(){
	for(uint8_t i = 0; i < SIZE_FRAME_POOL; i++){
    framePool[i].haveFrame = false;
  }
}

bool FramePool::push(Frame frame){
  if(!isFull()){
    uint8_t top = count();
    framePool[top].frame = frame;
    framePool[top].haveFrame = true;
    return true;
  } else {
  	return false;
  }
}

Frame FramePool::peek(){
  return framePool[count()-1].frame;
}

Frame FramePool::pop(){
  uint8_t top = count()-1;
  framePool[top].haveFrame = false;
  return framePool[top].frame;
}

Frame FramePool::frameAt(uint8_t index){
  return framePool[index].frame;
}

uint8_t FramePool::count(){
  uint8_t i = 0;
  while(i < SIZE_FRAME_POOL){
    if(framePool[i].haveFrame == false){
      break;
    } else {
      i++;
    }
  }
  return i;
}

bool FramePool::isFull(){
  if(count() == SIZE_FRAME_POOL){
    return true;
  } else {
    return false;
  }
}

bool FramePool::isEmpty(){
  if(count() == 0){
    return true;
  } else {
    return false;
  }
}

Frame FramePool::get(uint8_t index){
	uint8_t top = count();
	if(top > index){
		Frame f = framePool[index].frame;
		for(uint8_t i = index; i < top; i++){
			if(i < SIZE_FRAME_POOL-1){
				framePool[i].frame = framePool[i+1].frame;
				framePool[i].haveFrame = framePool[i+1].haveFrame;
			} else {
				framePool[i].frame.clear();
				framePool[i].haveFrame = false;
			}
		}
		return f;
	}
}

void FramePool::remove(uint8_t index){
	uint8_t top = count();
	if(top > index){
		for(uint8_t i = index; i < top; i++){
			if(i < SIZE_FRAME_POOL-1){
				framePool[i].frame = framePool[i+1].frame;
				framePool[i].haveFrame = framePool[i+1].haveFrame;
			} else {
				framePool[i].frame.clear();
				framePool[i].haveFrame = false;
			}
		}
	}
}

int8_t FramePool::checkIfAdded(Frame f){
	uint8_t top = count();
	for(uint8_t i = 0; i < top; i++){
		if(framePool[i].frame.equalsTo(f)){
			return i;
		}
	}
	return FRAME_NOT_FOUND;
}

#endif


#ifdef ARDUINO_ARCH_ESP32

/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------PULLDATA----------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/

PullData::PullData(){
	clear();
}

PullData::PullData(String nodeNameX, String hexNonceX, String hexHashX, long timeX, float loadCurrentX, float batteryCurrentX, float photovoltaicCurrentX, float loadVoltageX, float batteryVoltageX, float photovoltaicVoltageX, float batteryTemperatureX, float photovoltaicTemperatureX, float irradianceX, float windX, float rainX, float temperatureX, float humidityX, float pressureX, bool outputX){
	clear();

	//node values
	nodeName = nodeNameX;
	hexNonce = hexNonceX;
	hexHash = hexHashX;
	//time
	time = timeX;
	//current values
	loadCurrent = loadCurrentX;
	batteryCurrent = batteryCurrentX;
	photovoltaicCurrent = photovoltaicCurrentX;
	//voltage values
	loadVoltage = loadVoltageX;
	batteryVoltage = batteryVoltageX;
	photovoltaicVoltage = photovoltaicVoltageX;
	//temperature values
	batteryTemperature = batteryTemperatureX;
	photovoltaicTemperature = photovoltaicTemperatureX;
	//meteorologic values
	irradiance = irradianceX;
	wind = windX;
	rain = rainX;
	temperature = temperatureX;
	humidity = humidityX;
	pressure = pressureX;
	//output values
	output = outputX;
}

PullData::PullData(long timeX, Frame f, String nodeNameX, String hexNonceX, String hexHashX){
	clear();

	if(f.getCode() == CODE_SAMPLE){
		//node values
		nodeName = nodeNameX;
		hexNonce = hexNonceX;
		hexHash = hexHashX;
		//time
		time = timeX;

		int16_t loadCurrent_uint16 = f.getData((uint8_t)0) | (f.getData((uint8_t)1) << 8);
		int16_t batteryCurrent_uint16 = f.getData((uint8_t)2) | (f.getData((uint8_t)3) << 8);
		int16_t photovoltaicCurrent_uint16 = f.getData((uint8_t)4) | (f.getData((uint8_t)5) << 8);
		int16_t loadVoltage_uint16 = f.getData((uint8_t)6) | (f.getData((uint8_t)7) << 8);
		int16_t batteryVoltage_uint16 = f.getData((uint8_t)8) | (f.getData((uint8_t)9) << 8);
		int16_t photovoltaicVoltage_uint16 = f.getData((uint8_t)10) | (f.getData((uint8_t)11) << 8);

		setLoadCurrent((int) loadCurrent_uint16);
		setBatteryCurrent((int) batteryCurrent_uint16);
		setPhotovoltaicCurrent((int) photovoltaicCurrent_uint16);
		setLoadVoltage((int) loadVoltage_uint16);
		setBatteryVoltage((int) batteryVoltage_uint16);
		setPhotovoltaicVoltage((int) photovoltaicVoltage_uint16);


		int8_t i_batteryTemperature = ((int8_t)f.getData((uint8_t)12));
		int8_t d_batteryTemperature = ((int8_t)f.getData((uint8_t)13));
		batteryTemperature = (float)i_batteryTemperature+(float)d_batteryTemperature/100;
		int8_t i_photovoltaicTemperature = ((int8_t)f.getData((uint8_t)14));
		int8_t d_photovoltaicTemperature = ((int8_t)f.getData((uint8_t)15));
		photovoltaicTemperature = (float)i_photovoltaicTemperature+(float)d_photovoltaicTemperature/100;

		//output values
		output = f.getData((uint8_t)16);
	}
}

PullData::~PullData(){
	clear();
}


/*GETTERS & SETTERS*/
long PullData::getTime(){
	return time;
}
void PullData::setTime(long timeX){
	time = timeX;
}
float PullData::getLoadCurrent(){
	return loadCurrent;
}
void PullData::setLoadCurrent(float loadCurrentX){
	loadCurrent = loadCurrentX;
}
//LTSR 6-NP +-2
void PullData::setLoadCurrent(int loadCurrentX, long resolution, float resolution_voltage){
	loadCurrent = (float)loadCurrentX*0.59;
	//loadCurrent = ((float)loadCurrentX/resolution)*resolution_voltage*((float)2/0.625)*1000*3;
									//voltios * 2A / 0.625v *1000milis * correción
}

float PullData::getBatteryCurrent(){
	return batteryCurrent;
}
void PullData::setBatteryCurrent(float batteryCurrentX){
	batteryCurrent = batteryCurrentX;
}
//LTSR 6-NP +-6
void PullData::setBatteryCurrent(int batteryCurrentX, long resolution, float resolution_voltage){
	batteryCurrent = (float)batteryCurrentX*0.6;
										//x * 6A / 0.625v *1000milis * correción
	//batteryCurrent = ((float)batteryCurrentX/resolution)*resolution_voltage*((float)6/0.625)*1000*((float)2/3);
}
float PullData::getPhotovoltaicCurrent(){
	return photovoltaicCurrent;
}
void PullData::setPhotovoltaicCurrent(float photovoltaicCurrentX){
	photovoltaicCurrent = photovoltaicCurrentX;
}
//LTSR 15-NP +-5
void PullData::setPhotovoltaicCurrent(int photovoltaicCurrentX, long resolution, float resolution_voltage){
	photovoltaicCurrent = (float)photovoltaicCurrentX*1.46;
	//photovoltaicCurrent = ((float)photovoltaicCurrentX/resolution)*resolution_voltage*((float)5/0.625)*1000;
}
float PullData::getLoadVoltage(){
	return loadVoltage;
}
void PullData::setLoadVoltage(float loadVoltageX){
	loadVoltage = loadVoltageX;
}
void PullData::setLoadVoltage(int loadVoltageX, long resolution, float resolution_voltage){
	loadVoltage = (float)loadVoltageX/103;
	//loadVoltage = ((float)loadVoltageX/resolution)*resolution_voltage;
	//loadVoltage *= (2045/3);
}
float PullData::getBatteryVoltage(){
	return batteryVoltage;
}
void PullData::setBatteryVoltage(float batteryVoltageX){
	batteryVoltage = batteryVoltageX;
}
void PullData::setBatteryVoltage(int batteryVoltageX, long resolution, float resolution_voltage){
	batteryVoltage = (float)batteryVoltageX/104;
	//batteryVoltage = ((float)batteryVoltageX/resolution)*resolution_voltage;
	//batteryVoltage *= (2045/3);
}
float PullData::getPhotovoltaicVoltage(){
	return photovoltaicVoltage;
}
void PullData::setPhotovoltaicVoltage(float photovoltaicVoltageX){
	photovoltaicVoltage = photovoltaicVoltageX;
}
void PullData::setPhotovoltaicVoltage(int photovoltaicVoltageX, long resolution, float resolution_voltage){
	photovoltaicVoltage = (float)photovoltaicVoltageX/70;
	//photovoltaicVoltage = ((float)photovoltaicVoltageX/resolution)*resolution_voltage;
	//photovoltaicVoltage *= (2045/2);
}
float PullData::getBatteryTemperature(){
	return batteryTemperature;
}
void PullData::setBatteryTemperature(float batteryTemperatureX){
	batteryTemperature = batteryTemperatureX;
}
float PullData::getPhotovoltaicTemperature(){
	return photovoltaicTemperature;
}
void PullData::setPhotovoltaicTemperature(float photovoltaicTemperatureX){
	photovoltaicTemperature = photovoltaicTemperatureX;
}
float PullData::getIrradiance(){
	return irradiance;
}
void PullData::setIrradiance(float irradianceX){
	irradiance = irradianceX;
}
void PullData::setIrradiance(int irradianceX, long resolution, float resolution_voltage){
	irradiance = (((float)irradianceX/resolution)*resolution_voltage);
								//voltios*((W/m2)/voltios)
	irradiance *= 100;

}
float PullData::getWind(){
	return wind;
}
void PullData::setWind(float windX){
	wind = windX;
}
float PullData::getRain(){
	return rain;
}
void PullData::setRain(float rainX){
	rain = rainX;
}
float PullData::getTemperature(){
	return temperature;
}
void PullData::setTemperature(float temperatureX){
	temperature = temperatureX;
}
float PullData::getHumidity(){
	return humidity;
}
void PullData::setHumidity(float humidityX){
	humidity = humidityX;
}
float PullData::getPressure(){
	return pressure;
}
void PullData::setPressure(float pressureX){
	pressure = pressureX;
}
bool PullData::getOutput(){
	return output;
}
void PullData::setOutput(bool outputX){
	output = outputX;
}
String PullData::getNodeName(){
	return nodeName;
}
void PullData::setNodeName(String nodeNameX){
	nodeName = nodeNameX;
}
String PullData::getHexNonce(){
	return hexNonce;
}
void PullData::setHexNonce(String hexNonceX){
	hexNonce = hexNonceX;
}
String PullData::getHexHash(){
	return hexHash;
}
void PullData::setHexHash(String hexHashX){
	hexHash = hexHashX;
}
String PullData::getVerification(){
	return verification;
}
void PullData::setVerification(String verificationX){
	verification = verificationX;
}

void PullData::setCorrections(){
	loadCurrent *= 1;
	batteryCurrent *= 1;
	photovoltaicCurrent *= 1;
	loadVoltage *= 1;
	batteryVoltage *= 1;
	photovoltaicVoltage *= 1;
	irradiance *= 1;
	wind *= 1;
	rain *= 1;
}

void PullData::clear(){
	nodeName = "";
  hexHash = "";
  hexNonce = "";
	time = 0;
	loadCurrent = 0;
	batteryCurrent = 0;
	photovoltaicCurrent = 0;
	loadVoltage = 0;
	batteryVoltage = 0;
	photovoltaicVoltage = 0;
	batteryTemperature = 0;
	photovoltaicTemperature = 0;
	irradiance = 0;
	wind = 0;
	rain = 0;
	temperature = 0;
	humidity = 0;
	pressure = 0;
	output = 0;
	verification = "";
}


String PullData::toJSON(){

	String json = "{";
	json += "\"name\":\"" + nodeName + "\",";
	json += "\"rand\":\"" + hexNonce + "\",";
	json += "\"hash\":\"" + hexHash + "\",";
	json += "\"time\":\"" + (String) time + "\",";
	json += "\"cl\":\"" + String(loadCurrent, 3) + "\",";
	json += "\"cb\":\"" + String(batteryCurrent, 3) + "\",";
	json += "\"cp\":\"" + String(photovoltaicCurrent, 3) + "\",";
	json += "\"vl\":\"" + String(loadVoltage, 3) + "\",";
	json += "\"vb\":\"" + String(batteryVoltage, 3) + "\",";
	json += "\"vp\":\"" + String(photovoltaicVoltage, 3) + "\",";
	json += "\"i\":\"" + String(irradiance, 3) + "\",";
	json += "\"r\":\"" + String(rain, 3) + "\",";
	json += "\"w\":\"" + String(wind, 3) + "\",";
	json += "\"h\":\"" + String(humidity, 3) + "\",";
	json += "\"t\":\"" + String(temperature, 3) + "\",";
	json += "\"p\":\"" + String(pressure, 3) + "\",";
	json += "\"tb\":\"" + String(batteryTemperature, 3) + "\",";
	json += "\"tp\":\"" + String(photovoltaicTemperature, 3) + "\",";
	json += "\"out\":\"" + (String) output + "\",";
	json += "\"ver\":\"" + verification + "\"";
	json += "}";

	return json;
}

void PullData::toJSONPointer(String *json){

	*json = "{";
	*json += "\"name\":\"" + nodeName + "\",";
	*json += "\"rand\":\"" + hexNonce + "\",";
	*json += "\"hash\":\"" + hexHash + "\",";
	*json += "\"time\":\"" + (String) time + "\",";
	*json += "\"cl\":\"" + String(loadCurrent, 3) + "\",";
	*json += "\"cb\":\"" + String(batteryCurrent, 3) + "\",";
	*json += "\"cp\":\"" + String(photovoltaicCurrent, 3) + "\",";
	*json += "\"vl\":\"" + String(loadVoltage, 3) + "\",";
	*json += "\"vb\":\"" + String(batteryVoltage, 3) + "\",";
	*json += "\"vp\":\"" + String(photovoltaicVoltage, 3) + "\",";
	*json += "\"i\":\"" + String(irradiance, 3) + "\",";
	*json += "\"r\":\"" + String(rain, 3) + "\",";
	*json += "\"w\":\"" + String(wind, 3) + "\",";
	*json += "\"h\":\"" + String(humidity, 3) + "\",";
	*json += "\"t\":\"" + String(temperature, 3) + "\",";
	*json += "\"p\":\"" + String(pressure, 3) + "\",";
	*json += "\"tb\":\"" + String(batteryTemperature, 3) + "\",";
	*json += "\"tp\":\"" + String(photovoltaicTemperature, 3) + "\",";
	*json += "\"out\":\"" + (String) output + "\",";
	*json += "\"ver\":\"" + verification + "\"";
	*json += "}";

}

void PullData::fromJSON(String json){

	nodeName = json.substring(json.indexOf("\"name\":\"")+String("\"name\":\"").length(),json.indexOf(",\"rand\":\"")-1);
	json = json.substring(json.indexOf(",\"rand\":\""));

	hexNonce = json.substring(json.indexOf(",\"rand\":\"")+String(",\"rand\":\"").length(),json.indexOf(",\"hash\":\"")-1);
	json = json.substring(json.indexOf(",\"rand\":\""));

	hexHash = json.substring(json.indexOf(",\"hash\":\"")+String(",\"hash\":\"").length(),json.indexOf(",\"time\":\"")-1);
	json = json.substring(json.indexOf(",\"time\":\""));

	time = json.substring(json.indexOf(",\"time\":\"")+String(",\"time\":\"").length(),json.indexOf(",\"cl\":\"")-1).toInt();
	json = json.substring(json.indexOf(",\"cl\":\""));

	loadCurrent = json.substring(json.indexOf(",\"cl\":\"")+String(",\"cl\":\"").length(),json.indexOf(",\"cb\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"cb\":\""));

	batteryCurrent = json.substring(json.indexOf(",\"cb\":\"")+String(",\"cb\":\"").length(),json.indexOf(",\"cp\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"cp\":\""));

	photovoltaicCurrent = json.substring(json.indexOf(",\"cp\":\"")+String(",\"cp\":\"").length(),json.indexOf(",\"vl\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"vl\":\""));

	loadVoltage = json.substring(json.indexOf(",\"vl\":\"")+String(",\"vl\":\"").length(),json.indexOf(",\"vb\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"vb\":\""));

	batteryVoltage = json.substring(json.indexOf(",\"vb\":\"")+String(",\"vb\":\"").length(),json.indexOf(",\"vp\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"vp\":\""));

	photovoltaicVoltage = json.substring(json.indexOf(",\"vp\":\"")+String(",\"vp\":\"").length(),json.indexOf(",\"i\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"i\":\""));

	irradiance = json.substring(json.indexOf(",\"i\":\"")+String(",\"i\":\"").length(),json.indexOf(",\"r\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"r\":\""));

	rain = json.substring(json.indexOf(",\"r\":\"")+String(",\"r\":\"").length(),json.indexOf(",\"w\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"w\":\""));

	wind = json.substring(json.indexOf(",\"w\":\"")+String(",\"w\":\"").length(),json.indexOf(",\"h\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"h\":\""));

	humidity = json.substring(json.indexOf(",\"h\":\"")+String(",\"h\":\"").length(),json.indexOf(",\"t\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"t\":\""));

	temperature = json.substring(json.indexOf(",\"t\":\"")+String(",\"t\":\"").length(),json.indexOf(",\"p\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"p\":\""));

	pressure = json.substring(json.indexOf(",\"p\":\"")+String(",\"p\":\"").length(),json.indexOf(",\"tb\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"tb\":\""));

	batteryTemperature = json.substring(json.indexOf(",\"tb\":\"")+String(",\"tb\":\"").length(),json.indexOf(",\"tp\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"tp\":\""));

	photovoltaicTemperature = json.substring(json.indexOf(",\"tp\":\"")+String(",\"tp\":\"").length(),json.indexOf(",\"out\":\"")-1).toFloat();
	json = json.substring(json.indexOf(",\"out\":\""));

	if(json.substring(json.indexOf(",\"out\":\"")+String(",\"out\":\"").length(),json.indexOf(",\"ver\":\"")-1).equals("1")){
		output = true;	
	} else {
		output = false;
	}
	
	json = json.substring(json.indexOf(",\"ver\":\""));

	verification = json.substring(json.indexOf(",\"ver\":\"")+String(",\"ver\":\"").length(),json.indexOf("}")-1);
}


String PullData::toSD(){
	String s = "";
	s += nodeName + ",";
	s += hexNonce + ",";
	s += hexHash + ",";
	s += (String) time + ",";
	s += String(loadCurrent, 3) + ",";
	s += String(batteryCurrent, 3) + ",";
	s += String(photovoltaicCurrent, 3) + ",";
	s += String(loadVoltage, 3) + ",";
	s += String(batteryVoltage, 3) + ",";
	s += String(photovoltaicVoltage, 3) + ",";
	s += String(irradiance, 3) + ",";
	s += String(rain, 3) + ",";
	s += String(wind, 3) + ",";
	s += String(humidity, 3) + ",";
	s += String(temperature, 3) + ",";
	s += String(pressure, 3) + ",";
	s += String(batteryTemperature, 3) + ",";
	s += String(photovoltaicTemperature, 3) + ",";
	s += (String) output + ",";
	s += verification;
	
	return s;
}

void PullData::fromSD(String sd_string){
	String splitter_string = sd_string;
	int comma_index = splitter_string.indexOf(",");

	nodeName = splitter_string.substring(0,comma_index);
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");

	hexNonce = splitter_string.substring(0,comma_index);
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");

	hexHash = splitter_string.substring(0,comma_index);
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");

	time  = (long)splitter_string.substring(0,comma_index).toDouble();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");

	loadCurrent = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");

	batteryCurrent = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	photovoltaicCurrent = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	loadVoltage = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	batteryVoltage = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	photovoltaicVoltage = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	irradiance = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	rain = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	wind = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	humidity = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	temperature = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	pressure = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	batteryTemperature = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	photovoltaicTemperature = splitter_string.substring(0,comma_index).toFloat();
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	String output_str = splitter_string.substring(0,comma_index);
	splitter_string = splitter_string.substring(comma_index+1, splitter_string.length());
	comma_index = splitter_string.indexOf(",");
	
	if(output_str == "1"){
		output = true;
	} else {
		output = false;
	}

	verification = splitter_string;
}

bool PullData::verifyFromSD(){
	bool r = true;
	if(nodeName.length() <= 0 || hexNonce.length() <= 0 || hexHash.length() <= 0 || time <= 0 || verification.length() <= 0)
		r = false;

	return r;
}


/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-----------------------------------PULLDATAPOOL--------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/

PullDataPool::PullDataPool(){
  for(uint16_t i = 0; i < SIZE_PULLDATA_POOL; i++){
    pullDataPool[i].havePullData = false;
  }
}

PullDataPool::~PullDataPool(){
	for(uint16_t i = 0; i < SIZE_PULLDATA_POOL; i++){
    pullDataPool[i].havePullData = false;
  }
}

bool PullDataPool::push(PullData pullData){
  if(!isFull()){
    uint16_t top = count();
    pullDataPool[top].pullData = pullData;
    pullDataPool[top].havePullData = true;
    return true;
  } else {
  	return false;
  }
}

PullData PullDataPool::peek(){
  return pullDataPool[count()-1].pullData;
}

PullData PullDataPool::pop(){
  uint16_t top = count()-1;
  pullDataPool[top].havePullData = false;
  return pullDataPool[top].pullData;
}

PullData PullDataPool::popFirst(){
  PullData p = pullDataPool[0].pullData;

  for(uint16_t i = 0; i < count(); i++){
  	if(i < SIZE_PULLDATA_POOL-1){
	  	pullDataPool[i].pullData.clear();
	  	pullDataPool[i].pullData = pullDataPool[i+1].pullData;
	  	pullDataPool[i].havePullData = pullDataPool[i+1].havePullData;
	  } else {
			pullDataPool[SIZE_PULLDATA_POOL-1].pullData.clear();
			pullDataPool[SIZE_PULLDATA_POOL-1].havePullData = false;
		}
  }

  return p;
}

PullData PullDataPool::pullDataAt(uint16_t index){
  return pullDataPool[index].pullData;
}

void PullDataPool::remove(uint16_t index){
	uint16_t top = count();
	if(top > index){
		for(uint16_t i = index; i < top; i++){
			if(i < SIZE_PULLDATA_POOL-1){
				pullDataPool[i].pullData.clear();
				pullDataPool[i].pullData = pullDataPool[i+1].pullData;
				pullDataPool[i].havePullData = pullDataPool[i+1].havePullData;
			} else {
				pullDataPool[i].pullData.clear();
				pullDataPool[i].havePullData = false;
			}
		}
	}
}

uint16_t PullDataPool::count(){
  uint16_t i = 0;
  while(i < SIZE_PULLDATA_POOL){
    if(pullDataPool[i].havePullData == false){
      break;
    } else {
      i++;
    }
  }
  return i;
}

bool PullDataPool::isFull(){
  if(count() == SIZE_PULLDATA_POOL){
    return true;
  } else {
    return false;
  }
}

bool PullDataPool::isEmpty(){
  if(count() == 0){
    return true;
  } else {
    return false;
  }
}


#endif //#ifdef ARDUINO_ARCH_ESP32

/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-----------------------------------NODEINFO--------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/

NodeInfo::NodeInfo(bool isSlaveX){
	clear();
	isSlave = isSlaveX;
}

NodeInfo::NodeInfo(String json){
	clear();

}

NodeInfo::NodeInfo(long ZBAddressHighX, long ZBAddressLowX, bool isSlaveX){
	clear();
	isSlave = isSlaveX;

	ZBAddressHigh = ZBAddressHighX;
	ZBAddressLow = ZBAddressLowX;
}

NodeInfo::~NodeInfo(){
	clear();
}

void NodeInfo::setZBAddressHigh(long ZBAddressHighX){
	ZBAddressHigh = ZBAddressHighX;
}

long NodeInfo::getZBAddressHigh(){
	return ZBAddressHigh;
}

void NodeInfo::setZBAddressLow(long ZBAddressLowX){
	ZBAddressLow = ZBAddressLowX;
}
long NodeInfo::getZBAddressLow(){
	return ZBAddressLow;
}

void NodeInfo::setSampleRate(uint16_t sampleRateX){
	if(sampleRateX >= MIN_SAMPLE_RATE)
		sampleRate = sampleRateX;
}

uint16_t NodeInfo::getSampleRate(){
	return sampleRate;
}

void NodeInfo::setNodeName(String nodeNameX){
	nodeName = nodeNameX;
}

void NodeInfo::getNodeName(char nodeNameX[]){
	for(uint8_t i = 0; i < NODENAME_SIZE; i++){
		nodeNameX[i] = nodeName.charAt(i);
	}
}

String NodeInfo::getNodeName(){
	return nodeName;
}

void NodeInfo::setNodePass(String nodePassX){
	nodePass = nodePassX;
}

String NodeInfo::getNodePass(){
	return nodePass;
}

void NodeInfo::setIsSlave(bool isSlaveX){
	isSlave = isSlaveX;
}
bool NodeInfo::getIsSlave(){
	return isSlave;
}

void NodeInfo::setOutputValue(int outputValueX){
	if(outputValueX == 1){
		outputValue = true;
	} else {
		outputValue = false;
	}
}
void NodeInfo::setOutputValue(bool outputValueX){
	outputValue = outputValueX;
}
bool NodeInfo::getOutputValue(){
	return outputValue;
}


void NodeInfo::setAuthName(String authNameX){
	authName = authNameX;
}

String NodeInfo::getAuthName(){
	return authName;
}

void NodeInfo::setAuthPass(String authPassX){
	authPass = authPassX;
}

String NodeInfo::getAuthPass(){
	return authPass;
}

void NodeInfo::setNetworkId(unsigned int networkIdX){
	networkId = networkIdX;
}

unsigned int NodeInfo::getNetworkId(){
	return networkId;
}

void NodeInfo::setSSID(String ssidX){
	ssid = ssidX;
}
String NodeInfo::getSSID(){
	return ssid;
}

void NodeInfo::setSSIDPassword(String ssidPasswordX){
	ssidPassword = ssidPasswordX;
}
String NodeInfo::getSSIDPassword(){
	return ssidPassword;
}

void NodeInfo::setValidated(bool validatedX){
	validated = validatedX;
}
bool NodeInfo::getValidated(){
	return validated;    	
}

void NodeInfo::setServer(char* serverX){
	server = serverX;
}
char* NodeInfo::getServer(){
	return server;
}

void NodeInfo::clear(){
  ZBAddressHigh = 0;

  ZBAddressLow = 0;

  sampleRate = 0;

  nodeName = "";
  nodePass = "";

  networkId = 0;

  authName = "";
  authPass = "";
}

bool NodeInfo::simpleEqualsTo(NodeInfo n){
	if(ZBAddressHigh == n.getZBAddressHigh() && 
			ZBAddressLow == n.getZBAddressLow() && 
			nodeName.compareTo(n.getNodeName()) == 0){
		return true;
	}
	return false;
}

bool NodeInfo::equalsTo(NodeInfo n){
	if(ZBAddressHigh == n.getZBAddressHigh() && 
			ZBAddressLow == n.getZBAddressLow() && 
			nodeName.compareTo(n.getNodeName()) == 0 && 
			nodePass.compareTo(n.getNodePass()) == 0 &&
			sampleRate == n.getSampleRate() &&
			authName.compareTo(n.getAuthName()) == 0 &&
			authPass.compareTo(n.getAuthPass()) == 0){
		return true;
	}
	return false;
}

bool NodeInfo::checkAddress(long msb, long lsb){
	if(ZBAddressHigh == msb && ZBAddressLow == lsb){
		return true;
	}
	return false;
}

bool NodeInfo::checkAddress(NodeInfo n){
	if(ZBAddressHigh == n.getZBAddressHigh() && ZBAddressLow == n.getZBAddressLow()){
		return true;
	}
	return false;
}

bool NodeInfo::checkNodeName(NodeInfo n){
	if(nodeName.equals(n.getNodeName())){
		return true;
	} else {
		return false;
	}
}

void NodeInfo::setParametersByFrame(Frame f){
	if(f.getCode() == CODE_RESPONSE_AUTH){
		for(uint8_t i = 0; i < NODENAME_SIZE; i++){
			nodeName += (char)f.getData(i);
		}
		for(uint8_t i = 0; i < NODEPASS_SIZE; i++){
			nodePass += (char)f.getData(NODENAME_SIZE+i);
		}
	} else if(f.getCode() == CODE_RESPONSE_SAMPLERATE){
		uint16_t newSampleRate = f.getData((uint8_t)0)*256 +f.getData((uint8_t)1);
		if(newSampleRate >= MIN_SAMPLE_RATE){
			sampleRate = newSampleRate;
		}
	} else  if(f.getCode() == CODE_RESPONSE_OUTPUT){
		uint8_t newOutputValue = f.getData((uint8_t)2);
		if(newOutputValue == 1){
			outputValue = true;
		} else {
			outputValue = false;
		}
	} else if(f.getCode() == CODE_RESPONSE_PARAMETERS){
		uint16_t newSampleRate = f.getData((uint8_t)0)*256 +f.getData((uint8_t)1);
		if(newSampleRate >= MIN_SAMPLE_RATE){
			sampleRate = newSampleRate;
		}
		uint8_t newOutputValue = f.getData((uint8_t)2);
		if(newOutputValue == 1){
			outputValue = true;
		} else {
			outputValue = false;
		}
	} else if(f.getCode() == CODE_SET_PARAMETERS){
		uint16_t newSampleRate = f.getData((uint8_t)0)*256 +f.getData((uint8_t)1);
		if(newSampleRate >= MIN_SAMPLE_RATE){
			sampleRate = newSampleRate;
		}
		uint8_t newOutputValue = f.getData((uint8_t)2);
		if(newOutputValue == 1){
			outputValue = true;
		} else {
			outputValue = false;
		}
		
	}
}


String NodeInfo::toJSON(){
	
	String json = "{";
	json += "\"n\":\"" + nodeName + "\",";
	json += "\"m\":\"" + (String) ZBAddressHigh + "\",";
	json += "\"l\":\"" + (String) ZBAddressLow + "\",";
	json += "\"s\":\"" + (String) sampleRate + "\",";
	json += "\"o\":\"" + (String) outputValue + "\"";
	json += "}";

	return json;
}

String NodeInfo::getNodeNameJSON(){
	
	String json = "{";
	json += "\"n\":\"" + nodeName + "\"";
	json += "}";

	return json;
}

#ifdef ARDUINO_ARCH_ESP32


/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-----------------------------------NODEINFOPOOL--------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/

NodeInfoPool::NodeInfoPool(){
  for(uint8_t i = 0; i < SIZE_NODEINFO_POOL; i++){
    nodeInfoPool[i].haveNodeInfo = false;
  }
}

NodeInfoPool::~NodeInfoPool(){
	for(uint8_t i = 0; i < SIZE_NODEINFO_POOL; i++){
    nodeInfoPool[i].haveNodeInfo = false;
  }
}

bool NodeInfoPool::push(NodeInfo nodeInfo){
  if(!isFull()){
    uint8_t top = count();
    nodeInfoPool[top].nodeInfo = nodeInfo;
    nodeInfoPool[top].haveNodeInfo = true;
    return true;
  } else {
  	return false;
  }
}

NodeInfo NodeInfoPool::peek(){
  return nodeInfoPool[count()-1].nodeInfo;
}

NodeInfo NodeInfoPool::pop(){
  uint8_t top = count()-1;
  nodeInfoPool[top].haveNodeInfo = false;
  return nodeInfoPool[top].nodeInfo;
}

NodeInfo NodeInfoPool::nodeInfoAt(uint8_t index){
  return nodeInfoPool[index].nodeInfo;
}

uint8_t NodeInfoPool::count(){
  uint8_t i = 0;
  while(i < SIZE_NODEINFO_POOL){
    if(nodeInfoPool[i].haveNodeInfo == false){
      break;
    } else {
      i++;
    }
  }
  return i;
}

bool NodeInfoPool::isFull(){
  if(count() == SIZE_NODEINFO_POOL){
    return true;
  } else {
    return false;
  }
}

bool NodeInfoPool::isEmpty(){
  if(count() == 0){
    return true;
  } else {
    return false;
  }
}

void NodeInfoPool::remove(uint8_t index){
	uint8_t top = count();
	if(top > index){
		for(uint8_t i = index; i < top; i++){
			if(i < SIZE_NODEINFO_POOL-1){
				nodeInfoPool[i].nodeInfo.clear();
				nodeInfoPool[i].nodeInfo = nodeInfoPool[i+1].nodeInfo;
				nodeInfoPool[i].haveNodeInfo = nodeInfoPool[i+1].haveNodeInfo;
			} else {
				nodeInfoPool[i].nodeInfo.clear();
				nodeInfoPool[i].haveNodeInfo = false;
			}
		}
	}
}

void NodeInfoPool::updateNodeInfo(uint8_t index, NodeInfo updatedNode){
	uint8_t top = count();
	if(top > index){
		nodeInfoPool[index].nodeInfo = updatedNode;
	}
}

bool NodeInfoPool::checkIfValidToBeAdded(NodeInfo n){
	uint8_t top = count();
	for(uint8_t i = 0; i < top; i++){
		if(nodeInfoPool[i].nodeInfo.checkNodeName(n) == true){
			return false;
		}
	}
	return true;
}

int8_t NodeInfoPool::checkIfAdded(NodeInfo n){
	uint8_t top = count();
	for(uint8_t i = 0; i < top; i++){
		if(nodeInfoPool[i].nodeInfo.checkAddress(n)){
			return i;
		}
	}
	return NODE_NOT_FOUND;
}

int8_t NodeInfoPool::checkIfAdded(long msb, long lsb){
	uint8_t top = count();
	for(uint8_t i = 0; i < top; i++){
		if(nodeInfoPool[i].nodeInfo.checkAddress(msb, lsb)){
			return i;
		}
	}
	return NODE_NOT_FOUND;
}

int8_t NodeInfoPool::checkIfAdded(String nodeNameX){
	uint8_t top = count();
	for(uint8_t i = 0; i < top; i++){
		if(nodeInfoPool[i].nodeInfo.getNodeName().equals(nodeNameX)){
			return i;
		}
	}
	return NODE_NOT_FOUND;
}

#endif	//#ifdef ARDUINO_ARCH_ESP32
