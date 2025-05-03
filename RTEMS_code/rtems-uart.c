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
#define LEON3 1


 /* for device driver prototypes */

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS             4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS	32




//    #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER


//    #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART



//   #include <grlib/ambapp_bus.h>
//    /* APBUART0 */
//    struct drvmgr_key grlib_drv_res_apbuart0[] =
//    {
//    	{"mode", DRVMGR_KT_INT, {(unsigned int)1}},
//    	{"syscon", DRVMGR_KT_INT, {(unsigned int)0}},
//    	DRVMGR_KEY_EMPTY
//    };
//    /* APBUART1 */
//    struct drvmgr_key grlib_drv_res_apbuart1[] =
//    {
//   	{"mode", DRVMGR_KT_INT, {(unsigned int)1}},
//   	{"syscon", DRVMGR_KT_INT, {(unsigned int)1}},
//   	DRVMGR_KEY_EMPTY
//    };



//  #include <drvmgr/drvmgr_confdefs.h>




#include <rtems/confdefs.h>

int fd, fd_1;


struct termios options, options_1;

rtems_task Task_1(rtems_task_argument argument)
{
    // rtems_status_code status;
    char start_buffer[17] = "Started Task 1\r\n";
    write(fd, start_buffer, 17);

    char buffer[25] = "Task 1 running ...\r\n";
    for(;;)
    {
        write(fd, buffer, 25);
        rtems_task_wake_after(100);
    }

}

rtems_task Task_2(rtems_task_argument argument)
{
    // rtems_status_code status;
    char start_buffer[17] = "Started Task 2\r\n";
    write(fd, start_buffer, 17);

    char buffer[25] = "Task 2 running ...\r\n";
    for(;;)
    {
        write(fd, buffer, 25);
        rtems_task_wake_after(100);
    }

}

/* The Init task creates and starts the two UART tasks */
rtems_task Init(rtems_task_argument argument)
{

    fd = open("/dev/console", O_RDWR);
    tcgetattr(fd, &options);
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);

    fd_1 = open("/dev/console_b", O_RDWR);
    tcgetattr(fd_1, &options_1);
    cfsetispeed(&options_1, B38400);
    cfsetospeed(&options_1, B38400);

    iprintf("Started Application \r\n");

    char buffer[15] = "UART WORKING!\r\n";
    write(fd, buffer, sizeof(buffer));
    // write(fd_1, buffer, sizeof(buffer));

    char arr[15] = {0x55, 0x41, 0x52, 0x54, 0x20, 0x57, 0x4F, 0x52, 0x4B, 0x49, 0x4E, 0x47, 0x21, 0x30, 0x00};

    write(fd_1, arr, 15);
    
    iprintf("RTEMS_DRVMGR_STARTUP %d\n", RTEMS_DRVMGR_STARTUP);

    rtems_id task_1_id, task_2_id, task_3_id;
    rtems_status_code status;

    status = rtems_task_create(
        rtems_build_name('P','R','T','1'),
        2,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &task_1_id
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    status = rtems_task_create(
        rtems_build_name('P','R','T','2'),
        2,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &task_2_id
    );
    if (status != RTEMS_SUCCESSFUL) {
        exit(1);
    }

    iprintf("RTEMS System configured to support max %d UARTs\n\n\n",BSP_NUMBER_OF_TERMIOS_PORTS);

    // status = rtems_task_start(task_1_id, Task_1, 0);
    // if (status != RTEMS_SUCCESSFUL) {
    //     exit(1);
    // }

    // status = rtems_task_start(task_2_id, Task_2, 0);
    // if (status != RTEMS_SUCCESSFUL) {
    //     exit(1);
    // }

    iprintf("GOT HERE 2\n");

    rtems_task_delete(RTEMS_SELF);
}
