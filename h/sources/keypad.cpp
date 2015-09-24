#include "../gba.h"
#include "../keypad.h"

INLINE bool CheckPressed(u16 key)
{
	return !(REG_KEY & key);
}