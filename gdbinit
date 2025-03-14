set disassemble-next-line on
#b _start
target remote : 1234
b user.c:32
b trap.c:82
c
