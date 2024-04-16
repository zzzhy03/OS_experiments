elf
- 可执行文件的格式规范

针对Qemu的“-machine virt”参数设计，意味着运行环境包括
  - RAM
  - ROM，其中存放启动程序
  - 一个串口，连接到键盘和屏幕
  - 磁盘

RiscV的三种模式
- Machine mode：CPU启动时的状态，拥有最高权限
  - 有点类似x86的实模式
  - 通常用于对计算机做初始配置
  - Xv6在该模式下执行几行代码，然后就切换到Supervisor mode
- Supervisor mode：通常意义上的内核态
- 系统调用指令：ecall

用户态虚拟地址分布，从低至高：代码、全局数据、栈、堆、trapframe、trampoline

采用了RiscV的39位地址模式，但Xv6仅使用低一半，即38位，故最大虚拟地址MAXVA=$2^{38}-1$=0x3fffffffff
- Xv6使用39位虚拟地址，每个页4KB，页号27位，页表项最多$2^{27}$个
- 物理地址56位，由于每个页4KB，物理页号44位，最多可有$2^{44}$个物理页面
- 三级页表，9-9-9
内核空间采用最简化方案
- 直接映射，虚拟地址基本等同于物理地址，除了少数特例
- 假定物理内存128MB
- 假定每个进程的内核栈固定为4KB
Xv6把所有空闲物理内存按页组织成链表
- 物理内存从KERNBASE开始，存放内核程序、内核全局数据，然后是空闲物理内存
  - 空闲物理内存，从全局数组end开始，到PHYSTOP为止，内核连接时（kernel.ld），会将end设置在内核数据区的末尾
Xv6只支持基本的分页


ecall指令使CPU跳到预定义地址，此处的程序切换内核栈，执行系统调用程序，sret使系统调用返回用户态


启动过程

- ROM中的boot loader载入kernel
  - 载入到物理地址0x80000000，更低的地址包含外设IO端口
- 注意：以下过程是每个CPU都要同时执行的
- CPU在machine mode下，执行kernel/entry.S中的入口点汇编程序
  - 此时没有启用分页机制
  - 设置栈
    - RiscV的栈向低地址方向增长
  - 最后调用start()函数，定义于kernel/start.c
- 继续在machine mode下，执行kernel/start.c中的start()函数
  - 核心功能：做各种准备，最终执行mret指令，进入supervisor mode
  - RiscV默认在machine mode下处理所有中断，但可选择部分中断委托给supervisor mode处理，全委托
  - 设置定时器，kernel/start.c中的timerinit()函数，间隔设置为约0.1s，具体细节不懂？
    - 注意此时仍处于machine mode，即定时器中断将产生于machine mode
- 在supervisor mode下，执行kernel/main.c中的main()函数
  - 对于0号CPU，执行各种初始化，调用kernel/proc.c中的userinit()，启动第一个进程
    - 最后将全局变量started置为1
  - 其余CPU，循环检查全局变量started是否从0变为1，一旦变为1，则执行本CPU需要的初始化
  - 所有CPU，完成初始化后，执行kernel/proc.c中的scheduler()，内核完成启动，进入正轨
    - main() - scheduler() 无限循环，工作于启动之初设置的栈上，不属于任何进程
    - 对比Linux，完成启动后，最终同样进入无限循环，不属于任何进程，但循环体只是do_idle()
- user/init.c进程完成以下工作
  - 创建并打开终端文件，构成标准输入、输出、错误
  - 创建shell进程


发生陷入时，RiscV的硬件行为

- 关中断
- 设置回程
- 跳转到supervisor mode指定程序

注意：此时CPU

- 没有切换到内核页表
- 没有切换到内核栈
- 没有保存PC以外的任何寄存器

- 启动时，main()调用trapinithart()，此时处于内核态，stvec写入kernelvec起始地址
- 陷入内核态时，stvec写入kernelvec起始地址，对应kernelvec.S
- 返回用户态时，stvec写入trampoline中userret起始地址，对应trampoline.S

trampoline
- 在用户态和内核态都被映射在相同的虚拟地址
- 在用户态的页表中，未标注PTE_U标识，所以虽在视野范围内，但用户态无法访问
- 包含两段程序：uservec和userret
- usertrap()
分三种情况做核心处理
    - 如果scause为8，表示系统调用，把trapframe的epc字段加4（表示返回时不是回到ecall指令，而是回到ecall之后那条指令），然后开启中断，调用syscall()
    - 否则，尝试调用devintr()，处理外部中断，返回值是中断源编号，如果返回值大于0，表示确有外部中断，则一切已妥当
    - 否则，发生了异常，调用setkilled()撤销该进程
- usertrapret()

内核态发生陷入，不可能因为系统调用，只有两种可能的原因：设备中断、异常
kernelvec.S，kernelvec，kerneltrap()，kernelvec

系统调用传参规范
- Xv6的系统调用使用a7寄存器存放调用功能编号
- Xv6的系统调用使用a0-a5寄存器存放系统调用的参数
参数处理
- argXXX()
