#include <drv/apbuart.h>
// #include <stdio.h>

// Command for the simulator: ./tsim-leon3 -gdb -uart0 /dev/ptmx -uart1 /dev/ptmx
// Command for Minicom: sudo minicom -D /dev/pts/1 -b 38400
// Command for Compiler: $GCC uart_test.c -o uart_test $FLAGS
// Command for GDB: $GDB -x config.gdb -tui uart_test

void uart_send_byte(struct apbuart_priv *dev, struct apbuart_config *cfg, char byte) {
    dev = apbuart_open(1);
    apbuart_config(dev, cfg);

    int i = 0;

    do {
        i = apbuart_outbyte(dev, byte);
    } while (1 != i);

    apbuart_close(dev);
}

void uart_read_byte(struct apbuart_priv *dev, struct apbuart_config *cfg, char *data) {
    dev = apbuart_open(0);
    apbuart_config(dev, cfg);
    
    do {
        *data = apbuart_inbyte(dev);
    } while (*data < 0);
    apbuart_close(dev);
}

int main() {
    struct apbuart_priv *dev;
    struct apbuart_config cfg;
    cfg.baud = 38400;
    cfg.parity = APBUART_PAR_NONE;
    cfg.flow = 0;
    cfg.mode = APBUART_MODE_NONINT;

    int a = 0;

    apbuart_autoinit();

    int dev_cnt = apbuart_dev_count();

    char msg[15] = "Hello World!\r\n";

    for(int i = 0; i<15; i ++)
    {
        uart_send_byte(dev, &cfg, msg[i]);
    }

    for(int i = 0; i<15; i ++)
    {
        uart_send_byte(dev, &cfg, msg[i]);
    }

    char data;
    uart_read_byte(dev, &cfg, &data);

    uart_send_byte(dev, &cfg, data);

    // USE STDIO library function when NOT in debug mode, as it gets blocked
    // printf("FINISHED!\n");

    return 0;
}
