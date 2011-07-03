#include <stdio.h>
#include <string.h>

#include "les_core.h"
#include "les_psn.h"

int main(const int argc, const char* const argv[])
{
	printf("argc=%d\n", argc);
	for (int i=0; i<argc;i++)
	{
		printf("argv[%d]='%s'\n", i, argv[i]);
	}

	LES_Init();

	LES_sceNpInit(666);

	LES_Shutdown();
	return -1;
}

