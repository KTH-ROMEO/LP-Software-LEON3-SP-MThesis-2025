#include <rtems.h>
// #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#define UART_DEVICE "/dev/console"  /* or "/dev/console_b" if you prefer */


#include <bsp.h> /* for device driver prototypes */

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS             4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_SCHEDULER_TIMESLICE
#define CONFIGURE_SCHEDULER_TIMESLICE_PRIORITY 2  // Enable for priority-2 tasks

#include <rtems/confdefs.h>

/* If --drvmgr was enabled during the configuration of the RTEMS kernel */
#ifdef RTEMS_DRVMGR_STARTUP
 #ifdef LEON3
  /* Add Timer and UART Driver for this example */
  #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
   #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
  #endif
  #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
   #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
  #endif

  /* OPTIONAL FOR GRLIB SYSTEMS WITH APBUART AS SYSTEM CONSOLE.
   *
   * Assign a specific UART the system console (syscon=1). Note that the
   * default is to have APBUART0 as system/debug console, one must override
   * it by setting syscon=0 on APBUART0.
   *
   * Note also that the debug console does not have to be on the same UART
   * as the system console.
   *
   * Determine if console driver should do be interrupt driven (mode=1)
   * or polling (mode=0).
   */
   #include <grlib/ambapp_bus.h>
   /* APBUART0 */
   struct drvmgr_key grlib_drv_res_apbuart0[] =
   {
   	{"mode", DRVMGR_KT_INT, {(unsigned int)1}},
   	{"syscon", DRVMGR_KT_INT, {(unsigned int)0}},
   	DRVMGR_KEY_EMPTY
   };
   /* APBUART1 */
   struct drvmgr_key grlib_drv_res_apbuart1[] =
   {
  	{"mode", DRVMGR_KT_INT, {(unsigned int)1}},
  	{"syscon", DRVMGR_KT_INT, {(unsigned int)1}},
  	DRVMGR_KEY_EMPTY
   };
  #if 0
   /* LEON3 System with driver configuration for 2 APBUARTs, the
    * the rest of the AMBA device drivers use their defaults.
    */
   struct drvmgr_bus_res grlib_drv_resources =
   {
     .next = NULL,
     .resource = {
   	{DRIVER_AMBAPP_GAISLER_APBUART_ID, 0, &grlib_drv_res_apbuart0[0]},
        {DRIVER_AMBAPP_GAISLER_APBUART_ID, 1, &grlib_drv_res_apbuart1[0]},
   	DRVMGR_RES_EMPTY
     }
   };

   /* Override defualt debug UART assignment.
    * 0 = Default APBUART. APBUART[0], but on MP system CPU0=APBUART0,
    *     CPU1=APBUART1...
    * 1 = APBUART[0]
    * 2 = APBUART[1]
    * 3 = APBUART[2]
    * ...
    */
   int leon3_debug_uart_index = 2; /* second UART -- APBUART[1] */
  #endif
 #endif

 #include <drvmgr/drvmgr_confdefs.h>
#endif


// #include <stdio.h>

rtems_task Print_Task(rtems_task_argument argument)
{
    rtems_status_code status;
    // iprintf("Started Print Task ... \r\n");

    int fd = open("/dev/console", O_RDWR);

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    tcsetattr(fd, TCSANOW, &options);

    char buffer = 'a';
    int result = write(fd, &buffer, sizeof(buffer));
    int res = close(fd);

    // for(;;)
    // {
    //     // iprintf("Print Task running ...\r\n");
    //     rtems_task_wake_after(100);
    // }

    

    // iprintf("Finished Print Task ... \r\n");
}

rtems_task Read_Task(rtems_task_argument argument)
{

    rtems_status_code status;
    char buffer[100];

    // iprintf("Started Read Task ... \r\n");

    // // Option 1: Use fgets (blocking)
    // if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
    //     iprintf("You entered: %s", buffer);
    // }

    // for(;;)
    // {
    //     iprintf("Read Task running ...\r\n");
    //     rtems_task_wake_after(100);
    // }

    // rtems_task_wake_after(100);
    // rtems_task_wake_after(1);

}

/* The Init task creates and starts the two UART tasks */
rtems_task Init(rtems_task_argument argument)
{
    rtems_id print_task_id, read_task_id, third_task_id;
    rtems_status_code status;

    // iprintf("\r\nStarted progrm ... \r\n");

    status = rtems_task_create(
        rtems_build_name('P','R','T','1'),
        2,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &print_task_id
    );
    if (status != RTEMS_SUCCESSFUL) {
        // printf("Init: Error creating Print_Task\n");
        exit(1);
    }

    // status = rtems_task_create(
    //     rtems_build_name('R','D','T','1'),
    //     2,                         /* priority */
    //     RTEMS_MINIMUM_STACK_SIZE,
    //     RTEMS_DEFAULT_MODES,
    //     RTEMS_DEFAULT_ATTRIBUTES,
    //     &read_task_id
    // );
    // if (status != RTEMS_SUCCESSFUL) {
    //     // printf("Init: Error creating Read_Task\n");
    //     exit(1);
    // }

    // status = rtems_task_create(
    //     rtems_build_name('R','D','T','2'),
    //     2,                         /* priority */
    //     RTEMS_MINIMUM_STACK_SIZE,
    //     RTEMS_DEFAULT_MODES,
    //     RTEMS_DEFAULT_ATTRIBUTES,
    //     &third_task_id
    // );
    // if (status != RTEMS_SUCCESSFUL) {
    //     printf("Init: Error creating Read_Task\n");
    //     exit(1);
    // }


    int fd = open("/dev/console", O_RDWR);

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    tcsetattr(fd, TCSANOW, &options);

    char buffer = 'a';
    int result = write(fd, &buffer, sizeof(buffer));
    fflush(NULL);
    int res = close(fd);


    // status = rtems_task_start(print_task_id, Print_Task, 0);
    // if (status != RTEMS_SUCCESSFUL) {
    //     // iprintf("Init: Error starting Print_Task\n");
    //     exit(1);
    // }

    // status = rtems_task_start(read_task_id, Read_Task, 0);
    // if (status != RTEMS_SUCCESSFUL) {
    //     iprintf("Init: Error starting Read_Task\n");
    //     exit(1);
    // }

    // status = rtems_task_start(third_task_id, Read_Task, 0);
    // if (status != RTEMS_SUCCESSFUL) {
    //     iprintf("Init: Error starting Read_Task\n");
    //     exit(1);
    // }

    /* Delete the Init task after starting the others */
    rtems_task_delete(RTEMS_SELF);
}
