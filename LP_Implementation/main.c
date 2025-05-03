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


rtems_task handle_UART_OUT_OBC_Task(rtems_task_argument argument)
{
	UART_OUT_OBC_msg UART_OUT_msg_received;
	size_t received_size;
	rtems_status_code status;

    uart_print(fd_UART_0, "Receiving task started \n\r");

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
			// Add_SPP_PUS_and_send_TM(&UART_OUT_msg_received);
            uart_print(fd_UART_0, "Message received \n\r");
		}

		// Delay 1 tick (similar to osDelay(1))
		rtems_task_wake_after(1);
	}
}



rtems_task Task_2(rtems_task_argument argument)
{
    // rtems_status_code status;
    char start_buffer[17] = "Started Task 2\r\n";
    write(fd_UART_1, start_buffer, 17);

    char buffer[25] = "Task 2 running ...\r\n";
    for(;;)
    {
        write(fd_UART_1, buffer, 25);
        rtems_task_wake_after(100);
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

    iprintf("Started Application \r\n");

    uart_print(fd_UART_0, "UART 0 WORKING!\r\n");
    uart_print(fd_UART_1, "UART 1 WORKING!\r\n");


    rtems_status_code status;

    status = rtems_task_create(
        rtems_build_name('P','R','T','1'),
        2,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &PUS_3
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_create(
        rtems_build_name('Q','E','U','E'),
        1,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &handle_UART_OUT_OBC
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_message_queue_create(
        rtems_build_name('Q', 'U', 'E', 'E'),  // unique 4-character name
        5,                         // number of messages (queue length)
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


    uart_print(fd_UART_0, "END OF INIT TASK!\r\n");

    rtems_task_delete(RTEMS_SELF);
}