#include "context.c"

int main(int argc, char *argv[])
{
	os_innit();
	
	OS_Handle win = os_openWindow("Wenk Wenk", 0, 0, 960, 540);
	printf("window opened\n");
	
	//Arena *arena = arenaAlloc();
	Arena *frame = arenaAlloc();
	
	b32 run = 1;
	for(;run;)
	{
		ArenaTemp temp = arenaTempBegin(frame);
		
		run = !os_keyTest(temp.arena);
		
		arenaTempEnd(&temp);
	}
	printf("quit\n");
}