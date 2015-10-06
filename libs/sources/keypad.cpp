#include "../gba.h"
#include "../keypad.h"

bool CheckPressed(u16 key)
{
	return !(REG_KEY & key);
}