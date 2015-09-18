#include "stdafx.h"

XorConnection::XorConnection(bool , char* , int)
{
	deb("SecureConnection class created(%s:%s)", host, port);
}

XorConnection::~XorConnection(void)
{
	deb("SecureConnection (%s:%s) class destroyed", host, port);
}

int XorConnection::connect(char *host, u_int port, u_int timeout)
{
	return 0;
}

void XorConnection::disconnect()//char *host, u_int port, u_int timeout)
{
	return ;
}

int XorConnection::recv(char* buffer, u_int len)
{
	return 0;
}

int XorConnection::send(char* buffer, u_int len)
{
	return 0;
}

int XorConnection::close(void)
{
	return 0;
}