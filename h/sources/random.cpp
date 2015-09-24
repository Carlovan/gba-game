#include "../gba.h"
#include "../random.h"

//Seed
int __qran_seed = 42;

// Funzione di seeding
int sqran(int seed){
	int old = __qran_seed;
	__qran_seed = seed;
	return old;
}

INLINE int qran(){
	__qran_seed = 1664525 * __qran_seed + 1013904223;
	return (__qran_seed>>16) & 0x7FFF;
}

INLINE int qran_range(int min, int max){
	return abs((qran() * (min-max)>>15)+min);
}