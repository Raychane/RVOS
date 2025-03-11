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
	// printf("异常发生！epc = %x, cause = %x\n", epc, cause);
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
		case 8:	 // 环境调用（来自用户模式）
		case 11: // 环境调用（来自机器模式）
			// 系统调用处理
			return_pc += 4;				// 跳过 ecall 指令
			handle_syscall(epc, cause); // 只传递epc和cause

			// 确保正确设置返回的特权级别
			if (cause_code == 8)
			{
				// 用户模式调用，确保返回用户模式
				reg_t mstatus = r_mstatus();
				mstatus &= ~MSTATUS_MPP; // 清除MPP位(设为用户模式)
				w_mstatus(mstatus);
			}
			else if (cause_code == 11)
			{
				// 机器模式调用，确保返回机器模式
				reg_t mstatus = r_mstatus();
				mstatus &= ~MSTATUS_MPP;
				mstatus |= 3 << 11; // 设置MPP为机器模式(3)
				w_mstatus(mstatus);
			}
			break;
		default:
			printf("同步异常!, code = %d, epc = %x\n", cause_code, epc);
			panic("OOPS! 无法处理的异常！");
			return_pc += 4;
			break;
		}
	}

	return return_pc;
}
