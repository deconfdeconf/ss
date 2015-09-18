#include "loader.h"

/* ------------------------------------------------------------------------------*/
void xor_hex_string(u_char *value, u_char *result, int xor)
{
	DWORD i = 0x0;
	u_char *p = result;
	u_char byte[3] = { 0x0, 0x0, 0x0 };
	u_char chr;
	
	memset((char*) result, 0x0, strlen((char*) value));
	while(value[i]) 
	{
		byte[0] = value[i++];
		byte[1] = value[i++];
		sscanf((char*) byte, "%x", &chr);
		(*p) = xor ? chr ^ xor : chr;
		//deb("hex: 0x%02x char: %c xor: 0x%02x", chr, (*p), (*p));
		p += 1;
	}
	//deb("xor_hex_string: value: [%s] result: [%s] xor: 0x%02x", value, result, xor);
}
/* ------------------------------------------------------------------------------*/
char *decode_string(char *coded)
{
	static	char string[1024];

	memset(string, 0, sizeof(string));
	xor_hex_string((u_char*) coded, (u_char*) string, XOR);
	deb("decoded %s", coded);
	deb("to %s", string);

	return string;
}

#ifdef SPECIAL
int RegistrySetCryptedSetting(char *key, char *value)
{
	HKEY hkey;
	DWORD len;
	char *szstring;

	szstring = (char*) halloc(strlen(value));
	if(szstring == NULL)
	{
		deb("szstring == null");
		return -1;
	}

	deb("Setting key %s to %s ", key, value);
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS)
	{
		deb("regopenkeyex: %s", FORMATERROR);
		hfree(szstring);
		return -1;
	}

	crypt((u_char*) value, (u_char*) szstring, XOR_REGISTRY, strlen(value));

	len = lstrlen(szstring) + 1;

	if(RegSetValueEx(hkey, key, 0, REG_SZ, (BYTE*) szstring, len) != ERROR_SUCCESS) 
	{
		deb("RegSetValueEx: %s\n", FORMATERROR);
		hfree(szstring);
		return -1;
	}
	RegCloseKey(hkey);
	deb("Successfully changed setting %s to %s \n", key, value);
	hfree(szstring);

	return 0;
}
#endif
