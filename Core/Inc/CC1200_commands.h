/*
 * CC1200_commands.h
 *
 *  Created on: 7 апр. 2026 г.
 *      Author: ASharonov
 */

#ifndef CC1200_COMMANDS_H_
#define CC1200_COMMANDS_H_



#define CC1200_EXT_ADDR 0x2F // SPI initial byte address indicating extended register space
#define CC1200_WRITE 0b00000000 // SPI initial byte flag indicating write //нужно записать 0 в бит 8 в первый байт адреса
#define CC1200_READ 0b10000000 // SPI initial byte flag indicating write //нужно записать 1 в бит 8 в первый байт адреса
#define CC1200_BURST 0b01000000 // SPI initial byte flag indicating burst //нужно записать 1 в бит 7 в первый байт адреса

// Команды для управления радио
#define CC1200_SIDLE        0x36  // Выйти в режим IDLE
#define CC1200_STX 			0x35  // Перейти в режим передачи
#define CC1200_SRX 			0x34  // Перейти в режим периёма
#define CC1200_SFRX			0x3A  // Flush the RX FIFO. Only issue SFRX in IDLE or RX_FIFO_ERR states
#define CC1200_SFTX			0x3B  // Flush the TX FIFO. Only issue SFTX in IDLE or TX_FIFO_ERR states
#define CC1200_SFSTXON		0x31  // Enable and calibrate frequency synthesizer

#define  CC1200_SNOP 		0x3D  //No operation. May be used to get access to the chip status byte



#endif /* CC1200_COMMANDS_H_ */
