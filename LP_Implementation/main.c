// #include <rtems.h>
#include <bsp.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "main.h"

#include <rtems/confdefs.h>

#include "PUS_3.h"
#include "Device_State.h"
#include "General_Functions.h"

int fd_UART_0, fd_UART_1;

struct termios options_UART_0, options_UART_1;

rtems_id PUS_3, handle_UART_OUT_OBC, handle_UART_Rx_Callback, handle_UART_IN_OBC;
rtems_id queue_1_id, queue_2_id;
rtems_event_set events;

uint8_t UART_OBC_OPEN = 1; // Flag used to process incoming UART data

DeviceState Current_Global_Device_State = NORMAL_MODE;

extern volatile UART_Rx_OBC_Msg UART_RxBuffer;
extern volatile uint16_t UART_recv_count;
extern volatile uint8_t UART_recv_char;
extern uint8_t current_uC_report_frequency;
extern uint8_t current_FPGA_report_frequency;

rtems_task PUS_3_Task(rtems_task_argument argument)
{
    uart_print(fd_UART_1, "STARTED PUS 3 TASK\r\n");

    rtems_interval    current_ticks = 0;
    volatile uint8_t           periodic_report = 0;
    PUS_3_msg         pus3_msg_received;
    size_t            received_size;
    rtems_status_code status;
    uint8_t           result = NO_ERROR;
    rtems_interval    two_second_ticks = rtems_clock_get_ticks_per_second()/4;

    for (;;) {
        if (!periodic_report) {
            /* ——— Equivalent to xQueueReceive(..., portMAX_DELAY) —— */
            status = rtems_message_queue_receive(
            queue_2_id,
            &pus3_msg_received,
            &received_size,
            RTEMS_WAIT,    /* block */
            RTEMS_NO_TIMEOUT          /* infinite wait */
            );
            if (status == RTEMS_SUCCESSFUL) {
            /* process command */
            result = PUS_3_set_report_frequency(
                        pus3_msg_received.data,
                        &pus3_msg_received
                        );

            if (result == NO_ERROR) {
                current_ticks = rtems_clock_get_ticks_since_boot();
                PUS_3_collect_HK_data(current_ticks);
                PUS_3_HK_send(&pus3_msg_received);
                PUS_1_send_succ_comp(
                &pus3_msg_received.SPP_header,
                &pus3_msg_received.PUS_TC_header
                );
                if ( current_uC_report_frequency == 2 || current_FPGA_report_frequency == 2) {
                    // uart_print(fd_UART_1, "ENTERED HERE\r\n");
                    periodic_report = 1;
                }
            }
            else {
                PUS_1_send_fail_comp(
                &pus3_msg_received.SPP_header,
                &pus3_msg_received.PUS_TC_header,
                result
                );
            }
            }
        }
        else {
            /* ——— Emulate xQueuePeek(queue, buf, 2000) ——— */
            status = rtems_message_queue_receive(
                queue_2_id,
                &pus3_msg_received,
                &received_size,
                RTEMS_WAIT, /* block if timeout > 0 */
                two_second_ticks       /* 2 seconds */
            );
            if (status == RTEMS_SUCCESSFUL) {
            /* “Peek” succeeded ⇒ clear flag and re-queue the message */
            periodic_report = 0;
            rtems_message_queue_send(
                queue_2_id,
                &pus3_msg_received,
                received_size
            );
            }
            else {
            /* no new command ⇒ send periodic HK */
            current_ticks = rtems_clock_get_ticks_since_boot();
            PUS_3_collect_HK_data(current_ticks);
            PUS_3_HK_send(&pus3_msg_received);
            }
        }

        /* delay 1 tick (like osDelay(1)) */
        rtems_task_wake_after(1);
    }
}


rtems_task handle_UART_OUT_OBC_Task(rtems_task_argument argument)
{
	UART_OUT_OBC_msg UART_OUT_msg_received;
	size_t received_size;
	rtems_status_code status;

    uart_print(fd_UART_1, "STARTED UART SENDING TASK \n\r");

	while (1)
	{
		// Blocking receive (wait forever)
		status = rtems_message_queue_receive(
			queue_1_id,
			&UART_OUT_msg_received,
			&received_size,
			RTEMS_WAIT,
			RTEMS_NO_TIMEOUT
		);

		if (status == RTEMS_SUCCESSFUL )
		{
            uart_print(fd_UART_1, "Message received \n\r");

            Add_SPP_PUS_and_send_TM(&UART_OUT_msg_received);
		}

		// Delay 1 tick (similar to osDelay(1))
		rtems_task_wake_after(1);
	}
}

rtems_task handle_UART_Rx_Callback_Task(rtems_task_argument argument)
{
	UART_OUT_OBC_msg UART_OUT_msg_received;
	size_t received_size;
	rtems_status_code status;
    char UART_recv_char = {0xFF};
    uint8_t len; 

    uart_print(fd_UART_1, "STARTED UART READING TASK\r\n");

	while (1)
	{
        len = read(fd_UART_0, &UART_recv_char, 1);

        if(UART_OBC_OPEN == 0)
            continue;
        
        UART_RxBuffer.RxBuffer[UART_recv_count] = UART_recv_char;

        // Check if this is the end of frame (0x00 terminator) or the buffer is full
		if (UART_recv_char == 0x00 || UART_recv_count >= MAX_COBS_FRAME_LEN - 1)
		{
            uart_print(fd_UART_1, "Received a new COBS message\r\n");

			// Mark the frame size
			UART_RxBuffer.frame_size = UART_recv_count + 1;

			UART_recv_count = 0;
			UART_recv_char = 0xff;

			// Signal the task that a complete frame is ready to be processed
            status = rtems_event_send(handle_UART_IN_OBC, RTEMS_EVENT_1);

            UART_OBC_OPEN = 0;
			// DO NOT RE-ARM THE ISR, it will be done after the task processes the buffer,
			// thus ensuring no race condition (the buffer is not modified while being used)
		}
		else
		{
			// Continue accumulating characters
			UART_recv_count++;
		}
	}
}

rtems_task handle_UART_IN_OBC_Task(rtems_task_argument argument)
{
	UART_OUT_OBC_msg UART_OUT_msg_received;
	size_t received_size;
	rtems_status_code status;

    // uart_print(fd_UART_0, "Receiving task started \n\r");
    uart_print(fd_UART_1, "STARTED UART PROCESSING TASK\r\n");
    char UART_recv_char = {0xFF};

    uint8_t len; 

	while (1)
	{
        rtems_event_receive(
            RTEMS_EVENT_1,        // Wait for event bit 1
            RTEMS_WAIT | RTEMS_EVENT_ANY,
            RTEMS_NO_TIMEOUT,
            &events
        );

        uart_print(fd_UART_1, "RECEIVED MESSAGE\r\n");

        Handle_incoming_TC();

        memset((void*)UART_RxBuffer.RxBuffer, 0, sizeof(UART_RxBuffer.RxBuffer));

        UART_OBC_OPEN = 1;
	}
}

/* The Init task creates and starts the two UART tasks */
rtems_task Init(rtems_task_argument argument)
{
    fd_UART_0 = open("/dev/console", O_RDWR);
    tcgetattr(fd_UART_0, &options_UART_0);
    cfmakeraw(&options_UART_0); 
    cfsetispeed(&options_UART_0, B38400);
    tcsetattr(fd_UART_0, TCSANOW, &options_UART_0);

    fd_UART_1 = open("/dev/console_b", O_RDWR);
    tcgetattr(fd_UART_1, &options_UART_1);
    cfmakeraw(&options_UART_1); 
    cfsetispeed(&options_UART_1, B38400);
    tcsetattr(fd_UART_1, TCSANOW, &options_UART_1);

    uart_print(fd_UART_1, "\r\n!!! STARTED PROGRAM !!!\r\n");

    rtems_status_code status;

    status = rtems_task_create(
        rtems_build_name('P','U','S','3'),
        2,                         /* priority */
        10*RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &PUS_3
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_create(
        rtems_build_name('U','R','T','1'),
        1,                         /* priority */
        10*RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &handle_UART_OUT_OBC
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_create(
        rtems_build_name('U','R','T','0'),
        1,                         /* priority */
        10*RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &handle_UART_Rx_Callback
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_create(
        rtems_build_name('U','R','T','2'),
        2,                         /* priority */
        10*RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &handle_UART_IN_OBC
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_message_queue_create(
        rtems_build_name('Q', 'U', 'E', '1'),  // unique 4-character name
        10,                         // number of messages (queue length)
        sizeof(UART_OUT_OBC_msg),                         // size of each message in bytes
        RTEMS_DEFAULT_ATTRIBUTES,            // default attributes (FIFO)
        &queue_1_id                             // pointer to receive queue ID
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_message_queue_create(
        rtems_build_name('Q', 'U', 'E', '2'),  // unique 4-character name
        10,                         // number of messages (queue length)
        sizeof(UART_OUT_OBC_msg),                         // size of each message in bytes
        RTEMS_DEFAULT_ATTRIBUTES,            // default attributes (FIFO)
        &queue_2_id                             // pointer to receive queue ID
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_start(PUS_3, PUS_3_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_start(handle_UART_OUT_OBC, handle_UART_OUT_OBC_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_start(handle_UART_Rx_Callback, handle_UART_Rx_Callback_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_start(handle_UART_IN_OBC, handle_UART_IN_OBC_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    uart_print(fd_UART_1, "END OF INIT TASK!\r\n");

    rtems_task_delete(RTEMS_SELF);
}