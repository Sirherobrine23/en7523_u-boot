# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_PCS_AIROHA)	+= pcs-airoha-common.o
obj-$(CONFIG_PCS_AIROHA_AN7581)	+= pcs-an7581.o
obj-$(CONFIG_PCS_AIROHA_AN7583)	+= pcs-an7583.o
ifeq ($(CONFIG_AIROHA_ETH),y)
obj-$(CONFIG_TARGET_AN7552)	+= phy-an7552.o
endif
