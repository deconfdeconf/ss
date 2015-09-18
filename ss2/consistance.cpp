#include "Stdafx.h"

void heap_check(void)
{	
	deb("chk: validating heap...");
	if(!HeapValidate(GetProcessHeap(), 0, NULL))
		udpdeb("chk: heap corrupt!");
	else
		udpdeb("chk: heap OK");
}

void run_consistance_check(void)
{
	deb("running consistance check...");
	heap_check();
}