#include "Space_Packet_Protocol.h"
#include "PUS.h"
#include <stdint.h>

SPP_error PUS_encode_TM_header(PUS_TM_header_t* secondary_header, uint8_t* result_buffer) {
    for(int i = 0; i < SPP_PUS_TM_HEADER_LEN_WO_SPARE; i++) {
        result_buffer[i] ^= result_buffer[i];    // Clear result buffer.
    }

    result_buffer[0] |=  secondary_header->PUS_version_number << 4;
    result_buffer[0] |=  secondary_header->sc_time_ref_status;
    result_buffer[1] |=  secondary_header->service_type_id;
    result_buffer[2] |=  secondary_header->message_subtype_id;
    result_buffer[3] |= (secondary_header->message_type_counter & 0xFF00) >> 8;
    result_buffer[4] |= (secondary_header->message_type_counter & 0x00FF);
    result_buffer[5] |= (secondary_header->destination_id & 0xFF00) >> 8;
    result_buffer[6] |= (secondary_header->destination_id & 0x00FF);
    result_buffer[7] |= (secondary_header->time & 0xFF00) >> 8;
    result_buffer[8] |= (secondary_header->time & 0x00FF);
    
    return SPP_OK;
};

PUS_TM_header_t PUS_make_TM_header(uint8_t PUS_version_number, uint8_t sc_time_ref_status, uint8_t service_type_id,
                                uint8_t message_subtype_id, uint16_t message_type_counter, uint16_t destination_id, uint16_t time) {
    PUS_TM_header_t PUS_TM_header;
    PUS_TM_header.PUS_version_number      =  PUS_version_number;
    PUS_TM_header.sc_time_ref_status      =  sc_time_ref_status;
    PUS_TM_header.service_type_id         =  service_type_id;
    PUS_TM_header.message_subtype_id      =  message_subtype_id;
    PUS_TM_header.message_type_counter    =  message_type_counter;
    PUS_TM_header.destination_id          =  destination_id;
    PUS_TM_header.time                    =  time;
    return PUS_TM_header;
}
