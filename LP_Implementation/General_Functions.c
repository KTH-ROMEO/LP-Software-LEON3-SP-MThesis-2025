#include "General_Functions.h"
#include "Space_Packet_Protocol.h"
#include "PUS.h"
#include "PUS_3.h"
#include "PUS_1_service.h"
#include "PUS_17_service.h"
#include "COBS.h"

extern int fd_UART_0;

volatile uint8_t uart_tx_OBC_done = 1;

uint16_t SPP_SEQUENCE_COUNTER = 0;

extern UART_Rx_OBC_Msg UART_RxBuffer;
uint8_t UART_TxBuffer[MAX_COBS_FRAME_LEN];

// These are used as empty headers for when calling the PUS_1_send_fail_acc,
// as the system might not have the ones from the actual message (ex: COBS fail, CRC fail, etc)
SPP_header_t Error_SPP_Header;
PUS_TC_header_t Error_PUS_TC_Header;
PUS_1_Fail_Acc_Data_t PUS_1_Fail_Acc_Data;

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

// Function that processes incoming Telecommands
void Handle_incoming_TC() {

	memset(&Error_SPP_Header, 0, sizeof(Error_SPP_Header));
	memset(&Error_PUS_TC_Header, 0, sizeof(Error_PUS_TC_Header));
	memset(&PUS_1_Fail_Acc_Data, 0, sizeof(PUS_1_Fail_Acc_Data));

    // Decode COBS frame if valid
    if(!COBS_is_valid(UART_RxBuffer.RxBuffer, UART_RxBuffer.frame_size))
    {
    	PUS_1_send_fail_acc(&Error_SPP_Header, &Error_PUS_TC_Header, &PUS_1_Fail_Acc_Data, COBS_ERROR);
		return;
    }
    uint8_t 		decoded_msg[UART_RxBuffer.frame_size];
    COBS_decode(UART_RxBuffer.RxBuffer, UART_RxBuffer.frame_size, decoded_msg);

    // Decode SPP header if possible and verify its checksum
    SPP_header_t 	SPP_header;
    uint8_t 		decoded_msg_size = UART_RxBuffer.frame_size - 2; //After COBS decoding, the first and last byte are removed.
    PUS_1_Fail_Acc_Data.TC_ReceivedBytes = decoded_msg_size;

    if(!SPP_decode_header(decoded_msg, decoded_msg_size, &SPP_header))
    {
    	PUS_1_send_fail_acc(&Error_SPP_Header, &Error_PUS_TC_Header, &PUS_1_Fail_Acc_Data, SPP_DECODE_ERROR);
    	return;
    }

    // CCSDS SPP defines the packet data length as the number of octets - 1 of the packet data field (including the CRC)
    uint16_t  		decoded_msg_expected_size = SPP_HEADER_LEN + SPP_header.packet_data_length + 1; // length = SPP + PUS + data + CRC;
    PUS_1_Fail_Acc_Data.TC_Pcklen = decoded_msg_expected_size;

    // The computed data length has to be the same as the length of the message received
    if(decoded_msg_size != decoded_msg_expected_size)
    {
    	PUS_1_send_fail_acc(&SPP_header, &Error_PUS_TC_Header, &PUS_1_Fail_Acc_Data, DATA_LENGTH_MISMATCH_ERROR);
    	return;
    }

	if (SPP_validate_checksum(decoded_msg, decoded_msg_size, &PUS_1_Fail_Acc_Data) != SPP_OK) {
		PUS_1_send_fail_acc(&SPP_header, &Error_PUS_TC_Header, &PUS_1_Fail_Acc_Data, CRC_MISMATCH_ERROR);
		return;
	}


    // Decode PUS header if present
    if (SPP_header.secondary_header_flag) {

        PUS_TC_header_t PUS_TC_header;

        if(!PUS_decode_TC_header(decoded_msg + SPP_HEADER_LEN, &PUS_TC_header, SPP_header.packet_data_length - 1))
        {
        	PUS_1_send_fail_acc(&SPP_header, &Error_PUS_TC_Header, &PUS_1_Fail_Acc_Data, PUS_DECODE_ERROR);
        	return;
        }

		uint8_t* data = decoded_msg + SPP_HEADER_LEN + PUS_TC_HEADER_LEN_WO_SPARE;
		uint8_t data_size = SPP_header.packet_data_length - PUS_TC_HEADER_LEN_WO_SPARE - 1;

		uint8_t result = NO_ERROR;

		if (PUS_TC_header.service_type_id == HOUSEKEEPING_SERVICE_ID) {
			result = PUS_3_handle_HK_TC(&SPP_header, &PUS_TC_header, data, data_size);
			if(result != NO_ERROR)
				PUS_1_send_fail_acc(&SPP_header, &PUS_TC_header, &PUS_1_Fail_Acc_Data, result);
		}
		else if (PUS_TC_header.service_type_id == FUNCTION_MANAGEMNET_ID && data_size > 0) {
			// result = PUS_8_handle_FM_TC(&SPP_header, &PUS_TC_header, data, data_size);
			// if(result != NO_ERROR)
			// 	PUS_1_send_fail_acc(&SPP_header, &PUS_TC_header, &PUS_1_Fail_Acc_Data, result);
		}
		else if (PUS_TC_header.service_type_id == TEST_SERVICE_ID) {
			result = PUS_17_handle_TEST_TC(&SPP_header, &PUS_TC_header);
			if(result != NO_ERROR)
				PUS_1_send_fail_acc(&SPP_header, &PUS_TC_header, &PUS_1_Fail_Acc_Data, result);
		} else {
			PUS_1_send_fail_acc(&SPP_header, &PUS_TC_header, &PUS_1_Fail_Acc_Data, UNSUPPORTED_SERIVCE_ID_ERROR);
		}
    }
}