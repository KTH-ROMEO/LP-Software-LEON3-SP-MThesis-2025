#include <rtems.h>
#include <bsp.h>


#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define UART_DEVICE "/dev/console"  /* or "/dev/console_b" if you prefer */


 /* for device driver prototypes */

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS             4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS	32


#include <rtems/confdefs.h>

/* The Init task creates and starts the two UART tasks */
rtems_task Init(rtems_task_argument argument)
{

    int fd = open("/dev/console", O_RDWR);

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);

    iprintf("Started Application \r\n");

    char buffer[3] = "a\r\n";
    write(fd, buffer, sizeof(buffer));

    char input[3];

    //  read(fd, buffer, 16);

    iprintf("GOT HERE\n");

    exit(0);
}
