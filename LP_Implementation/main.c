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
#include "General_Functions.h"

void configure_port(int fd) {
    struct termios tty;

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return;
    }

    cfmakeraw(&tty); 

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
    }
}


int fd_UART_0, fd_UART_1;

struct termios options_UART_0, options_UART_1;

rtems_id PUS_3, handle_UART_OUT_OBC, task_2_id, handle_UART_IN_OBC;

rtems_id queue_id;

extern volatile UART_Rx_OBC_Msg UART_RxBuffer;
extern volatile uint16_t UART_recv_count;
extern volatile uint8_t UART_recv_char;

rtems_task PUS_3_Task(rtems_task_argument argument)
{
    // rtems_status_code status;
    uart_print(fd_UART_1, "Started PUS 3 \r\n");

    uint32_t current_ticks = 0;
    uint8_t periodic_report = 0;
    PUS_3_msg pus3_msg_received;

    for(;;)
    {
        // TO DO: PUS_3_set_report_frequency(pus3_msg_received.data, &pus3_msg_received);

        PUS_3_collect_HK_data(current_ticks);

        PUS_3_HK_send(&pus3_msg_received);

        rtems_task_wake_after(5000);
    }
}


rtems_task handle_UART_OUT_OBC_Task(rtems_task_argument argument)
{
	UART_OUT_OBC_msg UART_OUT_msg_received;
	size_t received_size;
	rtems_status_code status;

    // uart_print(fd_UART_0, "Receiving task started \n\r");

	while (1)
	{
		// Blocking receive (wait forever)
		status = rtems_message_queue_receive(
			queue_id,
			&UART_OUT_msg_received,
			&received_size,
			RTEMS_WAIT,
			RTEMS_NO_TIMEOUT
		);

		if (status == RTEMS_SUCCESSFUL )
		{
            // uart_print(fd_UART_0, "Message received \n\r");

            Add_SPP_PUS_and_send_TM(&UART_OUT_msg_received);
		}

		// Delay 1 tick (similar to osDelay(1))
		rtems_task_wake_after(1);
	}
}

rtems_task handle_UART_IN_OBC_Task(rtems_task_argument argument)
{
	UART_OUT_OBC_msg UART_OUT_msg_received;
	size_t received_size;
	rtems_status_code status;

    // uart_print(fd_UART_0, "Receiving task started \n\r");
    uart_print(fd_UART_1, "STARTED UART READING TASK\n");
    char UART_recv_char = {0xFF};

    uint8_t len; 

	while (1)
	{
        // uart_print(fd_UART_1, "STARTED UART READING TASK");

        len = read(fd_UART_0, &UART_recv_char, 1);
        // uart_print(fd_UART_1, "\nNew carachter: \n");
        // uart_print_2(fd_UART_1, &UART_recv_char, len);
        
        UART_RxBuffer.RxBuffer[UART_recv_count] = UART_recv_char;

        // Check if this is the end of frame (0x00 terminator) or the buffer is full
		if (UART_recv_char == 0x00 || UART_recv_count >= MAX_COBS_FRAME_LEN - 1)
		{
			// Mark the frame size
			UART_RxBuffer.frame_size = UART_recv_count + 1;

			UART_recv_count = 0;
			UART_recv_char = 0xff;

			// Signal the task that a complete frame is ready to be processed
			uart_print(fd_UART_1, "Received a new COBS message\n");

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

    // iprintf("Started Application \r\n");

    // uart_print(fd_UART_0, "UART 0 WORKING!\r\n");
    uart_print(fd_UART_1, "UART 1 WORKING!\r\n");


    rtems_status_code status;

    status = rtems_task_create(
        rtems_build_name('P','R','T','1'),
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
        rtems_build_name('U','R','T','2'),
        1,                         /* priority */
        10*RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &handle_UART_IN_OBC
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_message_queue_create(
        rtems_build_name('Q', 'U', 'E', 'E'),  // unique 4-character name
        10,                         // number of messages (queue length)
        sizeof(UART_OUT_OBC_msg),                         // size of each message in bytes
        RTEMS_DEFAULT_ATTRIBUTES,            // default attributes (FIFO)
        &queue_id                             // pointer to receive queue ID
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

    status = rtems_task_start(handle_UART_IN_OBC, handle_UART_IN_OBC_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }


    uart_print(fd_UART_1, "END OF INIT TASK!\r\n");

    rtems_task_delete(RTEMS_SELF);
}