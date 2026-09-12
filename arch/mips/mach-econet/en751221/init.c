// SPDX-License-Identifier: GPL-2.0+

#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/sizes.h>
#include <mach/en751221.h>
#include <soc/airoha/pkgids.h>

DECLARE_GLOBAL_DATA_PTR;

#define NP_SCU_BASE		((void __iomem *)CKSEG1ADDR(0x1fb00000))

#define EN7512_BOOTROM_RECOVERY_LATCH	BIT(0)

/*
 * The BootROM sets CHIP_SCU[0] when it enters the XMODEM recovery path, which
 * is how the chainloader gets us here. Leaving the latch set sends the SoC
 * straight back into recovery on the next warm reset.
 */
static void en751221_clear_bootrom_recovery_latch(void)
{
	void __iomem *reg = (void __iomem *)EN7512_CHIP_SCU_BASE;
	u32 val;

	val = __raw_readl(reg);
	if (!(val & EN7512_BOOTROM_RECOVERY_LATCH))
		return;

	__raw_writel(val & ~EN7512_BOOTROM_RECOVERY_LATCH, reg);

	/* Flush the MMIO write before a following reset. */
	(void)__raw_readl(reg);
}

int mach_cpu_init(void)
{
	en751221_clear_bootrom_recovery_latch();

	return 0;
}

int dram_init(void)
{
	u32 val = __raw_readl((void __iomem *)EN7512_REG_SAVE_INFO);
	u32 size_mb = val & EN7512_SAVE_DRAM_MASK;

	if (!size_mb)
		size_mb = 128;

	gd->ram_size = (phys_size_t)size_mb << 20;
	return 0;
}

ulong notrace get_tbclk(void)
{
	u32 val = __raw_readl((void __iomem *)EN7512_REG_SAVE_INFO);
	u32 clk = (val & EN7512_SAVE_CLK_MASK) >> EN7512_SAVE_CLK_SHIFT;

	/* CP0 Count advances at half the system clock on the MIPS34K. */
	if (clk)
		return (ulong)clk * 500000;

	return CONFIG_SYS_MIPS_TIMER_FREQ;
}

void _machine_restart(void)
{
	en751221_clear_bootrom_recovery_latch();

	__raw_writel(0x80000000, (void __iomem *)EN7512_RESET_CONTROL);

	for (;;)
		;
}
