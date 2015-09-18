#include "stdafx.h"
/*
BOOL Capture(char *file)
{
	HDC hdc;
	HBITMAP bitmap;
	BITMAPINFO bmpinfo;
	LPVOID pBits;
	HDC hdc2;
	DWORD dwWidth, dwHeight, dwBPP, dwNumColors;
	HGDIOBJ gdiobj;
	HANDLE hfile;
	DWORD dwBytes;	
	
	hdc = GetDC(NULL);
	if(hdc==NULL) 
	{
		deb("CreateDC failed : %s",FORMATERROR);
		return FALSE;
	}
	dwWidth = GetDeviceCaps(hdc, HORZRES);
	dwHeight = GetDeviceCaps(hdc, VERTRES);
	dwBPP = GetDeviceCaps(hdc, BITSPIXEL);
	if(dwBPP<=8) 
	{
		dwNumColors = GetDeviceCaps(hdc, NUMCOLORS);
		dwNumColors = 256;
	} else 
	{
		dwNumColors = 0;
	}
	hdc2 = CreateCompatibleDC(hdc);
	if( hdc2 == NULL ) 
	{
		deb("CreateCompatibleDC failed : %s",FORMATERROR);
		DeleteDC(hdc);
		return FALSE;
	}
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = dwWidth;
	bmpinfo.bmiHeader.biHeight = dwHeight;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = 0;
	bmpinfo.bmiHeader.biCompression = BI_JPEG;
	bmpinfo.bmiHeader.biSizeImage = dwHeight * dwWidth + 1;
	bmpinfo.bmiHeader.biXPelsPerMeter = 0;
	bmpinfo.bmiHeader.biYPelsPerMeter = 0;
	bmpinfo.bmiHeader.biClrUsed = 0;
	bmpinfo.bmiHeader.biClrImportant = 0;

	bitmap = CreateDIBSection(hdc, &bmpinfo, DIB_RGB_COLORS, &pBits, NULL, 0);

	if(bitmap==NULL) 
	{
		deb("CreateDIBSection failed : %s",FORMATERROR);
		DeleteDC(hdc);
		DeleteDC(hdc2);
		return FALSE;
	}
	gdiobj = SelectObject(hdc2, (HGDIOBJ)bitmap);
	if((gdiobj==NULL) || (gdiobj==(void *)GDI_ERROR)) 
	{
		deb("SelectObject failed : %s",FORMATERROR);
		DeleteDC(hdc);
		DeleteDC(hdc2);
		return FALSE;
	}
	deb("hdc2: %x dwWidth: %d dwHeight: %d hdc: %x", hdc2, dwWidth, dwHeight, hdc);
	if (!BitBlt(hdc2, 0,0, dwWidth, dwHeight, hdc, 0,0, SRCCOPY)) 
	{
		deb("BitBlt failed : %s",FORMATERROR);
		DeleteDC(hdc);
		DeleteDC(hdc2);
		return FALSE;
	}	
	RGBQUAD colors[256];
	if(dwNumColors!=0) 
		dwNumColors = GetDIBColorTable(hdc2, 0, dwNumColors, colors);

	BITMAPFILEHEADER bitmapfileheader;
	BITMAPINFOHEADER bitmapinfoheader;
	bitmapfileheader.bfType = 0x4D42;
	bitmapfileheader.bfSize = ((dwWidth * dwHeight * dwBPP)/8) + 
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (dwNumColors * sizeof(RGBQUAD));
	bitmapfileheader.bfReserved1 = 0;
	bitmapfileheader.bfReserved2 = 0;
	bitmapfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (dwNumColors * sizeof(RGBQUAD));  
	bitmapinfoheader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfoheader.biWidth = dwWidth;
	bitmapinfoheader.biHeight = dwHeight;
	bitmapinfoheader.biPlanes = 1;
	bitmapinfoheader.biBitCount = (WORD)dwBPP;
	bitmapinfoheader.biCompression = BI_RGB;
	bitmapinfoheader.biSizeImage = (dwHeight * dwWidth) + 1;
	bitmapinfoheader.biXPelsPerMeter = 0;
	bitmapinfoheader.biYPelsPerMeter = 0;
	bitmapinfoheader.biClrUsed = dwNumColors;
	bitmapinfoheader.biClrImportant = 0;	
	hfile = CreateFile(file,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile == INVALID_HANDLE_VALUE) 
	{
		deb("CreateFile failed : %s",FORMATERROR);
		DeleteObject(bitmap);
		DeleteDC(hdc2);
		DeleteDC(hdc);
		return FALSE;
	}
	WriteFile(hfile,&bitmapfileheader,sizeof(BITMAPFILEHEADER), &dwBytes, NULL);
	WriteFile(hfile,&bitmapinfoheader,sizeof(BITMAPINFOHEADER), &dwBytes, NULL);
	if(dwNumColors!=0)
		WriteFile(hfile,colors,sizeof(RGBQUAD)*dwNumColors,&dwBytes,NULL);
	WriteFile(hfile,pBits,bitmapinfoheader.biSizeImage,&dwBytes,NULL);
	CloseHandle(hfile);		
	DeleteObject(bitmap);
	DeleteDC(hdc2);
	DeleteDC(hdc);
	return TRUE;
}*/