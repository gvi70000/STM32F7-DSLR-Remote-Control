#include "AD5235.h"
#include "spi.h"
#include "gpio.h"
uint16_t ret = 0;
 
static void AD5235_write(uint8_t cmd, uint16_t val){
	uint8_t in[3] = {0, 0, 0};
	in[0] = cmd;
	in[1] = val >> 8;
	in[2] = val & 0xFF;
	setSpiSelect(OFF);
	HAL_SPI_Transmit(&hspi1, in, SEND_LIMIT, TIMEOUT_AD);
	setSpiSelect(ON);
}

static uint16_t AD5235_read(void){
	uint8_t in[3] = {0, 0, 0};
	uint8_t out[3] = {0, 0, 0};
	setSpiSelect(OFF);
	HAL_SPI_TransmitReceive(&hspi1, in, out, GET_LIMIT, TIMEOUT_AD);
	setSpiSelect(ON);
	ret = (uint16_t)(out[1] << 8) + out[2];
	return ret;
}

//uint16_t transferData(uint8_t cmd, uint16_t val)
//{
//	uint8_t bytes[SEND_LIMIT], i;
//	CS_PressOff;
//	bytes[0] = cmd;
//	bytes[1] = val >> 8;
//	bytes[2] = val & 0xFF;
//	__HAL_LOCK(&hspi2);
//  for(i=0; i<SEND_LIMIT; i++)
//  {
//  	// Wait until the transmit buffer is empty
//		while(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_TXE) == RESET){
//		}
//    // Send the byte
//		hspi2.Instance->DR = bytes[i];
//    // Wait to receive a byte
//		while(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_RXNE) != RESET){
//		}
//    // Return the byte read from the SPI bus
//    bytes[i] = hspi2.Instance->DR;
//  }	
//	__HAL_UNLOCK(&hspi2);
//	CS_PressOn;
//	return (uint16_t)(bytes[1] << 8) + bytes[2];
//}

//1 - Restore EEMEM (A0) contents to RDAC (A0) register. See Table 16.
void storeEEMEM2RDAC(uint8_t w) {
	AD5235_write(EEMEM2RDAC + w,  CMD_NOP);
} 

//2 - Store wiper setting. Store RDAC (A0) setting to EEMEM (A0). See Table 15. - Use a delay of 50ms!!!
void storeRDAC2EEMEM(uint8_t w) {
	AD5235_write(RDAC2EEMEM + w,  CMD_NOP);
} 

//3 - Store contents of Serial Register Data Byte 0 and Serial Register Data Bytes 1 (total 16 bits) to EEMEM (ADDR).
//See Table 18.- Use a delay of 50ms!!!/RDAC1 is 0, RDAC2 is 1, User1 is 2....User13 is 14
void setEEMEM(uint8_t w, uint16_t v) {
	AD5235_write(EEMEM_store + w, v);
} 

//4 - Decrement by 6 dB. Right-shift contents of RDAC (A0) register, stop at all 0s.
void stepDown6Db(uint8_t w) {
	AD5235_write(Down6Db + w,  CMD_NOP);
} 

//5 - Decrement all by 6 dB. Right-shift contents of all RDAC registers, stop at all 0s.
void stepDownAll6Db(void) {
	AD5235_write(DownAll6Db , CMD_NOP);
} 

//6 - Decrement contents of RDAC (A0) by 1, stop at all 0s.
void stepDown1(uint8_t w) {
	AD5235_write(Down1 + w, CMD_NOP);
} 

//7 - Decrement contents of all RDAC registers by 1, stop at all 0s.
void stepDown1All(void) {
	AD5235_write(Down1All, CMD_NOP);
} 

//8 - Reset. Refresh all RDACs with their corresponding EEMEM previously stored values. - Use a delay of 30us!!!
void refreshAllRDAC(void) {
	AD5235_write(AllRDAC, CMD_NOP);
} 

//9 - Read contents of EEMEM (ADDR) from SDO output in the next frame. See Table 19. - Use a delay of 30us!!!
uint16_t getEEMEM(uint8_t w) {
	AD5235_write(EEMEM_read + w, CMD_NOP);
	//delay_us(30);
	HAL_Delay(1);
	return AD5235_read();
}

//10 - Read RDAC wiper setting from SDO output in the next frame. See Table 20. - Use a delay of 30us!!!
uint16_t getWiper(uint8_t w) {
	AD5235_write(gWiper + w, CMD_NOP);
	//delay_us(30);
	HAL_Delay(1);
	return AD5235_read();
	//transferData(0xA0 + w, CMD_NOP);
	//HAL_Delay(1);
	//return transferData(CMD_NOP, CMD_NOP);
} 

//11 - Write contents of Serial Register Data Byte 0 and Serial Register Data Byte 1 (total 10 bits) to RDAC (A0). See Table 14.
void setWiper(uint8_t w, uint16_t v) {
	// Inverse the wiper value
	AD5235_write(sWiper + w, v);
	//transferData(0xB0 + w, v);
} 

//12 - Increment by 6 dB: Left-shift contents of RDAC (A0),stop at all 1s. See Table 17.
void stepUp6Db(uint8_t w) {
	AD5235_write(Up6Db, CMD_NOP);
} 

//13 - Increment all by 6 dB. Left-shift contents of all RDAC registers, stop at all 1s.
void stepUpAll6Db(void) {
	AD5235_write(UpAll6Db, CMD_NOP);
} 

//14 - Increment contents of RDAC (A0) by 1, stop at all 1s. See Table 15.
void stepUp1(uint8_t w) {
	AD5235_write(Up1 + w, CMD_NOP);
}

//15 - Increment contents of all RDAC registers by 1, stop at all 1s.
void stepUp1All(void) {
	AD5235_write(Up1All, CMD_NOP);
}

//16 - See page 21 in manual Another subtle feature of the AD5235 is that a subsequent CS strobe, without clock and data, repeats a previous command
void repeatCMD(void) {
	setSpiSelect(OFF);
	setSpiSelect(ON);
}

float getTolerance(void)
{
	float tol = getEEMEM(15);
	int8_t b_1 = ((uint16_t)tol >> 8);
	int8_t b_0 = ((uint16_t)tol & 0xFF);
	if(b_1 > 127) //check if first bit in b_1 is 1
	b_1 -= 128;
	else
	b_1 = -b_1;
	for(uint8_t p = 0; p < 8; p++)
	{
		if((b_0 >> p) & 1)//check if but in position p is 1
		{
			tol =(float)b_1 + (float)b_0/ (float)(2 << p);//add the decimal part of the tolerance
			break;
		}
	}
	return tol;
}
