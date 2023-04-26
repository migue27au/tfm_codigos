#ifndef TFMUtils_H
#define TFMUtils_H

#include "Arduino.h"
#include <string.h>

#ifdef ARDUINO_ARCH_ESP32
	#include "FS.h"
	#include "SD.h"
	#include "SPI.h"
#endif

String hexToString(const uint8_t *data, unsigned len, bool withCeros);
int charToInt(char c);


#define BLINK_DELAY 100
#define BLINK_DELAY_END 0
#define BLINK_SHORT 50
#define BLINK_LONG 100


//errors
#define ERROR_CONVERSION_CURRENT ".-"	//a
#define ERROR_CONVERSION_VOLTAGE "-..."	//b
#define ERROR_I2C_DEVICES "-.-."		//c
#define ERROR_NO_WIFI "-.."				//d
#define ERROR_MET_NOT_AVAILABLE "."		//e
#define ERROR_PULLDATAPOOL_FULL "..-."	//f
#define ERROR_ZIGBEE_ERROR "--."		//g
#define ERROR_FRAME_PRELUDE "...."		//h
#define ERROR_SERVER_TIME_SYNC ".."	//i
#define ERROR_SERVER_RESPONSE_CODE ".---"	//j
#define ERROR_NODE_NOT_CONFIGURED "-.-"	//k
#define ERROR_NODE_INVALID_PARAMETERS ".-.."	//l
#define ERROR_NODE_CANNOT_BE_ADDED_TO_NODES "--" //m
#define ERROR_NODE_CANNOT_BE_ADDED_TO_NODES_BECAUSE_ANOTHER_NODE_WITH_SAME_NODENAME_EXISTS "-." //n
#define ERROR_FRAME_VALIDATION "---"	//o
#define ERROR_RTC_NOT_RUNNING ".--."	//p
#define ERROR_SD_MOUNT "--.-"			//q
#define ERROR_SERVER_CREATE_CONFLICT ".-."	//r
#define ERROR_SERVER_CREATE_PARTIAL "..."	//s
#define ERROR_ADS "-"	//t
#define ERROR_XBEE_NOT_RUNNING "..-"	//u
#define ERROR_SERVER_NOT_AVAILABLE "...-"	//v

//info
#define INFO_STARTING ".-"				//a
#define INFO_CONNECTED_TO_WIFI "-..." 	//b
#define INFO_ZB_ATCMD_RESPONSE "-.-."	//c
#define INFO_ZB_RX_RESPONSE "-.."		//d
#define INFO_ZIGBEE_PACKET_RECEIVED "."	//e
#define INFO_ZB_TXSTAT_RESPONSE "..-."	//f
#define INFO_ZB_MODEMSTAT_RESPONSE "--."	//g
#define INFO_FRAME_CODE_SAMPLE_RECEIVED "...."	//h
#define INFO_FRAME_CODE_RESPONSE_PARAMETERS_RECEIVED ".."	//i
#define INFO_FRAME_CODE_RESPONSE_SAMPLERATE_RECEIVED ".---"	//j
#define INFO_FRAME_CODE_RESPONSE_OUTPUT_RECEIVED "-.-"	//k
#define INFO_FRAME_CODE_RESPONSE_AUTH_RECEIVED ".-.."	//l
#define INFO_FRAME_NODE_NO_AUTHENTICATED "--"	//m
#define INFO_SERVER_TIME "-."	//n
#define INFO_SERVER_TIME_SYNC "---"	//o
#define INFO_SERVER_CREATE ".--."	//p
#define INFO_SERVER_CREATE_DATA_SENDED "--.-"	//q
#define INFO_SERVER_INFO ".-."	//r
#define INFO_SERVER_INFO_DATA_SENDED "..."	//s
#define INFO_SERVER_GET "-"	//t
#define INFO_SERVER_GET_MASTER_VALUES "..-"	//u
#define INFO_SERVER_GET_SLAVE_VALUES "...-"	//v
#define INFO_READING_DATA ".--"	//w
#define INFO_NODE_ADDED_TO_NODES "-.--"	//y
#define INFO_SERVER_TEST "--.."	//z
#define INFO_PULLDATA_TO_SD ".----"	//1
#define INFO_REBOOTING_ESP32 "..---"	//2


class Blink{
public:
	Blink(uint8_t ledPinInfoX, uint8_t ledPinErrorX);
	void info(String info);
	void error(String error);
private:
	uint8_t ledPinInfo = 0;
	uint8_t ledPinError = 0;
};




#ifdef ARDUINO_ARCH_ESP32

#define SD_DATALOG_FOLDER "/datalog"

class SDManager{
public:
	SDManager(bool debugEnabledX = false);

	void begin(fs::FS * fs_aux);
	bool newPullDataInSD(String pulldata_sd_string);
	String getLastPullDataFromSD();
	bool deleteLastPullDataFromSD();
	bool clearPullDatasFromSD();
	bool isEmpty();
	long countFiles();


private:
	bool listDir(const char * dirname, uint8_t levels);
	long countElementsInDir(const char * dirname);
	bool createDir(const char * path);
	bool removeDir(const char * path);
	String readFile(const char * path);
	bool writeFile(const char * path, const char * message);
	bool appendFile(const char * path, const char * message);
	bool renameFile(const char * path1, const char * path2);
	bool deleteFile(const char * path);

	String FILENAME_NUMBERFILES = "/num_of_files.txt";
	fs::FS * fs;
	bool debugEnabled = false;
	long files_in_sd = 0;
};

#endif

#endif