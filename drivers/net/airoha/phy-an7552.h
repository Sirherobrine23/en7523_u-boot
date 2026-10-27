/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __AIROHA_PHY_AN7552_H
#define __AIROHA_PHY_AN7552_H

struct udevice;

#if CONFIG_IS_ENABLED(TARGET_AN7552)
int an7552_switch_phy_init(struct udevice *mdio_dev);
#else
static inline int an7552_switch_phy_init(struct udevice *mdio_dev)
{
	return -EOPNOTSUPP;
}
#endif

#endif
