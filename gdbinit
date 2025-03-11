set disassemble-next-line on
#b _start
b user_syscall.c:45
target remote : 1234
b trap.c:83
c
b syscall.c:42
c
