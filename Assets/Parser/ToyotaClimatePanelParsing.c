/*
Copyright (c) 2024 Mateusz Buraczewski
Mateusz is my former student at the Warsaw University of Technology
and he gave me the permission to use this parser for educational purposes.
Thank you, Mateusz!
Bartek (https://github.com/ufnalski)
*/

uint8_t linTestData[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


typedef struct{
	bool ac;
	bool syncDual;
	bool recycle;
	bool rearDefrost;
	bool automatic;
	bool sMode;
	bool illumination;
	/* 0 - ON, 1 - OFF
	Display backlight, but it only works with illuminaition pins connected
	When illumination is disconnected then display backlight is always ON
	*/
	uint8_t tempDriver;
	/* 0-255, the same for driver and passenger
	0x00 – 'LO'
	0x01 – 16.0 then incrementing +0.5 deg up to 0x21 – 32.0
	0x22 – 65 then incrementing +1 up to  0x36 – 85
	0x37 – 'HI'
	0x38 – NOTHING ON DISPLAY
	0x39 – 'OFF'
	0x3A – 'ECO'
	0x3B-0x3F – holds last value
	0x40 – 'L 1' then incrementing +1 up to 0x5F – 'L32'
	0x60 – 14.0 then incrementing +0.5 up to 0x63 – 15.5
	0x64 – 58 then incrementing +1 up to 0x6A – 64
	0x6B – 86 then incrementing +1 up to 0x6F – 90
	0x70 up to 0x7F – holds last value
	0x80 – 00, 0x81 – 11 then incrementing +1 (excluding 20, 30, 40) up to 0xA4 – 49
	0xA5 – 99
	0xA6 – -2 then incrementing +1 up to 0xB1 – 9
	0xB2 – 88.8
	0xB3 – -3
	0xB4 up to 0xC0 – holds last value
	0xC1 – 50 then incrementing +1 (excluding 60, 70, 80) up to 0xDF – 84
	0xF0 up to 0xFF – holds last value
	*/
	uint8_t tempPassenger;
	uint8_t mode;
	/* 0x0 - 0xF
	 * 0 - nothing
	 * 1 - face
	 * 2 - face and legs
	 * 3 - legs
	 * 4 - legs and window
	 * 5,6,7,8 - nothing
	 * 9 - front defrost
	 * A - window (with human)
	 * B - window (without human)
	 * C - only human
	 * D - front defrost (with human)
	 * E, F - nothing
	*/
	uint8_t fanPower;	/* 0-7 */
	uint8_t ecoFast;
	/* 0-7
	 * 0,1,4 - nothing
	 * 2,3 - eco
	 * 6,7 - fast
	 */
}linAcReqTypeDef;

typedef struct{
	bool ac;
	bool syncDual;
	bool sMode;
	bool recycle;
	bool mode;
	bool fanUp;
	bool fanDown;
	bool ecoFast;
	bool rearDefrost;
	bool frontDefrost;
	bool automatic;
	bool off;
	int8_t tempDriver;
	/* -15 to 15
	How many steps up or down since last request
	If requests are too slow (>1s) then panel will 'forget' this value
	If requests are fast (100 ms) it is hard to get more than 3
	*/
	int8_t tempPassenger;
}linAcResTypeDef;


linAcReqTypeDef linAcReq;
linAcResTypeDef linAcRes;







// To send requests to control panel send ID 0x31 with 8 bytes of data
	  if(linAcReq.fanPower > 7) linAcReq.fanPower = 7;
	  if(linAcReq.ecoFast > 7) linAcReq.ecoFast = 7;
	  if(linAcReq.mode > 0xF) linAcReq.mode = 0xF;

	  linTestData[0] = (linAcReq.ecoFast<<4) & 0x70;
	  linTestData[1] = linAcReq.fanPower & 0x7;
	  linTestData[2] = linAcReq.mode & 0xF;
	  linTestData[4] = linAcReq.tempDriver;
	  linTestData[5] = linAcReq.tempPassenger;
	  linTestData[1] = (linTestData[1] &~(1<<5)) | (linAcReq.automatic << 5);
	  linTestData[7] = (linTestData[7] &~(1<<0)) | (linAcReq.ac << 0);
	  linTestData[3] = (linTestData[3] &~(1<<5)) | (linAcReq.syncDual << 5);
	  linTestData[3] = (linTestData[3] &~(1<<6)) | (linAcReq.rearDefrost << 6);
	  linTestData[2] = (linTestData[2] &~(1<<5)) | (linAcReq.recycle << 5);
	  linTestData[0] = (linTestData[0] &~(1<<7)) | (linAcReq.sMode << 7);
	  linTestData[7] = (linTestData[7] &~(1<<6)) | (linAcReq.illumination << 6);






// To get button state send ID 0x39 with no data and receive from control panel data.
//	in my case rxBuff contains size[0], ID[1], Data[2-9]

	  linAcRes.automatic = LIN_Instance[0].rxBuff[2] & 0x08;
	  linAcRes.off = LIN_Instance[0].rxBuff[2] & 0x02;
	  if((LIN_Instance[0].rxBuff[3] & 0x3F) == 0x00){
		  linAcRes.fanUp = false;
		  linAcRes.fanDown = false;
	  } else if((LIN_Instance[0].rxBuff[3] & 0x3F) == 0x3C){
		  /* If both pressed panel has internal priority for 'fanUp' */
		  linAcRes.fanUp = true;
		  linAcRes.fanDown = false;
	  } else if((LIN_Instance[0].rxBuff[3] & 0x3F) == 0x3D){
		  linAcRes.fanUp = false;
		  linAcRes.fanDown = true;
	  }
	  linAcRes.ecoFast = LIN_Instance[0].rxBuff[3] & 0x40;
	  linAcRes.ac = LIN_Instance[0].rxBuff[3] & 0x80;
	  linAcRes.sMode = LIN_Instance[0].rxBuff[4] & 0x80;
	  linAcRes.mode = LIN_Instance[0].rxBuff[4] & 0x1C;
	  linAcRes.rearDefrost = LIN_Instance[0].rxBuff[5] & 0x40;
	  linAcRes.frontDefrost = LIN_Instance[0].rxBuff[5] & 0x80;
	  linAcRes.syncDual = LIN_Instance[0].rxBuff[5] & 0x20;
	  linAcRes.tempPassenger = LIN_Instance[0].rxBuff[6] - 0x10;
	  linAcRes.tempPassenger = LIN_Instance[0].rxBuff[7] - 0x90;
	  linAcRes.recycle = LIN_Instance[0].rxBuff[8] & 0xC0;
