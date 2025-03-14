#include "os.h"
#include "riscv.h"

extern void trap_vector(void);
extern void uart_isr(void);
extern void timer_handler(void);
extern void schedule(void);
extern void handle_syscall(reg_t epc, reg_t cause);

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
	reg_t return_pc = epc;
	reg_t cause_code = cause & 0xfff;

	if (cause & 0x80000000)
	{
		/* 异步陷阱 - 中断 */
		switch (cause_code)
		{
		case 3:
			// uart_puts("软件中断！\n");
			/*
			 * 清除软件中断
			 */
			{
				int id = r_mhartid();
				*(uint32_t *)CLINT_MSIP(id) = 0;

				// 切换到内核调度任务
				schedule();
				break;
			}
		case 7:
			// uart_puts("定时器中断！\n");
			timer_handler();
			break;
		case 11:
			// uart_puts("外部中断！\n");
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
		switch (cause_code)
		{
		case 8:  // 用户态ecall
		case 11: // 机器态ecall
			return_pc += 4;  // 跳过ecall指令
			handle_syscall(epc, cause);
			break;
		default:
			panic("无法处理的异常!");
			break;
		}
	}

	// 简化特权级管理 - 统一使用任务创建时的特权级
	if (_current >= 0) {
		reg_t task_mstatus = tasks[_current].ctx.mstatus;
		reg_t mstatus = r_mstatus();
		mstatus &= ~MSTATUS_MPP;
		mstatus |= (task_mstatus & MSTATUS_MPP);
		w_mstatus(mstatus);
	}

	return return_pc;
}
