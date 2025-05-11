#include "General_Functions.h"
#include "Space_Packet_Protocol.h"
#include "PUS.h"
#include "COBS.h"

extern int fd_UART_0;

volatile uint8_t uart_tx_OBC_done = 1;

uint16_t SPP_SEQUENCE_COUNTER = 0;

uint8_t UART_TxBuffer[MAX_COBS_FRAME_LEN];

void uart_print(int fd_UART, const char *msg)
{
    write(fd_UART, msg, strlen(msg));
}

void uart_print_2(int fd_UART, const char *msg, int msg_len)
{
    write(fd_UART, msg, msg_len);
}

void Prepare_full_msg(SPP_header_t* resp_SPP_header,
						PUS_TM_header_t* resp_PUS_header,
						uint8_t* data,
						uint16_t data_len,
						uint8_t* OUT_full_msg,
						uint16_t* OUT_full_msg_len ) {

    uint8_t* current_pointer = OUT_full_msg;

    SPP_encode_header(resp_SPP_header, current_pointer);
    current_pointer += SPP_HEADER_LEN;

    if (resp_PUS_header != NULL) {
        PUS_encode_TM_header(resp_PUS_header, current_pointer);
        current_pointer += SPP_PUS_TM_HEADER_LEN_WO_SPARE;
    }

    if (data != NULL) {
        SPP_add_data_to_packet(data, data_len, current_pointer);
        current_pointer += data_len;
    }

    SPP_add_CRC_to_msg(OUT_full_msg, current_pointer - OUT_full_msg, current_pointer);
    current_pointer += CRC_BYTE_LEN;
    *OUT_full_msg_len = current_pointer - OUT_full_msg;
}

void Send_TM(SPP_header_t* resp_SPP_header,
				PUS_TM_header_t* resp_PUS_header,
				uint8_t* data,
				uint16_t data_len) {

    uint8_t response_TM_packet[SPP_MAX_PACKET_LEN] = {0};
    uint8_t response_TM_packet_COBS[SPP_MAX_PACKET_LEN] = {0};
    uint16_t packet_total_len = 0;

    Prepare_full_msg(resp_SPP_header,
						resp_PUS_header,
						data, data_len,
						response_TM_packet,
						&packet_total_len);

    uint16_t cobs_packet_total_len = COBS_encode(response_TM_packet,
												packet_total_len,
												response_TM_packet_COBS);

    // Mark the DMA pipeline busy
    uart_tx_OBC_done = 0;

    memcpy(UART_TxBuffer, response_TM_packet_COBS, cobs_packet_total_len);
    UART_TxBuffer[cobs_packet_total_len] = 0x00; // Adding sentinel value.
    cobs_packet_total_len +=1;


    uart_print_2(fd_UART_0, UART_TxBuffer, cobs_packet_total_len);

}

void Add_SPP_PUS_and_send_TM(UART_OUT_OBC_msg* UART_OUT_msg_received) {

		if(SPP_SEQUENCE_COUNTER >= 65535)
			SPP_SEQUENCE_COUNTER = 0;
		else
			SPP_SEQUENCE_COUNTER++;

        if(UART_OUT_msg_received->PUS_HEADER_PRESENT == 1){
        	SPP_header_t TM_SPP_header = SPP_make_header(
        				SPP_VERSION,
        				SPP_PACKET_TYPE_TM,
        				UART_OUT_msg_received->PUS_HEADER_PRESENT,
        				SPP_APP_ID,
        				SPP_SEQUENCE_SEG_UNSEG,
        				SPP_SEQUENCE_COUNTER,
        				SPP_PUS_TM_HEADER_LEN_WO_SPARE + UART_OUT_msg_received->TM_data_len + CRC_BYTE_LEN - 1
        	        );

			PUS_TM_header_t TM_PUS_header  = PUS_make_TM_header(
				PUS_VERSION,
				0,
				UART_OUT_msg_received->SERVICE_ID,
				UART_OUT_msg_received->SUBTYPE_ID,
				0,
				UART_OUT_msg_received->PUS_SOURCE_ID,
				0
			);
			Send_TM(&TM_SPP_header, &TM_PUS_header, UART_OUT_msg_received->TM_data, UART_OUT_msg_received->TM_data_len);
        }
        else{
        	SPP_header_t TM_SPP_header = SPP_make_header(
				SPP_VERSION,
				SPP_PACKET_TYPE_TM,
				UART_OUT_msg_received->PUS_HEADER_PRESENT,
				SPP_APP_ID,
				SPP_SEQUENCE_SEG_UNSEG,
				SPP_SEQUENCE_COUNTER,
				UART_OUT_msg_received->TM_data_len + CRC_BYTE_LEN - 1
			);

        	Send_TM(&TM_SPP_header, NULL, UART_OUT_msg_received->TM_data, UART_OUT_msg_received->TM_data_len);
        }
}