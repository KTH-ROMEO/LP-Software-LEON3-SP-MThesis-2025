#!/bin/sh

export PATH=/opt/sparc-bcc-2.3.1-gcc/bin:$PATH
export MANPATH=/opt/sparc-bcc-2.3.1-gcc/man:$MANPATH

export GCC=/opt/sparc-bcc-2.3.1-gcc/bin/sparc-gaisler-elf-gcc
export FLAGS='-g -mcpu=leon3 -qbsp=leon3 -ldrv '

export GDB=/opt/sparc-bcc-2.3.1-gcc/bin/sparc-gaisler-elf-gdb

