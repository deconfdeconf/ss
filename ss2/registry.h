

enum { STORE_HEX, STORE_ASCII };

int RegistryGetCryptedSetting(char *key, char *value, int buflen);
int RegistrySetCryptedSetting(char *key, char *value);
int RegistryGetSetting(char *key, char *value, WORD vallen);
int RegistrySetSetting(char *key, char *value, WORD stype);