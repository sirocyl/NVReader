/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <dswifi7.h>
#include <maxmod7.h>

#define IPC ((u32*)0x27b0000)

typedef void(*call3)(u32,void*,u32);
void read_nvram(u32 src, void *dst, u32 size) {
	((call3)0x2437)(src,dst,size);
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	Wifi_Update();
}


//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}


u8 writeread(u8 data) {
	while (REG_SPICNT & SPI_BUSY);
	REG_SPIDATA = data;
	while (REG_SPICNT & SPI_BUSY);
	return REG_SPIDATA;
}

void startReadFirmware() {
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	swiDelay(1000);
	writeread(0x03);
	swiDelay(1000);
	writeread(0);
	swiDelay(1000);
	writeread(0);
	swiDelay(1000);
	writeread(0);
	swiDelay(1000);
}
void readBytes(u8* buf, int count) {
	int i;
	for (i = 0; i < count; i++) {
		swiDelay(100);
		buf[i] = writeread(0);
		swiDelay(100);
	}
}

void writeSector(u32 dst, u8* src) {
	int i;
//write enable
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	writeread(6);
	REG_SPICNT = 0;
	
	//Wait for Write Enable Latch to be set
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	writeread(5);
	while((writeread(0)&0x02)==0); //Write Enable Latch
	REG_SPICNT = 0;
		
		

	 
	//page write
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	writeread(0x0A);
	writeread((dst&0xff0000)>>16);
	writeread((dst&0xff00)>>8);
	writeread(0);
	for (i=0; i<256; i++) {
		writeread(src[i]);
	}
	REG_SPICNT = 0;
	// wait programming to finish
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	writeread(0x05);
	while(writeread(0)&0x01);	//Write In Progress
	REG_SPICNT = 0;
}
void endReadFirmware() {
	REG_SPICNT = 0;
}
//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	u32 volatile* comBuff = IPC;


	
	//readUserSettings();

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	fifoInit();

	mmInstall(FIFO_MAXMOD);

	SetYtrigger(80);

	installWifiFIFO();
	installSoundFIFO();

	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);
	
	setPowerButtonCB(powerButtonCB);   
	


	while (comBuff[0] != 0x1);
	comBuff[0] = 0x2;
	while (comBuff[0] != 0x3);
	startReadFirmware();
	readBytes(&comBuff[1], 256*1024);
	endReadFirmware();
	comBuff[0] = 0x4;
	
	
	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		swiWaitForVBlank();
	}
	return 0;
}
