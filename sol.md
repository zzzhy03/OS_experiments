在kernel/defs.h, user/user.h头文件中分别添加int getprocs(void);函数声明，并在proc.c里面完成函数定义

在usys.pl中添加entry("getprocs");

在 syscall.c 中添加系统调用函数 extern uint64 sys_getprocs(void);的声明，同时在系统调用列表syscalls中添加这个系统调用，同时在 syscall.h 中添加。

在sysproc.c这个文件中实现 sys_getprocs(实际上就是getprocs套壳)。

将测试代码放在./user目录下
Makefile UPROGS 中添加test_getprocs.c

make qemu