#!/bin/sh

export PATH=/opt/rcc-1.3.2-gcc/bin:$PATH
export MANPATH=/opt/rcc-1.3.2-gcc/man:$MANPATH

export GCC=/opt/rcc-1.3.2-gcc/bin/sparc-gaisler-rtems5-gcc
export FLAGS='-g -mcpu=leon3 -qbsp=leon3 '

export GDB=/opt/rcc-1.3.2-gcc/bin/sparc-gaisler-rtems5-gdb
