#include "PUS_3.h"
#include "General_Functions.h"

extern int fd_UART_1, fd_UART_0;
extern rtems_id queue_id;

uint16_t vbat_i = 1;
uint16_t temperature_i = 2;
uint16_t uc3v_i = 3;
uint16_t fpga3v_i = 4;
uint16_t fpga1p5v_i = 5;
uint16_t HK_SPP_APP_ID = 0;
uint16_t HK_PUS_SOURCE_ID = 0;

int current_uC_report_frequency = 2;
int current_FPGA_report_frequency = 2;

HK_par_report_structure_t HKPRS_uc = {
    .SID                    = UC_SID,
    .collection_interval    = DEF_COL_INTV,
    .N1                     = DEF_UC_N1,
    .parameters             = {0},
    .periodic_send          = DEF_UC_PS,
    .last_collect_tick      = 0,
    .seq_count              = 0,
};

HK_par_report_structure_t HKPRS_fpga = {
    .SID                    = FPGA_SID,
    .collection_interval    = DEF_COL_INTV,
    .N1                     = DEF_FPGA_N1,
    .parameters             = {0},
    .periodic_send          = DEF_FPGA_PS,
    .last_collect_tick      = 0,
    .seq_count              = 0,
};

static void fill_report_struct(uint16_t SID) {
    uint16_t s_vbat = vbat_i;
    uint16_t s_temp = temperature_i;
    uint16_t s_fpga3v = fpga3v_i;
    uint16_t s_fpga1p5v = fpga1p5v_i;
    uint16_t s_uc3v = uc3v_i;

    uint32_t uc_pars[DEF_UC_N1] = {s_vbat, s_temp, s_uc3v};
    uint32_t fpga_pars[DEF_FPGA_N1] = {s_fpga1p5v, s_fpga3v};

    switch(SID) {
        case UC_SID:
            for(int i = 0; i < HKPRS_uc.N1; i++) {
            	HKPRS_uc.parameters[i] = ((uc_pars[i] & 0xFF000000) >> 24) |  // Move byte 3 to byte 0
                                        ((uc_pars[i] & 0x00FF0000) >> 8)  |  // Move byte 2 to byte 1
                                        ((uc_pars[i] & 0x0000FF00) << 8)  |  // Move byte 1 to byte 2
                                        ((uc_pars[i] & 0x000000FF) << 24);
            }
            break;
        case FPGA_SID:
            for(int i = 0; i < HKPRS_fpga.N1; i++) {
            	HKPRS_fpga.parameters[i] = ((fpga_pars[i] & 0xFF000000) >> 24) |  // Move byte 3 to byte 0
                                        ((fpga_pars[i] & 0x00FF0000) >> 8)  |  // Move byte 2 to byte 1
                                        ((fpga_pars[i] & 0x0000FF00) << 8)  |  // Move byte 1 to byte 2
                                        ((fpga_pars[i] & 0x000000FF) << 24);
            }
            break;
    }
}

void PUS_3_collect_HK_data(uint32_t current_ticks) {
    // if ((current_ticks - HKPRS_uc.last_collect_tick) > HKPRS_uc.collection_interval) {
        fill_report_struct(HKPRS_uc.SID);
        HKPRS_uc.last_collect_tick = current_ticks;
    // }
    // if ((current_ticks - HKPRS_fpga.last_collect_tick) > HKPRS_fpga.collection_interval) {
        fill_report_struct(HKPRS_fpga.SID);
        HKPRS_fpga.last_collect_tick = current_ticks;
    // }
}

static uint16_t encode_HK_struct(HK_par_report_structure_t* HKPRS, uint8_t* out_buffer) {
    uint8_t* iterator_pointer = out_buffer;
    memcpy(iterator_pointer, &(HKPRS->SID), sizeof(HKPRS->SID));
    iterator_pointer += sizeof(HKPRS->SID);

    for (int i = 0; i < HKPRS->N1; i++) {
        memcpy(iterator_pointer, &(HKPRS->parameters[i]), sizeof(HKPRS->parameters[i]));
        iterator_pointer += sizeof(HKPRS->parameters[i]);
    }
    return iterator_pointer - out_buffer;
}

void PUS_3_HK_send(PUS_3_msg* pus3_msg_received) {
	if (current_uC_report_frequency >= 1) {
		if (current_uC_report_frequency == 1) {
			current_uC_report_frequency = 0;
		}

		UART_OUT_OBC_msg msg_to_send_uC = {0};

		msg_to_send_uC.PUS_HEADER_PRESENT	= 1;
		msg_to_send_uC.PUS_SOURCE_ID 		= 123; //pus3_msg_received->PUS_TC_header.source_id;
		msg_to_send_uC.SERVICE_ID			= HOUSEKEEPING_SERVICE_ID;
		msg_to_send_uC.SUBTYPE_ID			= HK_PARAMETER_REPORT;
		uint16_t tm_data_len = encode_HK_struct(&HKPRS_uc, msg_to_send_uC.TM_data);
		msg_to_send_uC.TM_data_len			= tm_data_len;

		// rtems_status_code send_status = rtems_message_queue_send(queue_id, &msg_to_send_uC, sizeof(msg_to_send_uC));
        // switch (send_status) {
        //     case RTEMS_SUCCESSFUL:
        //         uart_print(fd_UART_1, "Periodic uC Data sent successfully\r\n");
        //         break;
        //     case RTEMS_TOO_MANY:
        //         uart_print(fd_UART_1, "Queue full, message not sent\r\n");
        //         break;
        //     case RTEMS_INVALID_ID:
        //         uart_print(fd_UART_1, "Invalid queue ID\r\n");
        //         break;
        //     default:
        //         uart_print(fd_UART_1, "Unknown send error\r\n");
        //         break;
        // }
	}

	if (current_FPGA_report_frequency >= 1) {
		if (current_FPGA_report_frequency == 1) {
			current_FPGA_report_frequency = 0;
		}

		UART_OUT_OBC_msg msg_to_send_FPGA = {0};

		msg_to_send_FPGA.PUS_HEADER_PRESENT	= 1;
		msg_to_send_FPGA.PUS_SOURCE_ID 		= 123; //pus3_msg_received->PUS_TC_header.source_id;
		msg_to_send_FPGA.SERVICE_ID			= HOUSEKEEPING_SERVICE_ID;
		msg_to_send_FPGA.SUBTYPE_ID			= HK_PARAMETER_REPORT;
		uint16_t tm_data_len = encode_HK_struct(&HKPRS_fpga, msg_to_send_FPGA.TM_data);
		msg_to_send_FPGA.TM_data_len			= tm_data_len;

		// rtems_status_code send_status = rtems_message_queue_send(queue_id, &msg_to_send_FPGA, sizeof(msg_to_send_FPGA));
        // switch (send_status) {
        //     case RTEMS_SUCCESSFUL:
        //         uart_print(fd_UART_1, "Periodic FPGA Data sent successfully\r\n");
        //         break;
        //     case RTEMS_TOO_MANY:
        //         uart_print(fd_UART_1, "Queue full, message not sent\r\n");
        //         break;
        //     case RTEMS_INVALID_ID:
        //         uart_print(fd_UART_1, "Invalid queue ID\r\n");
        //         break;
        //     default:
        //         uart_print(fd_UART_1, "Unknown send error\r\n");
        //         break;
        // }
	}
    // uart_print(fd_UART_0, "Sending Periodic data\n");
}
