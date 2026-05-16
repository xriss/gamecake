#!/usr/bin/tcc -run

#include <stdio.h>


// unsure how performant this might be given the possible problems.
// just checking it does work as i understand it...

int main()
{
double d;
double du;
long long l;

	for(int i=1;i<=100;i++)
	{
		unsigned int ui=i*i*i;
		d=(double)(ui);
		du=(double)(d+0x0010000000000000ll);
		l=*((long long *)(&du));
		printf("normalized double : %llx : %llx : %f\n",l,(long long)d,d);
	}

	printf("Note we must force IEEE 754 for this to work probably...\n");
	printf("See the horror that is https://github.com/taschini/crlibm/blob/eb3063791aa75bc9705b49283bf14250465220a7/crlibm_private.c#L59\n");
	printf("Adding 0x0010000000000000ll to a double will normalize its bits, Allowing us to treat the bottom 52 as a raw int.\n");
	printf("bit banging 0x433 into the top 12bits of a 64bit int will turn an int into a double.\n");
	printf("Then we must subtract 0x0010000000000000ll (as a double) to correct number.\n");

}
