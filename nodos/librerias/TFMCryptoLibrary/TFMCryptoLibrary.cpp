#include "TFMCryptoLibrary.h"

TFMCrypto::TFMCrypto(bool logger){
    if(logger == true){
        log = true;
        Serial.begin(115200);
    }


    hash = &sha256;

    rngCalibrating = false;
    rngLength = 48; // First block should wait for the pool to fill up.

	RNG.begin(RNG_APP_TAG);
	RNG.addNoiseSource(noiseSource);
}

TFMCrypto::~TFMCrypto(){
}


size_t TFMCrypto::getRNGLength(){
    return rngLength;
}


void TFMCrypto::randomNumberGenerator(uint8_t *data, int dataLength) {
    // Track changes to the calibration state on the noise source.
    bool newCalibrating = noiseSource.calibrating();
    if (newCalibrating != rngCalibrating) {
        rngCalibrating = newCalibrating;
        if (rngCalibrating && log)
            Serial.println(F("TFMCRYPTO> calibrating"));
    }

    // Perform regular housekeeping on the random number generator.
    RNG.loop();

    // Generate output whenever 32 bytes of entropy have been accumulated.
    // The first time through, we wait for 48 bytes for a full entropy pool.
    if(RNG.available(rngLength)) {
        RNG.rand(data, dataLength);
        rngLength = 32;
        if(log){
            Serial.print(F("TFMCRYPTO> RandomNumber: "));
            for(int i = 0; i < dataLength; i++){
              Serial.print(data[i], HEX);
              Serial.print(" ");
            }
            Serial.println();
        }
    }
}



void TFMCrypto::sha256Hash(char *text, int textSize, uint8_t *value, int valueSize){
  hash->reset();
  hash->update(text, textSize);
  hash->finalize(value, valueSize);
}

void TFMCrypto::sha256Hash(const char *text, int textSize, uint8_t *value, int valueSize){
  hash->reset();
  hash->update(text, textSize);
  hash->finalize(value, valueSize);
}


String TFMCrypto::encryptAesCBC(String keyString, String ivString, String plainText){
    CBC<AESSmall128> cbc;

    size_t len = plainText.length();

    if(log){
        Serial.println(F("TFMCRYPTO.> Encrypting..."));
        Serial.print(F("TFMCRYPTO> len:"));
        Serial.println(len);
    }

    size_t lenPadded = ((len+CBC_BLOCK_SIZE-1)/16)*16;
    if(log){
        Serial.print(F("TFMCRYPTO> lenPadded:"));
        Serial.println(lenPadded);
    }

    size_t base64Len = ((4 * lenPadded / 3) + 3) & ~3;
    if(log){
        Serial.print(F("TFMCRYPTO> base64Len:"));
        Serial.println(base64Len);
    }
    uint8_t key[CBC_KEY_SIZE];
    uint8_t iv[CBC_IV_SIZE];
    uint8_t input[lenPadded];
    String output = "";

    if(log)
        Serial.print(F("TFMCRYPTO> key:"));

    for(int i = 0; i < CBC_KEY_SIZE; i++){
        key[i] = (uint8_t)keyString[i];

        if(log)
            Serial.print((char)key[i]);
    }
    
    if(log){
        Serial.println();
        Serial.print(F("TFMCRYPTO> iv:"));
    }

    for(int i = 0; i < CBC_IV_SIZE; i++){
        iv[i] = (uint8_t)ivString[i];
        if(log)
            Serial.print((char)iv[i]);
    }
    if(log){
        Serial.println();
        Serial.print(F("TFMCRYPTO> plain:"));
    }
    for(int i = 0; i < lenPadded; i++){
        if(i < len){  //pongo relleno
            input[i] = (uint8_t)plainText[i];
        } else {
            input[i] = PAD_CHAR;
        }
        if(log)
            Serial.print((char)input[i]);
    }
    if(log)
        Serial.println();


    cbc.setKey(key, 16);
    cbc.setIV(iv, 16);
    cbc.encrypt(input, input, lenPadded);

    char inputString[sizeof(input)];
    for(int i = 0; i < sizeof(input); i++){
        inputString[i] = (char)input[i];
    }

    int encodedLength = Base64.encodedLength(lenPadded);
    char base64[encodedLength];
    Base64.encode(base64, inputString, lenPadded);

    if(log)
        Serial.print(F("TFMCRYPTO> output:"));
    for(int i = 0; i < base64Len; i++){
        output += base64[i];
        if(log)
            Serial.print(base64[i]);
    }
    if(log)
        Serial.println();

    cbc.clear();

    return output; 
}


bool TFMCrypto::randomNumberAvailable(){
    if(RNG.available(rngLength)){
        return true;
    } else {
        return false;
    }
}

bool TFMCrypto::takeNoise(uint16_t timeout){
    uint16_t cont = 0;
    while(randomNumberAvailable() == false && cont < timeout){
        cont++;
        RNG.loop();
    }
    if(cont >= timeout){
        return false;
    } else {
        if(randomNumberAvailable()){
            return true;
        } else {
            return false;
        }
    }

}

bool TFMCrypto::pseudoRandomNumberGenerateSeed(){
    if (randomNumberAvailable() == false) {
        if(log)
            Serial.println(F("TFMCrypto> Taking noise..."));
        takeNoise();
    }

    if(randomNumberAvailable()){
        uint8_t seed_bytes[4] = {0,0,0,0};
        randomNumberGenerator(seed_bytes, 4);

        unsigned long seed = 0;
        seed += seed_bytes[0] | (seed_bytes[1] << 8) | (seed_bytes[2] << 16) | (seed_bytes[3] << 24);

        if(log){
            Serial.print(F("TFMCrypto> Seed: "));
            Serial.println(seed);
        }

        randomSeed(seed);

        //reseteo el lifetime de la seed
        seed_lifetime = millis();

        return true;
    } else {
        randomSeed(DEFAULT_SEED);
        return false;
    }
}

void TFMCrypto::pseudoRandomNumberGenerator(uint8_t *data, int dataLength){
    for(uint8_t i = 0; i < dataLength; i++){
        data[i] = random(255);
    }
}

unsigned long TFMCrypto::getSeedLifetime(){
    return seed_lifetime;
}


const char PROGMEM _Base64AlphabetTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

int Base64Class::encode(char *output, char *input, int inputLength) {
    int i = 0, j = 0;
    int encodedLength = 0;
    unsigned char A3[3];
    unsigned char A4[4];

    while(inputLength--) {
        A3[i++] = *(input++);
        if(i == 3) {
            fromA3ToA4(A4, A3);

            for(i = 0; i < 4; i++) {
                output[encodedLength++] = pgm_read_byte(&_Base64AlphabetTable[A4[i]]);
            }

            i = 0;
        }
    }

    if(i) {
        for(j = i; j < 3; j++) {
            A3[j] = '\0';
        }

        fromA3ToA4(A4, A3);

        for(j = 0; j < i + 1; j++) {
            output[encodedLength++] = pgm_read_byte(&_Base64AlphabetTable[A4[j]]);
        }

        while((i++ < 3)) {
            output[encodedLength++] = '=';
        }
    }
    output[encodedLength] = '\0';
    return encodedLength;
}

int Base64Class::decode(char * output, char * input, int inputLength) {
    int i = 0, j = 0;
    int decodedLength = 0;
    unsigned char A3[3];
    unsigned char A4[4];


    while (inputLength--) {
        if(*input == '=') {
            break;
        }

        A4[i++] = *(input++);
        if (i == 4) {
            for (i = 0; i <4; i++) {
                A4[i] = lookupTable(A4[i]);
            }

            fromA4ToA3(A3,A4);

            for (i = 0; i < 3; i++) {
                output[decodedLength++] = A3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            A4[j] = '\0';
        }

        for (j = 0; j <4; j++) {
            A4[j] = lookupTable(A4[j]);
        }

        fromA4ToA3(A3,A4);

        for (j = 0; j < i - 1; j++) {
            output[decodedLength++] = A3[j];
        }
    }
    output[decodedLength] = '\0';
    return decodedLength;
}

int Base64Class::encodedLength(int plainLength) {
    int n = plainLength;
    return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

int Base64Class::decodedLength(char * input, int inputLength) {
    int i = 0;
    int numEq = 0;
    for(i = inputLength - 1; input[i] == '='; i--) {
        numEq++;
    }

    return ((6 * inputLength) / 8) - numEq;
}

//Private utility functions
inline void Base64Class::fromA3ToA4(unsigned char * A4, unsigned char * A3) {
    A4[0] = (A3[0] & 0xfc) >> 2;
    A4[1] = ((A3[0] & 0x03) << 4) + ((A3[1] & 0xf0) >> 4);
    A4[2] = ((A3[1] & 0x0f) << 2) + ((A3[2] & 0xc0) >> 6);
    A4[3] = (A3[2] & 0x3f);
}

inline void Base64Class::fromA4ToA3(unsigned char * A3, unsigned char * A4) {
    A3[0] = (A4[0] << 2) + ((A4[1] & 0x30) >> 4);
    A3[1] = ((A4[1] & 0xf) << 4) + ((A4[2] & 0x3c) >> 2);
    A3[2] = ((A4[2] & 0x3) << 6) + A4[3];
}

inline unsigned char Base64Class::lookupTable(char c) {
    if(c >='A' && c <='Z') return c - 'A';
    if(c >='a' && c <='z') return c - 71;
    if(c >='0' && c <='9') return c + 4;
    if(c == '+') return 62;
    if(c == '/') return 63;
    return -1;
}

Base64Class Base64;