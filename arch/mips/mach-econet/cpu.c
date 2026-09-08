// SPDX-License-Identifier: GPL-2.0
/*
 * Author: Matheus Sampaio Queiroga <srherobrine20@gmail.com>
 */
#include <fdt_support.h>
#include <init.h>
#include <sysreset.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <soc/airoha/pkgids.h>

#define ECONET_NP_SCU_BASE \
	((void __iomem *)CKSEG1ADDR(AIROHA_NP_SCU_BASE))

#define ECONET_CHIP_SCU_BASE \
	((void __iomem *)CKSEG1ADDR(AIROHA_CHIP_SCU_BASE))

#define ECONET_EFUSE_BASE \
	((void __iomem *)CKSEG1ADDR(AIROHA_EFUSE_BASE))

const char *econet_get_soc_name(void)
{
	return airoha_soc_name_from_mips_mem(ECONET_NP_SCU_BASE,
										 ECONET_CHIP_SCU_BASE,
										 ECONET_EFUSE_BASE);
}

int print_cpuinfo(void)
{
	printf("SoC:   EcoNet %s\n", econet_get_soc_name());

	return 0;
}
