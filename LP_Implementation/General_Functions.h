#include <unistd.h>
#include <string.h>

#include "Space_Packet_Protocol.h"

typedef struct {
	uint8_t PUS_HEADER_PRESENT;
	uint16_t PUS_SOURCE_ID;
	uint8_t SERVICE_ID;
	uint8_t SUBTYPE_ID;
	uint8_t TM_data[SPP_MAX_PACKET_LEN];
	uint16_t TM_data_len;
} UART_OUT_OBC_msg;


void uart_print(int fd_UART, const char *msg);
