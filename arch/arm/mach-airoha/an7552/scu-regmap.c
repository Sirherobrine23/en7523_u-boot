// SPDX-License-Identifier: GPL-2.0+

#include <dm/ofnode.h>
#include <linux/err.h>
#include <soc/airoha/scu-regmap.h>

static struct regmap *an7552_scu_regmap(unsigned int index)
{
	struct regmap *map;
	ofnode node;
	int ret;

	node = ofnode_by_compatible(ofnode_null(), "airoha,an7552-scu");
	if (!ofnode_valid(node))
		return ERR_PTR(-ENODEV);

	/* Match the EN7523 U-Boot provider: CHIP SCU first, NP SCU second. */
	ret = regmap_init_mem_index(node, &map, index);
	return ret ? ERR_PTR(ret) : map;
}

struct regmap *airoha_get_scu_regmap(void)
{
	return an7552_scu_regmap(1);
}

struct regmap *airoha_get_chip_scu_regmap(void)
{
	return an7552_scu_regmap(0);
}
