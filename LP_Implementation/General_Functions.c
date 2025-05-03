#include "General_Functions.h"

void uart_print(int fd_UART, const char *msg)
{
    write(fd_UART, msg, strlen(msg));
}