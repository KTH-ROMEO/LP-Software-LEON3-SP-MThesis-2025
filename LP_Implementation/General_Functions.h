#ifndef GENERAL_FUNCTIONS_H_
#define GENERAL_FUNCTIONS_H_

#include <unistd.h>
#include <string.h>

#include "Space_Packet_Protocol.h"
#include "PUS.h"

typedef enum {
	NO_ERROR								= 1,
	COBS_ERROR								= 2,
	SPP_DECODE_ERROR 						= 3,
	DATA_LENGTH_MISMATCH_ERROR 				= 4,
	CRC_MISMATCH_ERROR						= 5,
	PUS_DECODE_ERROR 						= 6,
	UNSUPPORTED_SERIVCE_ID_ERROR			= 7,
	UNSUPPORTED_SUBSERVICE_ID_ERROR			= 8,
	PUS_PROCESS_BUSY_ERROR					= 9,
	WRONG_SYSTEM_STATE_ERROR				= 10,
	NULL_POINTER_DEREFERENCING_ERROR		= 11,
	NOT_ENOUGH_DATA_ERROR					= 12,
	UNSUPPORTED_ARGUMENT_ERROR 				= 13,
	UNSUPPORTED_INDEX_ERROR 				= 14,
	FPGA_MESSAGE_ERROR						= 15,
} TM_Err_Codes;

typedef struct {
	uint8_t PUS_HEADER_PRESENT;
	uint16_t PUS_SOURCE_ID;
	uint8_t SERVICE_ID;
	uint8_t SUBTYPE_ID;
	uint8_t TM_data[SPP_MAX_PACKET_LEN];
	uint16_t TM_data_len;
} UART_OUT_OBC_msg;


void uart_print(int fd_UART, const char *msg);
void uart_print_2(int fd_UART, const char *msg, int msg_len);
void Prepare_full_msg(SPP_header_t* resp_SPP_header,
						PUS_TM_header_t* resp_PUS_header,
						uint8_t* data,
						uint16_t data_len,
						uint8_t* OUT_full_msg,
						uint16_t* OUT_full_msg_len );
void Add_SPP_PUS_and_send_TM(UART_OUT_OBC_msg* UART_OUT_msg_received);

void Handle_incoming_TC();

#endif /* GENERAL_FUNCTIONS_H_ */