#include <unistd.h>
#include <string.h>

#include "Space_Packet_Protocol.h"
#include "PUS.h"

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

