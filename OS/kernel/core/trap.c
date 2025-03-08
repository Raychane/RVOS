#include "os.h"
#include "riscv.h"

extern void trap_vector(void);
extern void uart_isr(void);
extern void timer_handler(void);
extern void schedule(void);
extern void handle_syscall(reg_t epc, reg_t a0, reg_t a1, reg_t a2, reg_t a3, reg_t a4);

void trap_init()
{
	/*
	 * set the trap-vector base-address for machine-mode
	 */
	w_mtvec((reg_t)trap_vector);
}

void external_interrupt_handler()
{
	int irq = plic_claim();

	if (irq == UART0_IRQ)
	{
		uart_isr();
	}
	else if (irq)
	{
		printf("unexpected interrupt irq = %d\n", irq);
	}

	if (irq)
	{
		plic_complete(irq);
	}
}

reg_t trap_handler(reg_t epc, reg_t cause)
{
<<<<<<< Updated upstream:OS/trap.c
	printf("异常发生！epc = %x, cause = %x\n", epc, cause);
=======
// printf("异常发生！epc = %x, cause = %x\n", epc, cause);
>>>>>>> Stashed changes:OS/kernel/core/trap.c
	reg_t return_pc = epc;
	reg_t cause_code = cause & 0xfff;

	if (cause & 0x80000000)
	{
		/* 异步陷阱 - 中断 */
		switch (cause_code)
		{
		case 3:
			uart_puts("软件中断！\n");
			/*
			 * 清除软件中断
			 */
			int id = r_mhartid();
			*(uint32_t *)CLINT_MSIP(id) = 0;

			// 切换到内核调度任务
			schedule();
			break;
		case 7:
			uart_puts("定时器中断！\n");
			timer_handler();
			break;
		case 11:
			uart_puts("外部中断！\n");
			external_interrupt_handler();
			break;
		default:
			uart_puts("未知的异步异常！\n");
			break;
		}
	}
	else
	{
		/* 同步陷阱 - 异常 */
<<<<<<< Updated upstream:OS/trap.c
		printf("同步异常!, code = %d, epc = %d\n", cause_code, epc);
		panic("OOPS! 无法处理的异常！");
		return_pc += 4;
=======
		switch (cause_code)
		{
		case 8:  // 环境调用（来自用户模式）
			// 系统调用处理
			return_pc += 4;  // 跳过 ecall 指令
			handle_syscall(epc, r_a0(), r_a1(), r_a2(), r_a3(), r_a4());
			break;
		default:
			printf("同步异常!, code = %d, epc = %x\n", cause_code, epc);
			panic("OOPS! 无法处理的异常！");
			return_pc += 4;
			break;
		}
>>>>>>> Stashed changes:OS/kernel/core/trap.c
	}

	return return_pc;
}
