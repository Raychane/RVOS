set disassemble-next-line on
#b _start
b user/tasks/user.c:27
target remote : 1234
c
