#include <drv/apbuart.h>
// #include <stdio.h>

// Command for the simulator: ./tsim-leon3 -gdb  -uart0 /dev/ptmx -uart1 /dev/ptmx
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

// int main() {
//     struct apbuart_priv *dev;
//     struct apbuart_config cfg;
//     cfg.baud = 38400;
//     cfg.parity = APBUART_PAR_NONE;
//     cfg.flow = 0;
//     cfg.mode = APBUART_MODE_NONINT;

//     int a = 0;

//     apbuart_autoinit();

//     int dev_cnt = apbuart_dev_count();

//     char msg[15] = "Hello World!\r\n";

//     for(int i = 0; i<15; i ++)
//     {
//         uart_send_byte(dev, &cfg, msg[i]);
//     }

//     for(int i = 0; i<15; i ++)
//     {
//         uart_send_byte(dev, &cfg, msg[i]);
//     }

//     char data;
//     uart_read_byte(dev, &cfg, &data);

//     uart_send_byte(dev, &cfg, data);

//     // USE STDIO library function when NOT in debug mode, as it gets blocked
//     // printf("FINISHED!\n");

//     return 0;
// }

int main()
{

    uint8_t txfifobuf[32];
    uint8_t rxfifobuf[32];

    uint8_t userdata[4] = {'A', 'B', 'C', 'D'};

    struct apbuart_priv *device;
    struct apbuart_config cfg;
    cfg.baud = 38400;
    cfg.parity = APBUART_PAR_NONE;
    cfg.flow = 0;
    cfg.mode = APBUART_MODE_NONINT;
    cfg.txfifobuflen = 32;
    cfg.txfifobuf = txfifobuf;
    cfg.rxfifobuflen = 32;
    cfg.rxfifobuf = rxfifobuf;

    int count;
    int j;
    
    apbuart_autoinit();


    device = apbuart_open(1);
    if (!device)
        return -1; /* Failure */
    apbuart_config(device, &cfg);


    int i = 0;
    do{
    i = apbuart_write(device, userdata, 4);
    }while(i<2);

    i = apbuart_drain(device);
    // i = uart_send_data(device, userdata, 4);

    apbuart_close(device);
    return 0;
}
