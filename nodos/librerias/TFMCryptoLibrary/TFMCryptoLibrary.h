#ifndef TFMCryptoLibrary_H
#define TFMCryptoLibrary_H

#include "Arduino.h"

#include <string.h>

#include <Crypto.h>
#include <AES.h>
#include <CBC.h>
#include <RNG.h>
#include <SHA256.h>
#include <TransistorNoiseSource.h>


#define HASH_SIZE 32
#define BLOCK_SIZE 64
#define RNG_SIZE 32

#define CBC_IV_SIZE 16
#define CBC_KEY_SIZE 16
#define CBC_BLOCK_SIZE 16


#define NOISE_PIN 35
#define RNG_APP_TAG "SecureNumbers"

#define PAD_CHAR '%'

#define DEFAULT_SEED 1234567890
#define MAX_SEED_LIFETIME 60000
										 

class TFMCrypto {
public:
	TFMCrypto(bool logger = false);
	~TFMCrypto();

	void randomNumberGenerator(uint8_t *data, int dataLength);
	void sha256Hash(char *text, int textSize, uint8_t *value, int valueSize);
	void sha256Hash(const char *text, int textSize, uint8_t *value, int valueSize);
	String encryptAesCBC(String keyString, String ivString, String plainText);
	size_t getRNGLength();
	bool randomNumberAvailable();
	bool takeNoise(uint16_t timeout = 10000);

	bool pseudoRandomNumberGenerateSeed();	//return true if ok
	void pseudoRandomNumberGenerator(uint8_t *data, int dataLength);
	unsigned long getSeedLifetime();


private:
	bool log;

	SHA256 sha256;

	bool rngCalibrating = false;
	size_t rngLength = 48; // First block should wait for the pool to fill up.

	TransistorNoiseSource noiseSource = TransistorNoiseSource(NOISE_PIN);
	Hash *hash;

	unsigned long seed_lifetime = 0; //indica el tiempo de vida de la seed
};


class Base64Class{
  public:
    int encode(char *output, char *input, int inputLength);
    int decode(char * output, char * input, int inputLength);
    int encodedLength(int plainLength);
    int decodedLength(char * input, int inputLength);

  private:
    inline void fromA3ToA4(unsigned char * A4, unsigned char * A3);
    inline void fromA4ToA3(unsigned char * A3, unsigned char * A4);
    inline unsigned char lookupTable(char c);
};
extern Base64Class Base64;

#endif