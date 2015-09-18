#include "stdafx.h"

int RegistryGetSetting(char *key, char *value, WORD vallen)
{
	HKEY hkey;
	DWORD len, type;

	deb("Checking setting %s...", key);
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
		return -1;

	len = 255;
	if(RegQueryValueEx(hkey, key, 0, &type, (LPBYTE) value, &len) != ERROR_SUCCESS) {
		deb("RegQuery for coreserver: %s\n", FORMATERROR);
		return -1;
	}

	RegCloseKey(hkey);

	deb("Setting %s = %s\n", key, value);
	return len;
}

int RegistryGetCryptedSetting(char *key, char *value, int buflen = 255)
{
	HKEY hkey;
	DWORD len, type;
	char *crypted;

	crypted = (char*) halloc(buflen);
	if(crypted == NULL)
	{
		deb("crypted == null");
		return -1;
	}
	deb("registry: checking setting %s...", key);
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
	{
		hfree(crypted);
		return -1;
	}

	len = buflen;
	if(RegQueryValueEx(hkey, key, 0, &type, (LPBYTE) crypted, &len) != ERROR_SUCCESS) 
	{
		deb("RegQuery for coreserver: %s\n", FORMATERROR);
		hfree(crypted);
		return -1;
	}

	RegCloseKey(hkey);

	deb("setting %s = %s\n", key, crypted);
	decrypt((u_char*) crypted, (u_char*) value, XOR);
	deb("decrypted setting %s = %s\n", key, value);
	hfree(crypted);

	return len;
}

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

int RegistrySetSetting(char *key, char *value, WORD stype)
{
	HKEY hkey;
	DWORD len;
	char core_string[255];

	deb("Setting key %s to %s ", key, value);
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS)
		return -1;

	if(stype == STORE_ASCII)
		strncpy(core_string, value, 255);
	else if(stype == STORE_HEX) 
		string_hex_xor((u_char*) value, (u_char*) core_string, XOR_REGISTRY, strlen(value));

	len = lstrlen(core_string) + 1;

	if(RegSetValueEx(hkey, key, 0, REG_SZ, (BYTE*) core_string, len) != ERROR_SUCCESS) {
		deb("RegSetValueEx: %s\n", FORMATERROR);
		return -1;
	}
	RegCloseKey(hkey);
	deb("Successfully changed setting %s to %s \n", key, value);
	return 0;
}


