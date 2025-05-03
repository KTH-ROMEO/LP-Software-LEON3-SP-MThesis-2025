#include "PUS_3.h"
#include "General_Functions.h"

extern int fd_UART_1;
extern rtems_id queue_id;

rtems_task PUS_3_Task(rtems_task_argument argument)
{
    // rtems_status_code status;
    uart_print(fd_UART_1, "Started PUS 3 \r\n");

    for(;;)
    {
        UART_OUT_OBC_msg msg;
        // rtems_message_queue_send(queue_id, &msg, sizeof(msg));
        rtems_status_code send_status = rtems_message_queue_send(queue_id, &msg, sizeof(msg));
        switch (send_status) {
            case RTEMS_SUCCESSFUL:
                uart_print(fd_UART_1, "Periodic Data sent successfully\r\n");
                break;
            case RTEMS_TOO_MANY:
                uart_print(fd_UART_1, "Queue full, message not sent\r\n");
                break;
            case RTEMS_INVALID_ID:
                uart_print(fd_UART_1, "Invalid queue ID\r\n");
                break;
            default:
                uart_print(fd_UART_1, "Unknown send error\r\n");
                break;
        }
        rtems_task_wake_after(1000);
    }
}