#include <stdio.h>
#include <string.h>

#include "les_psn.h"

#include "les_core.h"
#include "les_stringentry.h"

void LES_sceNpInit(int a)
{
	int r;
	LES_FUNCTION_START(sceNpInit, void)
	LES_FUNCTION_INPUTS_1(int, a)
	LES_FUNCTION_OUTPUTS_1(short, r)
	fprintf(stderr, "END\n");
}
