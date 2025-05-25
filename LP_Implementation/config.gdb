target extended-remote localhost:1234
monitor reset halt
load

b Init
b main.c:129