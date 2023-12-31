﻿/*
 * DFplayer.h
 *
 * Created: 2023-09-09 오전 11:57:43
 *  Author: CTM
 */ 


#ifndef DFPLAYER_H_
#define DFPLAYER_H_

#define BAUD 9600
#define U2X_S 2     // Set of U2X --> 1 or 2
#define MYUBRR ((F_CPU*U2X_S)/(16L*BAUD)-1)
#define sbi(reg,bit)    reg |= (1<<(bit))      // Set "bit"th bit of Register "reg"
#define cbi(reg,bit)    reg &= ~(1<<(bit))
//
#define MP3_NEXT                    0x01
#define MP3_PREVIOUS                0x02
#define MP3_TRAKING_NUM                0x03 // 0..2999
#define MP3_INC_VOLUME                0x04
#define MP3_DEC_VOLUME                0x05
#define MP3_VOLUME                    0x06 // 0..30
#define MP3_EQ                        0x07 // 0-Normal / 1-Pop / 2-Rock / 3-Jazz / 4-Classic / 5-Base
#define MP3_PLAYBACK_MODE            0x08 // 0-Repeat / 1-folder repeat / 2-single repeat / 3-random
#define MP3_PLAYBACK_SOURCE            0x09 // 0-U / 1-TF / 2-AUX / 3-SLEEP / 4-FLASH
#define MP3_STANDBY                    0x0A
#define MP3_NORMAL_WORK                0x0B
#define MP3_RESET                    0x0C
#define MP3_PLAYBACK                0x0D
#define MP3_PAUSE                    0x0E
#define MP3_PLAY_FOLDER_FILE        0x0F // 0..10
#define MP3_VOLUME_ADJUST            0x10
#define MP3_REPEAT                    0x11 // 0-stop play / 1-start repeat play
// Query the System Parameters
#define MP3_Q_STAY1                    0x3C
#define MP3_Q_STAY2                    0x3D
#define MP3_Q_STAY3                    0x3E
#define MP3_Q_SEND_PRM                0x3F
#define MP3_Q_ERROR                    0x40
#define MP3_Q_REPLY                    0x41
#define MP3_Q_STATUS                0x42
#define MP3_Q_VALUE                    0x43
#define MP3_Q_EQ                    0x44
#define MP3_Q_PLAYBACK_MODE            0x45
#define MP3_Q_SOFT_VERSION            0x46
#define MP3_Q_TF_CARD_FILES            0x47
#define MP3_Q_U_DISK_CARD_FILES        0x48
#define MP3_Q_FLASH_CARD_FILES        0x49
#define MP3_Q_KEEPON                0x4A
#define MP3_Q_CURRENT_TRACK_TF        0x4B
#define MP3_Q_CURRENT_TRACK_U_DISK    0x4C
#define MP3_Q_CURRENT_TRACK_FLASH    0x4D
////////////////////////////////////////////////////////////////////////////////
//Commands parameters
////////////////////////////////////////////////////////////////////////////////
#define MP3_EQ_Normal                    0
#define MP3_EQ_Pop                        1
#define MP3_EQ_Rock                        2
#define MP3_EQ_Jazz                        3
#define MP3_EQ_Classic                    4
#define MP3_EQ_Base                        5
#define MP3_PLAYBACK_MODE_Repeat        0
#define MP3_PLAYBACK_MODE_folder_repeat    1
#define MP3_PLAYBACK_MODE_single_repeat    2
#define MP3_PLAYBACK_MODE_random        3
#define MP3_PLAYBACK_SOURCE_U            0
#define MP3_PLAYBACK_SOURCE_TF            1
#define MP3_PLAYBACK_SOURCE_AUX            2
#define MP3_PLAYBACK_SOURCE_SLEEP        3
#define MP3_PLAYBACK_SOURCE_FLASH        4

typedef unsigned char INT8;
typedef unsigned int INT16;
INT8 default_buffer[10] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF}; // Default Buffer
volatile INT8 mp3_cmd_buf[10] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};
	
INT16 MP3_checksum (void)
{
	INT16 sum = 0;
	INT8 i;
	for (i=1; i<7; i++) {
		sum += mp3_cmd_buf[i];
	}
	return -sum;
}

/*DFplayer에 정보 보내는 함수*/
void MP3_send_cmd (INT8 cmd, INT16 high_arg, INT16 low_arg)
{
	INT8 i;
	INT16 checksum;
	mp3_cmd_buf[3] = cmd;
	mp3_cmd_buf[5] = high_arg;
	mp3_cmd_buf[6] = low_arg;
	checksum = MP3_checksum();
	mp3_cmd_buf[7] = (INT8) ((checksum >> 8) & 0x00FF);
	mp3_cmd_buf[8] = (INT16) (checksum & 0x00FF);
	for( i=0; i< 10; i++){
		USART0_Transmit(mp3_cmd_buf[i]);
		//putchar(mp3_cmd_buf[i]);
		mp3_cmd_buf[i] = default_buffer[i];
	}
}

/*DFplayer 모듈 초기화 함수*/
void dfplayer_init(void)
{
	MP3_send_cmd(MP3_PLAYBACK_SOURCE,0,MP3_PLAYBACK_SOURCE_TF); 
	_delay_ms(10);
	MP3_send_cmd(MP3_VOLUME, 0, 15); 
	_delay_ms(10);
}

/*UART 값 반환 함수*/
uint8_t USART_Receive() {
	while (!(UCSRA & (1 << RXC)));
	return UDR;
}

void USART0_Init( INT16 ubrr )
{
	// Set baud rate
	UBRRH = (INT8)(ubrr>>8);
	UBRRL = (INT8)ubrr;
	// Enable U2X
	if(U2X_S == 2)
	sbi(UCSRA, U2X);
	else
	cbi(UCSRA, U2X);
	// Enable receiver and transmitter
	sbi(UCSRB, RXEN);
	sbi(UCSRB, TXEN);
	// Set frame format: 8data, 1stop bit
	cbi(UCSRC, UMSEL);  // asynch
	cbi(UCSRC, USBS);   // 1 Stop bit
	cbi(UCSRC, UPM1);  // No parity
	cbi(UCSRC, UPM0);
	cbi(UCSRB, UCSZ2); // 8-bit
	sbi(UCSRC, UCSZ1);
	sbi(UCSRC, UCSZ0);
}
void USART0_Transmit( char data )
{
	// Wait for empty transmit buffer
	while ( !( UCSRA & 0x20 ) )   // (1<<UDRE)
	;
	// Put data into buffer, sends the data
	UDR = data;
}



uint8_t USART0_available() {
	return (UCSRA & (1 << RXC)); // RXC 비트 체크
}


#endif /* DFPLAYER_H_ */