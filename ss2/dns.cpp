#include "StdAfx.h"

/// Some dns and named shit

int resolve(char* Host) 
{
	DWORD addr;
	struct hostent *he;

	addr = inet_addr(Host);

	if(addr != INADDR_NONE) {
		return addr;
	}

	he = gethostbyname((char FAR*) Host);
	
	if(he == NULL) {
		deb("resolve error: %d",WSAGetLastError());
		return NULL;
	}

	addr = ((struct in_addr *) (he->h_addr_list[0]))->s_addr;

	return addr;
}

int DnsResolve(char* Host) 
{
	DWORD addr;
	struct hostent *he;

	addr = inet_addr(Host);

	if(addr != INADDR_NONE) {
		return addr;
	}

	he = gethostbyname((char FAR*) Host);
	
	if(he == NULL) {
		deb("resolve error: %d",WSAGetLastError());
		return ERR;
	}

	addr = ((struct in_addr *) (he->h_addr_list[0]))->s_addr;

	return addr;
}

void DoDns( char *what, char *buffer)
{
	HOSTENT *hostent = NULL;
	IN_ADDR iaddr;

	DWORD addr = inet_addr(what);
	
	if (addr != INADDR_NONE) {
        hostent = gethostbyaddr((char *)&addr, sizeof(struct in_addr), AF_INET);
        if (hostent != NULL)
			wsprintf(buffer, "%s",hostent->h_name);
	} else {
        hostent = gethostbyname(what);
        if (hostent != NULL) {
			iaddr = *((LPIN_ADDR)*hostent->h_addr_list);
			wsprintf(buffer, "%s", inet_ntoa(iaddr));
        }
	}
	if (hostent == NULL)
        wsprintf(buffer, "unable to resolve host");

	return;
}