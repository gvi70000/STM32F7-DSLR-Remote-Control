#ifndef __AD5235_H
#define __AD5235_H

#ifdef __cplusplus
extern "C" {
#endif
	#include "spi.h"
	//#include "myDelay.h"	
	#define SEND_LIMIT		3
	#define GET_LIMIT			(2 * SEND_LIMIT)
	#define TIMEOUT_AD		100
	
	#define CMD_NOP	0x00 //0- Do nothing
	//Control and Error Registers
	#define EEMEM2RDAC	0x10 //1 - Restore EEMEM (A0) contents to RDAC (A0) register. See Table 16.
	#define RDAC2EEMEM	0x20 //2 - Store wiper setting. Store RDAC (A0) setting to EEMEM (A0). See Table 15. - Use a delay of 50ms!!!
	#define EEMEM_store	0x30 //3 - Store contents of Serial Register Data Byte 0 and Serial Register Data Bytes 1 (total 16 bits) to EEMEM (ADDR). See Table 18.- Use a delay of 50ms!!!/RDAC1 is 0, RDAC2 is 1, User1 is 2....User13 is 14
	#define Down6Db			0x40 //4 - Decrement by 6 dB. Right-shift contents of RDAC (A0) register, stop at all 0s.
	#define DownAll6Db	0x50 //5 - Decrement all by 6 dB. Right-shift contents of all RDAC registers, stop at all 0s.
	#define Down1				0x60 //6 - Decrement contents of RDAC (A0) by 1, stop at all 0s.
	#define Down1All		0x70 //7 - Decrement contents of all RDAC registers by 1, stop at all 0s.
	#define AllRDAC 		0x80 //8 - Reset. Refresh all RDACs with their corresponding EEMEM previously stored values. - Use a delay of 30us!!!
	#define EEMEM_read	0x90 //9 - Read contents of EEMEM (ADDR) from SDO output in the next frame. See Table 19. - Use a delay of 30us!!!
	#define gWiper			0xA0 //10 - Read RDAC wiper setting from SDO output in the next frame. See Table 20. - Use a delay of 30us!!!
	#define sWiper			0xB0 //11 - Write contents of Serial Register Data Byte 0 and Serial Register Data Byte 1 (total 10 bits) to RDAC (A0). See Table 14.
	#define Up6Db				0xC0 //12 - Increment by 6 dB: Left-shift contents of RDAC (A0),stop at all 1s. See Table 17.
	#define UpAll6Db		0xD0 //13 - Increment all by 6 dB. Left-shift contents of all RDAC registers, stop at all 1s.
	#define Up1					0xE0 //14 - Increment contents of RDAC (A0) by 1, stop at all 1s. See Table 15.
	#define Up1All			0xF0 //15 - Increment contents of all RDAC registers by 1, stop at all 1s.
	
	void storeEEMEM2RDAC(uint8_t w);
	void storeRDAC2EEMEM(uint8_t w);
	void setEEMEM(uint8_t w, uint16_t v);
	void stepDown6Db(uint8_t w);
	void stepDownAll6Db(void);
	void stepDown1(uint8_t w);
	void stepDown1All(void);
	void refreshAllRDAC(void);
	uint16_t getEEMEM(uint8_t w);
	uint16_t getWiper(uint8_t w);
	void setWiper(uint8_t w, uint16_t v);
	void stepUp6Db(uint8_t w);
	void stepUpAll6Db(void);
	void stepUp1(uint8_t w);
	void stepUp1All(void);
	float getTolerance(void);
	void repeatCMD(void);
	
#ifdef __cplusplus
}
#endif

#endif /* __AD5235_H */
