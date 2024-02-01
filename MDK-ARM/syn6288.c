#include "syn6288.h"
#include "usart.h"
#include "string.h"
void SYN_FrameInfo(uint8_t music, uint8_t *data)
{
	uint8_t frame[50];
	uint8_t dataLength;
	uint8_t ecc = 0;
	dataLength = strlen((char*)data);
	
	//setting
	frame[0] = 0xfd;
	frame[1] = 0x00;
	frame[2] = dataLength + 3;
	frame[3] = 0x01;
	frame[4] = 0x01 | music << 4;
	
	//ecc calculate
	for(int i = 0;i<5;i++)
	{
		ecc = ecc ^ (frame[i]);
	}
	
	for(int i = 0;i<dataLength;i++)
	{
		ecc = ecc ^ (data[i]);
	}
	
 
	//send msg to syn 6288
	memcpy(&frame[5], data, dataLength);
	frame[5 + dataLength] = ecc;
	HAL_UART_Transmit(&huart6, frame, 5 + dataLength + 1, 1000);
}

void SYN_SET(uint8_t *setting)
{
	uint8_t cmdLen;
	cmdLen = strlen((char*)setting);
	HAL_UART_Transmit(&huart6, setting, cmdLen, 1000);
}