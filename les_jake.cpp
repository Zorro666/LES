#include <stdio.h>
#include <string.h>

#include "les_jake.h"
#include "les_core.h"
#include "les_stringentry.h"

void LES_jakeInit(int a, short b)
{
	float r;
	LES_FUNCTION_START(jakeInit, void);
	LES_FUNCTION_INPUTS_2(int, a, short, b);
	LES_FUNCTION_OUTPUTS_1(float, r);
}
