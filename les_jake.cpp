#include "les_jake.h"
#include "les_function_macros.h"

void LES_jakeInit(int a, short b)
{
	float r = 456.0f;
	float* rPtr = &r;
	LES_FUNCTION_START(jakeInit, void);
	LES_FUNCTION_INPUTS_2(int, a, short, b);
	LES_FUNCTION_OUTPUTS_1(float*, rPtr);
	LES_FUNCTION_END();
}
