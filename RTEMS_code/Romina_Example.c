// #include <rtems.h>
#include <bsp.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define UART_DEVICE "/dev/console"  /* or "/dev/console_b" if you prefer */
#define LEON3 1

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS             4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS	32

#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 4

#include <rtems/confdefs.h>


int fd_UART_0;

struct termios options_UART_0;

void uart_print(int fd_UART, const char *msg)
{
    write(fd_UART, msg, strlen(msg));
}


/* The Init task creates and starts the two UART tasks */
rtems_task Init(rtems_task_argument argument)
{
    fd_UART_0 = open("/dev/console", O_RDWR);
    tcgetattr(fd_UART_0, &options_UART_0);
    cfsetispeed(&options_UART_0, B38400);
    cfsetospeed(&options_UART_0, B38400);

    uart_print(fd_UART_0, "APPLICATION STARTED!!!\r\n");

    uart_print(fd_UART_0, "UART 1 WORKING!\r\n");

    while(1)
    {
        uart_print(fd_UART_0, "PERIODIC PRINT\r\n");
        rtems_task_wake_after(100);
    }

    rtems_task_delete(RTEMS_SELF);
}