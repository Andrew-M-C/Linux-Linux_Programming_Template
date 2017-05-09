
#ifndef	__AES_H
#define	__AES_H

#include <stdio.h>
#include "AMCDataTypes.h"


#ifndef	BOOL
#define	BOOL	int
#endif

#ifndef	FALSE
#define	FALSE	(0)
#define	TRUE	(!FALSE)
#endif


#define	AES_INITIAL_VECTOR_SIZE		16
#define	AES_KEY_MAX_SIZE				(256 / 8)

#define	AES_KEY_BIT_LEN_128		128
#define AES_KEY_BIT_LEN_192		192
#define AES_KEY_BIT_LEN_256		256


size_t aesCbcRequiredEncryptBufferSize(size_t plainTextLen);
size_t aesCbcRequiredDecryptBufferSize(size_t cipherTextLen);

int aesCbcEncrypt(uint8_t *plainText, size_t plainTextLen, 
					uint8_t *key, size_t keyLenBit, 
					uint8_t *iv,
					uint8_t *cipherText, size_t cipherTextLen);

int aesCbcDecrypt(uint8_t *cipherText, size_t cipherTextLen, 
					uint8_t *key, size_t keyLenBit, 
					uint8_t *iv,
					uint8_t *plainText, size_t plainTextLen);


#endif

