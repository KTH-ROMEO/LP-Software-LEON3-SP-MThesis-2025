// #include <rtems.h>
#include <bsp.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "main.h"

#include <rtems/confdefs.h>

#include "PUS_3.h"
#include "General_Functions.h"


int fd_UART_0, fd_UART_1;

struct termios options_UART_0, options_UART_1;

rtems_id PUS_3, handle_UART_OUT_OBC, task_2_id;

rtems_id queue_id;

rtems_task PUS_3_Task(rtems_task_argument argument)
{
    // rtems_status_code status;
    uart_print(fd_UART_1, "Started PUS 3 \r\n");

    uint32_t current_ticks = 0;
    uint8_t periodic_report = 0;
    PUS_3_msg pus3_msg_received;

    for(;;)
    {
        // UART_OUT_OBC_msg msg;

        // rtems_status_code send_status = rtems_message_queue_send(queue_id, &msg, sizeof(msg));
        // switch (send_status) {
        //     case RTEMS_SUCCESSFUL:
        //         uart_print(fd_UART_1, "Periodic Data sent successfully\r\n");
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

        // TO DO: PUS_3_set_report_frequency(pus3_msg_received.data, &pus3_msg_received);

        PUS_3_collect_HK_data(current_ticks);

        PUS_3_HK_send(&pus3_msg_received);

        rtems_task_wake_after(1000);
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

/* The Init task creates and starts the two UART tasks */
rtems_task Init(rtems_task_argument argument)
{
    fd_UART_0 = open("/dev/console", O_RDWR);
    tcgetattr(fd_UART_0, &options_UART_0);
    cfsetispeed(&options_UART_0, B38400);
    cfsetospeed(&options_UART_0, B38400);

    fd_UART_1 = open("/dev/console_b", O_RDWR);
    tcgetattr(fd_UART_1, &options_UART_1);
    cfsetispeed(&options_UART_1, B38400);
    cfsetospeed(&options_UART_1, B38400);

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
        rtems_build_name('U','A','R','T'),
        1,                         /* priority */
        10*RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &handle_UART_OUT_OBC
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


    uart_print(fd_UART_1, "END OF INIT TASK!\r\n");

    rtems_task_delete(RTEMS_SELF);
}