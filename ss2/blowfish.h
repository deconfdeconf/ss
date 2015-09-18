// blowfish.h     interface file for blowfish.cpp
// _THE BLOWFISH ENCRYPTION ALGORITHM_
// by Bruce Schneier
// Revised code--3/20/94
// Converted to C++ class 5/96, Jim Conger

#define MAXKEYBYTES 	56		// 448 bits max
#define NPASS           16		// SBox passes

#define DWORD  		unsigned long
#define WORD  		unsigned short
#define BYTE  		unsigned char


void blowfish_init(void);
void blowfish_free(void);
void 		Blowfish_encipher (DWORD *xl, DWORD *xr) ;
void 		Blowfish_decipher (DWORD *xl, DWORD *xr) ;
void 		blowfish_setkey (BYTE key[], int keybytes) ;
DWORD		getlen (DWORD lInputLong) ;
DWORD		blowfish_encrypt (BYTE * pInput, BYTE * pOutput, DWORD lSize) ;
void		blowfish_decrypt (BYTE * pInput, BYTE * pOutput, DWORD lSize) ;

union aword 
{
   DWORD dword;
   BYTE byte [4];
   struct 
   {
      unsigned int byte3:8;
      unsigned int byte2:8;
      unsigned int byte1:8;
      unsigned int byte0:8;
   } w;
};

