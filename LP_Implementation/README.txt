Normal TSIM3 run:

./tsim-leon3 ../../../LP-Software-LEON3-SP-MThesis-2025/LP_Implementation/main -uart1 /dev/ptmx -uart0 /dev/ptmx

GDB TSIM3 run:

./tsim-leon3 -gdb  -uart0 /dev/ptmx -uart1 /dev/ptmx
source ENV_setup.sh
$GDB -x config.gdb -tui main