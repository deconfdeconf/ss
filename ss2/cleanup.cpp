#include "stdafx.h"

char	*shit_files[] = { "c:\\ms.hta", "c:\\poc.exe", "c:\\loader.exe", NULL };

void cleanup_loader_shit(void)
{
	int		k;

	for(k = 0; shit_files[k]; k++)
		DeleteFile(shit_files[k]);
}