#include "stdafx.h"

#define DECODED_STRING_SIZE 1024

DWORD g_dwTlsIndex;

/* ------------------------------------------------------------------------------*/
char *decode_hex_string(char *coded)
{
	char *szString, *mem;
	DWORD dwHeapSize=0;
//	int result;

	if (TlsGetValue(g_dwTlsIndex) == NULL) 
	{ 
		mem = (char*) halloc(DECODED_STRING_SIZE);
		if(mem == NULL)
		{
			deb("dhs: fail to alloc memory for szString");
			return NULL;
		}
		/*deb("dhs: allocated %d bytes for szString at 0x%p", 
			DECODED_STRING_SIZE, mem);*/
		TlsSetValue(g_dwTlsIndex, (LPVOID) mem);
		szString = mem;
	} 
	else
	{
		szString = (char*) TlsGetValue(g_dwTlsIndex);
		/*deb("dhs: memory for szString at 0x%p", szString);*/
	}
#ifndef FRELEASE
	if(IsBadWritePtr((LPVOID) szString, DECODED_STRING_SIZE))
	{
		deb("dhs: szString is bad write pointer");
		dwHeapSize = HeapSize(GetProcessHeap(), 0, szString);
		deb("dhs: HeapSize(%d) say it is %d bytes long", 
			(int) dwHeapSize, dwHeapSize);
		deb("dhs: validating heap...");
		if(!HeapValidate(GetProcessHeap(), 0, NULL))
		{
			deb("dhs: heap corrupt!");
		}
		else
		{
			deb("dhs: heap OK");
		}
	}
#endif
	memset(szString, 0, DECODED_STRING_SIZE);
	xor_hex_string((u_char*) coded, (u_char*) szString, XOR);
	/*deb("decoded %s", coded);
	deb("to %s", szString);*/

	return szString;
} 