#include <stdio.h>
#include <string.h>

#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"

int main(const int argc, const char* const argv[])
{
	printf("argc=%d\n", argc);
	for (int i=0; i<argc;i++)
	{
		printf("argv[%d]='%s'\n", i, argv[i]);
	}

	return -1;
}

void LES_sceNpInit(int a)
{
	int r;
	LES_FUNCTION_START(sceNpInit, void)
	LES_FUNCTION_INPUTS_1(int, a)
	LES_FUNCTION_OUTPUTS_1(int, r)
}
