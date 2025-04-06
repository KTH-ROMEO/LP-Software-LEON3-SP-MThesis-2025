#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

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
#include <stdio.h>

rtems_task Print_Task(rtems_task_argument argument)
{
      rtems_status_code status;
    iprintf("Started Print Task ... \r\n");

    rtems_task_wake_after(1);

}

rtems_task Read_Task(rtems_task_argument argument)
{

    rtems_status_code status;

    iprintf("Started Read Task ... \r\n");

    // rtems_task_wake_after(100);
    rtems_task_wake_after(1);

}

/* The Init task creates and starts the two UART tasks */
rtems_task Init(rtems_task_argument argument)
{
    rtems_id print_task_id, read_task_id, third_task_id;
    rtems_status_code status;

    iprintf("\r\nStarted progrm ... \r\n");

    status = rtems_task_create(
        rtems_build_name('P','R','T','1'),
        2,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &print_task_id
    );
    if (status != RTEMS_SUCCESSFUL) {
        printf("Init: Error creating Print_Task\n");
        exit(1);
    }

    status = rtems_task_create(
        rtems_build_name('R','D','T','1'),
        2,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &read_task_id
    );
    if (status != RTEMS_SUCCESSFUL) {
        printf("Init: Error creating Read_Task\n");
        exit(1);
    }

    status = rtems_task_create(
        rtems_build_name('R','D','T','2'),
        2,                         /* priority */
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &third_task_id
    );
    if (status != RTEMS_SUCCESSFUL) {
        printf("Init: Error creating Read_Task\n");
        exit(1);
    }

    status = rtems_task_start(print_task_id, Print_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        iprintf("Init: Error starting Print_Task\n");
        exit(1);
    }

    status = rtems_task_start(read_task_id, Read_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        iprintf("Init: Error starting Read_Task\n");
        exit(1);
    }

    status = rtems_task_start(third_task_id, Read_Task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        iprintf("Init: Error starting Read_Task\n");
        exit(1);
    }

    /* Delete the Init task after starting the others */
    rtems_task_delete(RTEMS_SELF);
}
