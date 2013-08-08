#include <nds.h>

#include <nds/arm9/input.h>
#include <nds/arm9/console.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define IPC ((u32*)0x27b0000)



void error(char* msg) {
	iprintf("%s\n", msg);
	while(1) {
		swiWaitForVBlank();
	}
}


//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	u32 volatile * comBuff = IPC;
	// Initialise the console, required for printf
	consoleDemoInit();
	

	comBuff[0] = 0x1;
	iprintf("waiting...\n");
	while(comBuff[0] != 0x2);
	iprintf("reading...\n");
	comBuff[0] = 0x3;
	while(comBuff[0] != 0x4);
	iprintf("read ok, saving to fwread.bin ...\n");
	
	/*
	startReadFirmware();
	readBytes(buff, sizeof(buff));
	endReadFirmware();*/
	if (!fatInitDefault()) {
		error("fat init failed");
	}
	FILE* f = fopen("/fwread.bin", "wb");
	if (!f) {
		error("fopen failed");
	}

	fwrite(&comBuff[1], 256*1024, 1, f);
	
	fclose(f);
	
	iprintf("done!\n");
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if (keysDown()&KEY_START) break;
	}
	iprintf("exiting...\n");
	swiSoftReset();
	return 0;
}
