#include "TFMUtils.h"

String hexToString(const uint8_t *data, unsigned len, bool withCeros)
{
  static char const hexchars[] = "0123456789ABCDEF";
  String s = "";
  while (len > 0) {
    int b = *data++;
    
    if(((b >> 4) & 0x0F) == 0){
      if(withCeros == true){
        s += hexchars[(b >> 4) & 0x0F];
        s += hexchars[b & 0x0F];
      }
      --len;
    }else{ 
      s += hexchars[(b >> 4) & 0x0F];
      s += hexchars[b & 0x0F];
      --len;
    }
  }
  return s;
}

int charToInt(char c){
  switch(c){
    case '0':
      return 0;
    case'1':
      return 1;
    case'2':
      return 2;
    case'3':
      return 3;
    case'4':
      return 4;
    case'5':
      return 5;
    case'6':
      return 6;
    case'7':
      return 7;
    case'8':
      return 8;
    case'9':
      return 9;
    default:
      return -1;
  }
}


Blink::Blink(uint8_t ledPinInfoX, uint8_t ledPinErrorX){
  ledPinInfo = ledPinInfoX;
  pinMode(ledPinInfo, OUTPUT);
  ledPinError = ledPinErrorX;
  pinMode(ledPinError, OUTPUT);
}

void Blink::info(String info){
  for(uint8_t i = 0; i < info.length(); i++){
    digitalWrite(ledPinInfo, HIGH);
    if(info.charAt(i) == '.'){
      delay(BLINK_SHORT);
    } else {
      delay(BLINK_LONG);
    }
    digitalWrite(ledPinInfo, LOW);
    if(i < info.length()-1){
      delay(BLINK_DELAY);
    }
  }
  delay(BLINK_DELAY_END);
}

void Blink::error(String error){
  for(uint8_t i = 0; i < error.length(); i++){
    digitalWrite(ledPinError, HIGH);
    if(error.charAt(i) == '.'){
      delay(BLINK_SHORT);
    } else {
      delay(BLINK_LONG);
    }
    digitalWrite(ledPinError, LOW);
    if(i < error.length()-1){
      delay(BLINK_DELAY);
    }
  }
  delay(BLINK_DELAY_END);
}


#ifdef ARDUINO_ARCH_ESP32

//-------------SD CONFIGURATION---------------------
SDManager::SDManager(bool debugEnabledX){
  debugEnabled = debugEnabledX;
}

void SDManager::begin(fs::FS *fs_aux){
  fs = fs_aux;
  countFiles();
}

bool SDManager::listDir(const char * dirname, uint8_t levels){
  if(debugEnabled){
    Serial.print(F("SDManager> Listing directory: ")); Serial.println(dirname);
  }

  File root = fs->open(dirname);
  if(!root){
    Serial.println(F("SDManager> Failed to open directory"));
    return false;
  }
  if(!root.isDirectory()){
    Serial.println(F("SDManager> Not a directory"));
    return false;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print(F("  DIR : "));
      Serial.println(file.name());
      if(levels){
        listDir(file.name(), levels -1);
      }
    } else {
      Serial.print(F("  FILE: "));
      Serial.print(file.name());
      Serial.print(F("  SIZE: "));
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  return true;
}

long SDManager::countElementsInDir(const char * dirname){
  if(debugEnabled){
    Serial.print(F("SDManager> Counting files in directory: ")); Serial.println(dirname);
  }
  long num_files = 0;

  File root = fs->open(dirname);
  if(!root){
    if(debugEnabled)
      Serial.println(F("SDManager> Failed to open directory"));
    return -1;
  }
  if(!root.isDirectory()){
    if(debugEnabled)
      Serial.println(F("SDManager> Not a directory"));
    return -1;
  }

  File file = root.openNextFile();
  while(file){
    num_files++;
    file = root.openNextFile();
  }

  return num_files;
}

bool SDManager::createDir(const char * path){
  if(debugEnabled){
    Serial.print(F("SDManager> Creating directory: ")); Serial.println(path);
  }
  if(fs->mkdir(path)){
    if(debugEnabled)
      Serial.println(F("SDManager> Dir created"));
    return true;
  } else {
    if(debugEnabled)
      Serial.println(F("SDManager> mkdir failed"));
    return false;
  }
}

bool SDManager::removeDir(const char * path){
  if(debugEnabled){
    Serial.print(F("SDManager> Deleting directory: ")); Serial.println(path);
  }
  if(fs->rmdir(path)){
    if(debugEnabled)
      Serial.println(F("SDManager> Dir removed"));
    return true;
  } else {
    if(debugEnabled)
      Serial.println(F("SDManager> rmdir failed"));
    return false;
  }
}

String SDManager::readFile(const char * path){
  String s = "";
  if(debugEnabled){
    Serial.print(F("SDManager> Reading file: ")); Serial.println(path);
  }

  File file = fs->open(path);
  if(!file){
    if(debugEnabled)
      Serial.println(F("SDManager> Failed to open file for reading"));
    return s;
  }

  while(file.available()){
    s += (char)file.read();
  }
  file.close();
  if(debugEnabled){
    Serial.print(F("SDManager> Bytes readed: ")); Serial.println(s.length());
  }
  return s;
}

bool SDManager::writeFile(const char * path, const char * message){
  if(debugEnabled){
    Serial.print(F("SDManager> Writing file: ")); Serial.println(path);
  }

  File file = fs->open(path, FILE_WRITE );
  if(!file){
    if(debugEnabled)
      Serial.println(F("SDManager> Failed to open file for writing"));
    return false;
  }

  bool ok = false;
  
  if(file.print(message)){
    if(debugEnabled)
      Serial.println(F("SDManager> File written"));
    ok = true;
  } else if(debugEnabled){
    Serial.println(F("SDManager> Write failed"));
  }
  file.close();

  return ok;
}

bool SDManager::appendFile(const char * path, const char * message){
  if(debugEnabled){
    Serial.print(F("SDManager> Appending to file: ")); Serial.println(path);
  }

  File file = fs->open(path, FILE_APPEND);
  if(!file){
    if(debugEnabled){
      Serial.println(F("SDManager> Failed to open file for appending"));
    }
    return false;
  }

  bool ok = false;

  if(file.print(message)){
    if(debugEnabled)
      Serial.println(F("SDManager> Message appended"));
    ok = true;
  } else if(debugEnabled){
    Serial.println(F("SDManager> Append failed"));
  }
  file.close();
  return ok;
}

bool SDManager::renameFile(const char * path1, const char * path2){
  if(debugEnabled){
    Serial.print(F("SDManager> Renaming file: ")); Serial.println(path1); Serial.print(F(" to ")); Serial.println(path2);
  }

  if (fs->rename(path1, path2)) {
    if(debugEnabled)
      Serial.println(F("SDManager> File renamed"));
    return true;
  } else {
    if(debugEnabled)
      Serial.println(F("SDManager> Rename failed"));
    return false;
  }
}

bool SDManager::deleteFile(const char * path){
  if(debugEnabled){
    Serial.print(F("SDManager> Deleting file: ")); Serial.println(path);
  }
  if(fs->remove(path)){
    if(debugEnabled)
      Serial.println(F("SDManager> File deleted"));
    return true;
  } else {
    if(debugEnabled)
      Serial.println(F("SDManager> Delete failed"));
    return false;
  }
}

bool SDManager::newPullDataInSD(String pulldata_sd_string){
  long files_in_datalog_folder = countFiles();

  if(files_in_datalog_folder == -1){
    if(debugEnabled)
      Serial.println(F("SDManager> Error opening datalog folder"));
    return false;
  }

  String filename = String(SD_DATALOG_FOLDER) + "/" + String((files_in_datalog_folder+1));

  if(writeFile(filename.c_str(), pulldata_sd_string.c_str())){
    files_in_sd++;
    
    String files_in_sd_str = String(files_in_sd);
    writeFile(FILENAME_NUMBERFILES.c_str(), files_in_sd_str.c_str());
    return true;
  } else {
    return false;
  }
}

String SDManager::getLastPullDataFromSD(){
  String s = "";

  long files_in_datalog_folder = countFiles();

  if(files_in_datalog_folder == -1){
    if(debugEnabled)
      Serial.println(F("SDManager> Error opening datalog folder"));
    return "";
  } else {
    String filename = String(SD_DATALOG_FOLDER) + "/" + String(files_in_datalog_folder);
    
    if(fs->exists(filename.c_str())){
      s = readFile(filename.c_str());
    } else if(debugEnabled){
        Serial.print(F("SDManager> File: "));Serial.print(FILENAME_NUMBERFILES);Serial.println(F(" does not exists"));
    }
  }

  return s;
}

bool SDManager::deleteLastPullDataFromSD(){
  
  long files_in_datalog_folder = countFiles();

  if(files_in_datalog_folder == -1){
    if(debugEnabled)
      Serial.println(F("SDManager> Error opening datalog folder"));
    return false;
  } else if(files_in_datalog_folder > 0){

    String filename = String(SD_DATALOG_FOLDER) + "/" + String(files_in_datalog_folder);
    if(fs->exists(filename.c_str())){
      if(deleteFile(filename.c_str())){
        files_in_sd--;

        String files_in_sd_str = String(files_in_sd);
        writeFile(FILENAME_NUMBERFILES.c_str(), files_in_sd_str.c_str());
      } else {
        return false;
      }
    } else {
      if(debugEnabled){
        Serial.print(F("SDManager> File: "));Serial.print(FILENAME_NUMBERFILES);Serial.println(F(" does not exists"));
      }
      return false;
    }
  } else {
    if(debugEnabled)
      Serial.println(F("SDManager> There are no files to delete"));
    countFiles();   //cuenta los archivos para resetear el valor de files_in_sd;
    return false;
  }
}

bool SDManager::clearPullDatasFromSD(){
  long files_in_datalog_folder = countFiles();
  if(files_in_datalog_folder == -1){
    if(debugEnabled)
      Serial.println(F("SDManager> Error opening datalog folder"));
    return false;
  }
  while(files_in_datalog_folder > 0 && deleteLastPullDataFromSD()){
    files_in_datalog_folder--;
  }
  return true;
}

bool SDManager::isEmpty(){
  if(files_in_sd > 0){
    return false;
  } else {
    return true;
  }
}

long SDManager::countFiles(){
  if (files_in_sd <= 0){
    if(fs->exists(FILENAME_NUMBERFILES.c_str())){
      String s = readFile(FILENAME_NUMBERFILES.c_str());
      if(s.length() == 0){
        if(debugEnabled){
          Serial.print(F("SDManager> File: "));Serial.print(FILENAME_NUMBERFILES);Serial.println(F(" is empty"));
          Serial.println(F("SDManager> Counting files one by one"));
        }
        long files_num = countElementsInDir(SD_DATALOG_FOLDER);
        if(files_num == -1){
          if(debugEnabled)
            Serial.println(F("SDManager> Error counting files"));
          return -1;
        } else {
          files_in_sd = files_num;
          
          String files_in_sd_str = String(files_in_sd);
          writeFile(FILENAME_NUMBERFILES.c_str(), files_in_sd_str.c_str());
        }
      } else {
        files_in_sd = s.toInt();
      }
    } else {
      if(debugEnabled){
        Serial.print(F("SDManager> File: "));Serial.print(FILENAME_NUMBERFILES);Serial.println(F(" does not exists"));
        Serial.println(F("SDManager> Counting files one by one"));
      }
      long files_num = countElementsInDir(SD_DATALOG_FOLDER);
      if(files_num == -1){
        if(debugEnabled)
          Serial.println(F("SDManager> Error counting files"));
        return -1;
      } else {
        files_in_sd = files_num;
        
        String files_in_sd_str = String(files_in_sd);
        writeFile(FILENAME_NUMBERFILES.c_str(), files_in_sd_str.c_str());
      }
    }
  }
  return files_in_sd;
}

#endif