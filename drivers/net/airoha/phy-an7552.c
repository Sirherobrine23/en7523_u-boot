// SPDX-License-Identifier: GPL-2.0+
/*
 * AN7552/AN7563 integrated PHY initialization.
 * Derived from Linux drivers/net/phy/mediatek/airoha-ge-soc.c and the
 * A7552.04.02 SDK private/tcphy/tcetherphy_7552.c calibration profile.
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <miiphy.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>

#include "phy-an7552.h"

#define AIROHA_DEFAULT_PORT0_ADDR 9

#define MTK_PHY_TXVLD_DA_RG			0x12
#define   MTK_PHY_DA_TX_I2MPB_A_GBE_MASK	GENMASK(15, 10)
#define   MTK_PHY_DA_TX_I2MPB_A_TBT_MASK	GENMASK(5, 0)
#define MTK_PHY_TX_I2MPB_TEST_MODE_A2		0x16
#define   MTK_PHY_DA_TX_I2MPB_A_HBT_MASK	GENMASK(15, 10)
#define   MTK_PHY_DA_TX_I2MPB_A_TST_MASK	GENMASK(5, 0)
#define MTK_PHY_TX_I2MPB_TEST_MODE_B1		0x17
#define   MTK_PHY_DA_TX_I2MPB_B_GBE_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_B_TBT_MASK	GENMASK(5, 0)
#define MTK_PHY_TX_I2MPB_TEST_MODE_B2		0x18
#define   MTK_PHY_DA_TX_I2MPB_B_HBT_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_B_TST_MASK	GENMASK(5, 0)
#define MTK_PHY_TX_I2MPB_TEST_MODE_C1		0x19
#define   MTK_PHY_DA_TX_I2MPB_C_GBE_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_C_TBT_MASK	GENMASK(5, 0)
#define MTK_PHY_TX_I2MPB_TEST_MODE_C2		0x20
#define   MTK_PHY_DA_TX_I2MPB_C_HBT_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_C_TST_MASK	GENMASK(5, 0)
#define MTK_PHY_TX_I2MPB_TEST_MODE_D1		0x21
#define   MTK_PHY_DA_TX_I2MPB_D_GBE_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_D_TBT_MASK	GENMASK(5, 0)
#define MTK_PHY_TX_I2MPB_TEST_MODE_D2		0x22
#define   MTK_PHY_DA_TX_I2MPB_D_HBT_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_D_TST_MASK	GENMASK(5, 0)
#define MTK_PHY_RG_ANA_CAL_RG0			0xdb
#define   MTK_PHY_RG_CAL_CKINV			BIT(12)
#define   MTK_PHY_RG_ANA_CALEN			BIT(8)
#define   MTK_PHY_RG_REXT_CALEN			BIT(4)
#define   MTK_PHY_RG_ZCALEN_A			BIT(0)
#define MTK_PHY_RG_ANA_CAL_RG1			0xdc
#define   MTK_PHY_RG_ZCALEN_B			BIT(12)
#define   MTK_PHY_RG_ZCALEN_C			BIT(8)
#define   MTK_PHY_RG_ZCALEN_D			BIT(4)
#define   MTK_PHY_RG_TXVOS_CALEN		BIT(0)
#define MTK_PHY_RG_ANA_CAL_RG5			0xe0
#define   MTK_PHY_RG_REXT_TRIM_MASK		GENMASK(13, 8)
#define   MTK_PHY_RG_REXT_ZCAL_CTRL_MASK	GENMASK(5, 0)
#define MTK_PHY_RG_ANA_CAL_RG6			0xe1
#define MTK_PHY_RG_CR_TX_AMP_OFFSET_A_B		0x172
#define   MTK_PHY_CR_TX_AMP_OFFSET_A_MASK	GENMASK(13, 8)
#define   MTK_PHY_CR_TX_AMP_OFFSET_B_MASK	GENMASK(6, 0)
#define MTK_PHY_RG_CR_TX_AMP_OFFSET_C_D		0x173
#define   MTK_PHY_CR_TX_AMP_OFFSET_C_MASK	GENMASK(13, 8)
#define   MTK_PHY_CR_TX_AMP_OFFSET_D_MASK	GENMASK(6, 0)
#define MTK_PHY_RG_AD_CAL_COMP			0x17a
#define   MTK_PHY_AD_CAL_COMP_OUT_MASK		GENMASK(8, 8)
#define MTK_PHY_RG_AD_CAL_CLK			0x17b
#define   MTK_PHY_DA_CAL_CLK			BIT(0)
#define MTK_PHY_RG_AD_CALIN			0x17c
#define   MTK_PHY_DA_CALIN_FLAG			BIT(0)
#define MTK_PHY_RG_DASN_DAC_IN0_A		0x17d
#define MTK_PHY_RG_DASN_DAC_IN0_B		0x17e
#define MTK_PHY_RG_DASN_DAC_IN0_C		0x17f
#define MTK_PHY_RG_DASN_DAC_IN0_D		0x180
#define MTK_PHY_RG_DASN_DAC_IN1_A		0x181
#define MTK_PHY_RG_DASN_DAC_IN1_B		0x182
#define MTK_PHY_RG_DASN_DAC_IN1_C		0x183
#define MTK_PHY_RG_DASN_DAC_IN1_D		0x184
#define MTK_PHY_RG_BG_RASEL			0x115
#define   MTK_PHY_RG_BG_RASEL_MASK		GENMASK(2, 0)

enum { REXT, TX_OFFSET, TX_AMP, TX_R50, RX_OFFSET };
enum { NO_PAIR, PAIR_A, PAIR_B, PAIR_C, PAIR_D };
enum airoha_mdi_resister_type {
	MDI_0R,
	MDI_5R,

	MDI_TYPE_MAX,
};

enum airoha_transformer_type {
	TXMR,
	DISCRETE,

	TRANSFORMER_TYPE_MAX,
};

enum airoha_calib_const_type {
	TX_AMP_TEST_A,
	TX_AMP_TEST_B,
	TX_AMP_TEST_C,
	TX_AMP_TEST_D,
	TX_AMP_1G_A,
	TX_AMP_1G_B,
	TX_AMP_1G_C,
	TX_AMP_1G_D,
	TX_AMP_100M_A,
	TX_AMP_100M_B,
	TX_AMP_10M_A,
	TX_AMP_10M_B,
	R50_A,
	R50_B,
	R50_C,
	R50_D,

	CALIB_CONST_TYPE_MAX,
};

struct an7552_phy_shared {
	bool rext_sw_calib_done;
	int error;
	const u8 *r50_cal_tbl;
	enum airoha_mdi_resister_type mdi_resister_type;
	enum airoha_transformer_type transformer_type[4];
};

struct airoha_mmd_reg {
	int devad;
	u16 reg;
	u16 val;
};

/* AN7552 A7552.04.02 PHY tuning, separate from the EN7523 profile. */
static const struct airoha_mmd_reg an7552_ge_init_regs[] = {
	{ MDIO_MMD_VEND2, 0x0044, 0x00a0 },
	{ MDIO_MMD_VEND2, 0x027c, 0x0808 },
	{ MDIO_MMD_VEND2, 0x027b, 0x1177 },
	{ MDIO_MMD_VEND2, 0x0417, 0x7775 },
	{ MDIO_MMD_VEND2, 0x0024, 0xc007 },
	{ MDIO_MMD_VEND2, 0x0025, 0x003f },
	{ MDIO_MMD_VEND2, 0x0026, 0xc007 },
	{ MDIO_MMD_VEND2, 0x0027, 0x003f },
	{ MDIO_MMD_VEND2, 0x0021, 0x800a },
	{ MDIO_MMD_VEND1, 0x0000, 0x0190 },
	{ MDIO_MMD_VEND1, 0x0001, 0x01c8 },
	{ MDIO_MMD_VEND1, 0x0002, 0x01c2 },
	{ MDIO_MMD_VEND1, 0x0003, 0x0100 },
	{ MDIO_MMD_VEND1, 0x0004, 0x020d },
	{ MDIO_MMD_VEND1, 0x0005, 0x020a },
	{ MDIO_MMD_VEND1, 0x0006, 0x038f },
	{ MDIO_MMD_VEND1, 0x0007, 0x03c8 },
	{ MDIO_MMD_VEND1, 0x0008, 0x03c4 },
	{ MDIO_MMD_VEND1, 0x0009, 0x0300 },
	{ MDIO_MMD_VEND1, 0x000a, 0x0008 },
	{ MDIO_MMD_VEND1, 0x000b, 0x0002 },
	{ MDIO_MMD_VEND1, 0x0011, 0x0f00 },
	{ MDIO_MMD_VEND1, 0x0013, 0x0000 },
	{ MDIO_MMD_VEND1, 0x0014, 0x0000 },
	{ MDIO_MMD_VEND1, 0x0044, 0x0000 },
	{ MDIO_MMD_VEND1, 0x0176, 0x6600 },
	{ MDIO_MMD_VEND1, 0x0177, 0x0066 },
	{ MDIO_MMD_VEND1, 0x0041, 0x3333 },
	{ MDIO_MMD_VEND1, 0x0040, 0x0000 },
	{ MDIO_MMD_VEND1, 0x0201, 0x4000 },
	{ MDIO_MMD_VEND1, 0x003d, 0x0000 },
	{ MDIO_MMD_VEND1, 0x0198, 0x0001 },
	{ MDIO_MMD_VEND1, 0x003e, 0x0000 },
	{ MDIO_MMD_VEND1, 0x023c, 0x0a20 },
	{ MDIO_MMD_VEND1, 0x01a3, 0x00d2 },
	{ MDIO_MMD_VEND1, 0x01a4, 0x010e },
	{ MDIO_MMD_VEND1, 0x0123, 0xffff },
	{ MDIO_MMD_VEND1, 0x0147, 0x0000 },
	{ MDIO_MMD_VEND1, 0x0236, 0x0020 },
	{ MDIO_MMD_VEND1, 0x0190, 0x0110 },
	{ MDIO_MMD_VEND1, 0x0191, 0x4444 },
	{ MDIO_MMD_VEND1, 0x00a6, 0x0350 },
	{ MDIO_MMD_VEND1, 0x00e7, 0x5555 },
	{ MDIO_MMD_VEND1, 0x00e9, 0x0001 },
	{ MDIO_MMD_VEND1, 0x00fe, 0x0000 },
	{ MDIO_MMD_VEND1, 0x00e6, 0x1111 },
	{ MDIO_MMD_VEND1, 0x0023, 0x0885 },
	{ MDIO_MMD_VEND1, 0x0024, 0x0885 },
	{ MDIO_MMD_VEND1, 0x0025, 0x0885 },
	{ MDIO_MMD_VEND1, 0x0026, 0x0885 },
	{ MDIO_MMD_AN, 0x003c, 0x0006 },
};

static const u8 an7552_zcal_to_r50ohm[2][64] = {
	{
		127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
		127, 126, 122, 117, 112, 109, 104, 101, 97, 94, 90, 88, 84, 80, 78, 74,
		72, 68, 66, 64, 61, 58, 56, 53, 51, 48, 47, 44, 42, 40, 38, 36,
		34, 32, 31, 28, 27, 24, 24, 22, 20, 18, 16, 16, 14, 12, 11, 9,
	},
	{
		127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
		127, 127, 127, 127, 127, 127, 127, 127, 123, 119, 115, 112, 108, 104, 100, 96,
		94, 92, 88, 85, 82, 80, 76, 74, 72, 68, 66, 64, 62, 60, 56, 55,
		52, 50, 48, 46, 44, 42, 40, 39, 36, 35, 32, 32, 30, 28, 27, 25,
	},
};

static const s8 an7552_tx_comp[TRANSFORMER_TYPE_MAX][MDI_TYPE_MAX][CALIB_CONST_TYPE_MAX][4] = {
	[TXMR][MDI_5R] = {
		[TX_AMP_TEST_A] = { 17, 16, 19, 14 },
		[TX_AMP_TEST_B] = { 16, 16, 16, 16 },
		[TX_AMP_TEST_C] = { 14, 17, 17, 13 },
		[TX_AMP_TEST_D] = { 17, 15, 15, 17 },
		[TX_AMP_1G_A] = { 17, 16, 19, 14 },
		[TX_AMP_1G_B] = { 16, 16, 16, 16 },
		[TX_AMP_1G_C] = { 14, 17, 17, 13 },
		[TX_AMP_1G_D] = { 17, 15, 15, 17 },
		[TX_AMP_100M_A] = { 31, 32, 31, 28 },
		[TX_AMP_100M_B] = { 31, 29, 27, 30 },
		[TX_AMP_10M_A] = { 22, 22, 22, 23 },
		[TX_AMP_10M_B] = { 22, 22, 22, 23 },
		[R50_A] = { 7, 8, 9, 11 },
		[R50_B] = { 8, 9, 10, 11 },
		[R50_C] = { 10, 10, 10, 11 },
		[R50_D] = { 10, 10, 10, 10 },
	},
	[DISCRETE][MDI_5R] = {
		[TX_AMP_TEST_A] = { 3, 2, 1, 3 },
		[TX_AMP_TEST_B] = { 1, -1, 0, 4 },
		[TX_AMP_TEST_C] = { 4, 4, 1, 2 },
		[TX_AMP_TEST_D] = { 3, 3, 1, 2 },
		[TX_AMP_1G_A] = { 3, 2, 1, 3 },
		[TX_AMP_1G_B] = { 1, -1, 0, 4 },
		[TX_AMP_1G_C] = { 4, 4, 1, 2 },
		[TX_AMP_1G_D] = { 3, 3, 1, 2 },
		[TX_AMP_100M_A] = { 17, 17, 16, 15 },
		[TX_AMP_100M_B] = { 17, 15, 14, 18 },
		[TX_AMP_10M_A] = { 22, 22, 22, 23 },
		[TX_AMP_10M_B] = { 22, 22, 22, 23 },
		[R50_A] = { 0, 0, 0, 0 },
		[R50_B] = { 0, 0, 0, 0 },
		[R50_C] = { 0, 0, 0, 0 },
		[R50_D] = { 0, 0, 0, 0 },
	},
};

static int an7552_phy_read(struct phy_device *phydev, int devad, int reg)
{
	struct an7552_phy_shared *shared = phydev->priv;
	int ret = phy_read(phydev, devad, reg);

	if (ret < 0 && !shared->error)
		shared->error = ret;
	return ret;
}

static int an7552_phy_write(struct phy_device *phydev, int devad, int reg,
			    u16 value)
{
	struct an7552_phy_shared *shared = phydev->priv;
	int ret = phy_write(phydev, devad, reg, value);

	if (ret < 0 && !shared->error)
		shared->error = ret;
	return ret;
}

/* The switch MDIO controller supports native Clause 45 transactions. */
static struct phy_driver an7552_phy_access = {
	.features = PHY_GBIT_FEATURES,
	.read_mmd = an7552_phy_read,
	.write_mmd = an7552_phy_write,
};

/* REXT and the analog comparator are shared by all four PHYs. */
static int airoha_p0_read_mmd(struct phy_device *phydev, int devad, u32 reg)
{
	struct phy_device p0 = *phydev;

	p0.addr = AIROHA_DEFAULT_PORT0_ADDR;
	return phy_read_mmd(&p0, devad, reg);
}

static int airoha_p0_write_mmd(struct phy_device *phydev, int devad, u32 reg,
			       u16 val)
{
	struct phy_device p0 = *phydev;

	p0.addr = AIROHA_DEFAULT_PORT0_ADDR;
	return phy_write_mmd(&p0, devad, reg, val);
}

static int airoha_p0_modify_mmd(struct phy_device *phydev, int devad, u32 reg,
				u16 mask, u16 val)
{
	struct phy_device p0 = *phydev;

	p0.addr = AIROHA_DEFAULT_PORT0_ADDR;
	return phy_modify_mmd(&p0, devad, reg, mask, val);
}

static int airoha_cal_cycle_wait(struct phy_device *phydev)
{
	struct an7552_phy_shared *shared = phydev->priv;
	int ret;

	if (shared->error)
		return shared->error;

	/*
	 * The EN7523 SDK flow does not poll here: it raises DA_CALIN,
	 * waits 20 us for the analog comparator, reads AD_CAL_CLK/COMP,
	 * then lowers DA_CALIN.  Using the generic MTK helper can sample the
	 * comparator too early on EN7523 and drive TX AMP to the rail.
	 */
	ret = airoha_p0_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_AD_CALIN,
				   0, MTK_PHY_DA_CALIN_FLAG);
	if (ret)
		return ret;

	udelay(20);

	ret = airoha_p0_read_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_AD_CAL_CLK);
	if (ret < 0)
		goto out;

	if (!(ret & MTK_PHY_DA_CAL_CLK)) {
		ret = -ETIMEDOUT;
		goto out;
	}

	ret = airoha_p0_read_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_AD_CAL_COMP);
	if (ret < 0)
		goto out;

	ret = FIELD_GET(MTK_PHY_AD_CAL_COMP_OUT_MASK, ret);

out:
	airoha_p0_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_AD_CALIN,
			     MTK_PHY_DA_CALIN_FLAG, 0);

	return ret;
}

static int airoha_cal_cycle(struct phy_device *phydev, int devad,
			    u32 regnum, u16 mask, u16 cal_val)
{
	int ret;

	ret = phy_modify_mmd(phydev, devad, regnum, mask, cal_val);
	if (ret)
		return ret;

	ret = airoha_cal_cycle_wait(phydev);

	return ret;
}

static int airoha_p0_cal_cycle(struct phy_device *phydev, int devad,
			       u32 regnum, u16 mask, u16 cal_val)
{
	int ret;

	ret = airoha_p0_modify_mmd(phydev, devad, regnum, mask, cal_val);
	if (ret)
		return ret;

	return airoha_cal_cycle_wait(phydev);
}

static int airoha_rext_cal_sw(struct phy_device *phydev)
{
	int calibration_polarity;
	u8 zcal_ctrl = 32;
	int first_calib;
	int ret;

	/* BG voltage output */
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0xc000);

	/* tst_mode2 */
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND2, 0xff, 0x2);
	airoha_p0_modify_mmd(phydev, MDIO_MMD_VEND2, 0xff,
			     GENMASK(15, 4) | GENMASK(1, 0), 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			    MTK_PHY_RG_CAL_CKINV | MTK_PHY_RG_ANA_CALEN |
			    MTK_PHY_RG_REXT_CALEN);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6, 0);

	first_calib = airoha_p0_cal_cycle(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG5,
					  MTK_PHY_RG_REXT_ZCAL_CTRL_MASK, zcal_ctrl);

	if (first_calib < 0) {
		printf("REXT SW calibration failed.\n");
		return -EINVAL;
	}

	/* If REXT calibration failed:
	 * - increase dB until calibration succeed.
	 * If REXT calibration succeeded:
	 * - decrease dB until calibration fail to fine tune it.
	 */
	if (first_calib == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	while (zcal_ctrl > 0 &&
	       zcal_ctrl < FIELD_MAX(MTK_PHY_RG_REXT_ZCAL_CTRL_MASK)) {
		zcal_ctrl += calibration_polarity;

		ret = airoha_p0_cal_cycle(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG5,
					  MTK_PHY_RG_REXT_ZCAL_CTRL_MASK, zcal_ctrl);
		/* Exit if we either failed or succeeded compared to the
		 * first calibration result. (aka we finished fine tuning or
		 * we succeeded with calibration)
		 */
		if (ret != first_calib)
			break;
	}

	if (ret < 0) {
		printf("REXT SW calibration failed.\n");
		return -EINVAL;
	}

	if (zcal_ctrl == 0 ||
	    zcal_ctrl == FIELD_MAX(MTK_PHY_RG_REXT_ZCAL_CTRL_MASK)) {
		zcal_ctrl = 32;
		printf("REXT SW calibration saturation. Defaulting to %x.\n",
		       zcal_ctrl);
	}

	airoha_p0_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG5,
			     MTK_PHY_RG_REXT_TRIM_MASK,
			     FIELD_PREP(MTK_PHY_RG_REXT_TRIM_MASK, zcal_ctrl));
	airoha_p0_modify_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_RG_BG_RASEL,
			     MTK_PHY_RG_BG_RASEL_MASK,
			     FIELD_PREP(MTK_PHY_RG_BG_RASEL_MASK, zcal_ctrl >> 3));

	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0, 0);

	return 0;
}

static int airoha_tx_offset_cal_sw(struct phy_device *phydev, u8 txg_calen_x)
{
	u16 dev1e_145_tmp, bmcr_tmp;
	int calibration_polarity;
	u16 reg_dac1, reg_dac2;
	int zcal_ctrl = 0;
	int first_calib;
	u16 reg, mask;
	int ret;

	/* BG voltage output */
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0xc000);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
		      MTK_PHY_RG_ANA_CALEN);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1,
		      MTK_PHY_RG_TXVOS_CALEN);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			    MTK_PHY_RG_ANA_CALEN);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1,
			    MTK_PHY_RG_TXVOS_CALEN);

	/* Force 1G full duplex for calibration */
	bmcr_tmp = an7552_phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_FULLDPLX | BMCR_SPEED1000);

	/* Force MDI */
	dev1e_145_tmp = phy_read_mmd(phydev, MDIO_MMD_VEND1, 0x0145);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x0145, 0x1010);

	/* 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x96,
		      0x8000);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x3e,
		      0xf808);

	switch (txg_calen_x) {
	case PAIR_A:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(12));

		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_A;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_A;
		reg = MTK_PHY_RG_CR_TX_AMP_OFFSET_A_B;
		mask = MTK_PHY_CR_TX_AMP_OFFSET_A_MASK;
		break;
	case PAIR_B:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(8));

		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_B;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_B;
		reg = MTK_PHY_RG_CR_TX_AMP_OFFSET_A_B;
		mask = MTK_PHY_CR_TX_AMP_OFFSET_B_MASK;
		break;
	case PAIR_C:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(4));

		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_C;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_C;
		reg = MTK_PHY_RG_CR_TX_AMP_OFFSET_C_D;
		mask = MTK_PHY_CR_TX_AMP_OFFSET_C_MASK;
		break;
	case PAIR_D:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(0));

		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_D;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_D;
		reg = MTK_PHY_RG_CR_TX_AMP_OFFSET_C_D;
		mask = MTK_PHY_CR_TX_AMP_OFFSET_D_MASK;
		break;
	default:
		return -EINVAL;
	}

	phy_write_mmd(phydev, MDIO_MMD_VEND1, reg_dac1,
		      0x8000);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, reg_dac2,
		      0x8000);

	first_calib = airoha_cal_cycle(phydev, MDIO_MMD_VEND1, reg, mask,
				       zcal_ctrl << __ffs(mask));

	if (first_calib < 0)
		return first_calib;

	/* If TX Offset calibration failed:
	 * - increase dB until calibration succeed.
	 * If TX Offset calibration succeeded:
	 * - decrease dB until calibration fail to fine tune it.
	 */
	if (first_calib == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	while (zcal_ctrl > -32 && zcal_ctrl < 32) {
		u32 val;

		zcal_ctrl += calibration_polarity;
		if (zcal_ctrl >= 0)
			val = zcal_ctrl;
		else
			/* BIT(5) signal negative number for TX Offset */
			val = BIT(5) | abs(zcal_ctrl);

		ret = airoha_cal_cycle(phydev, MDIO_MMD_VEND1, reg, mask,
				       val << __ffs(mask));
		/* Exit if we either failed or succeeded compared to the
		 * first calibration result. (aka we finished fine tuning or
		 * we succeeded with calibration)
		 */
		if (ret != first_calib)
			break;
	}

	if (ret < 0) {
		printf("TX Offset calibration failed.\n");
		return -EINVAL;
	}

	if (zcal_ctrl == -32 ||
	    zcal_ctrl == 32) {
		zcal_ctrl = 0;
		printf("TX Offset SW calibration saturation. Defaulting to %x.\n",
		       zcal_ctrl);

		phy_modify_mmd(phydev, MDIO_MMD_VEND1, reg, mask,
			       zcal_ctrl << __ffs(mask));
	}

	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0x0);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_A, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_B, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_C, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_D, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_A, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_B, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_C, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_D, 0);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x96, 0x0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x3e, 0xc000);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, 0);

	/* Restore BMCR */
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, bmcr_tmp);

	/* Restore MDI */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x0145, dev1e_145_tmp);

	return 0;
}

static int airoha_tx_amp_cal_sw(struct phy_device *phydev, u8 txg_calen_x)
{
	u16 mask_gbe, mask_tbt, mask_tst, mask_hbt;
	u16 reg, reg_100, reg_dac1, reg_dac2;
	int calibration_polarity;
	int dev1e_145_tmp;
	int bmcr_tmp;
	u8 zcal_ctrl = 32;
	int first_calib;
	int ret = 0;

	bmcr_tmp = an7552_phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	if (bmcr_tmp < 0)
		return bmcr_tmp;

	dev1e_145_tmp = phy_read_mmd(phydev, MDIO_MMD_VEND1, 0x145);
	if (dev1e_145_tmp < 0)
		return dev1e_145_tmp;

	/* BG voltage output */
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0xc000);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x145, 0x1010);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
		      MTK_PHY_RG_CAL_CKINV | MTK_PHY_RG_ANA_CALEN);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1,
		      MTK_PHY_RG_TXVOS_CALEN);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6, 0);

	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			    MTK_PHY_RG_CAL_CKINV | MTK_PHY_RG_ANA_CALEN);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1,
			    MTK_PHY_RG_TXVOS_CALEN);

	/* Enable Tx VLD. */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x3e, 0xf808);

	/* Force 1G full duplex for calibration */
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_FULLDPLX | BMCR_SPEED1000);

	switch (txg_calen_x) {
	case PAIR_A:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(12));
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG0,
			      MTK_PHY_RG_CAL_CKINV |
			      MTK_PHY_RG_ANA_CALEN |
			      MTK_PHY_RG_ZCALEN_A);
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG1,
			      MTK_PHY_RG_TXVOS_CALEN);
		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_A;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_A;
		reg = MTK_PHY_TXVLD_DA_RG;
		mask_gbe = MTK_PHY_DA_TX_I2MPB_A_GBE_MASK;
		mask_tbt = MTK_PHY_DA_TX_I2MPB_A_TBT_MASK;
		reg_100 = MTK_PHY_TX_I2MPB_TEST_MODE_A2;
		mask_hbt = MTK_PHY_DA_TX_I2MPB_A_HBT_MASK;
		mask_tst = MTK_PHY_DA_TX_I2MPB_A_TST_MASK;
		break;
	case PAIR_B:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(8));
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG0,
			      MTK_PHY_RG_CAL_CKINV |
			      MTK_PHY_RG_ANA_CALEN);
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG1,
			      MTK_PHY_RG_ZCALEN_B |
			      MTK_PHY_RG_TXVOS_CALEN);
		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_B;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_B;
		reg = MTK_PHY_TX_I2MPB_TEST_MODE_B1;
		mask_gbe = MTK_PHY_DA_TX_I2MPB_B_GBE_MASK;
		mask_tbt = MTK_PHY_DA_TX_I2MPB_B_TBT_MASK;
		reg_100 = MTK_PHY_TX_I2MPB_TEST_MODE_B2;
		mask_hbt = MTK_PHY_DA_TX_I2MPB_B_HBT_MASK;
		mask_tst = MTK_PHY_DA_TX_I2MPB_B_TST_MASK;
		break;
	case PAIR_C:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(4));
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG0,
			      MTK_PHY_RG_CAL_CKINV |
			      MTK_PHY_RG_ANA_CALEN);
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG1,
			      MTK_PHY_RG_ZCALEN_C |
			      MTK_PHY_RG_TXVOS_CALEN);
		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_C;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_C;
		reg = MTK_PHY_TX_I2MPB_TEST_MODE_C1;
		mask_gbe = MTK_PHY_DA_TX_I2MPB_C_GBE_MASK;
		mask_tbt = MTK_PHY_DA_TX_I2MPB_C_TBT_MASK;
		reg_100 = MTK_PHY_TX_I2MPB_TEST_MODE_C2;
		mask_hbt = MTK_PHY_DA_TX_I2MPB_C_HBT_MASK;
		mask_tst = MTK_PHY_DA_TX_I2MPB_C_TST_MASK;
		break;
	case PAIR_D:
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, BIT(0));
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG0,
			      MTK_PHY_RG_CAL_CKINV |
			      MTK_PHY_RG_ANA_CALEN);
		phy_write_mmd(phydev, MDIO_MMD_VEND1,
			      MTK_PHY_RG_ANA_CAL_RG1,
			      MTK_PHY_RG_ZCALEN_D |
			      MTK_PHY_RG_TXVOS_CALEN);
		reg_dac1 = MTK_PHY_RG_DASN_DAC_IN0_D;
		reg_dac2 = MTK_PHY_RG_DASN_DAC_IN1_D;
		reg = MTK_PHY_TX_I2MPB_TEST_MODE_D1;
		mask_gbe = MTK_PHY_DA_TX_I2MPB_D_GBE_MASK;
		mask_tbt = MTK_PHY_DA_TX_I2MPB_D_TBT_MASK;
		reg_100 = MTK_PHY_TX_I2MPB_TEST_MODE_D2;
		mask_hbt = MTK_PHY_DA_TX_I2MPB_D_HBT_MASK;
		mask_tst = MTK_PHY_DA_TX_I2MPB_D_TST_MASK;
		break;
	default:
		return -EINVAL;
	}

	phy_write_mmd(phydev, MDIO_MMD_VEND1, reg_dac1,
		      0x8000 | 0xf0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, reg_dac2,
		      0x8000 | 0xf0);

	first_calib = airoha_cal_cycle(phydev, MDIO_MMD_VEND1, reg,
				       mask_gbe, zcal_ctrl << __ffs(mask_gbe));
	if (first_calib < 0) {
		printf("TX AMP calibration failed.\n");
		ret = -EINVAL;
		goto restore;
	}

	/* If TX Amp calibration failed:
	 * - increase dB until calibration succeed.
	 * If TX Amp calibration succeeded:
	 * - decrease dB until calibration fail to fine tune it.
	 */
	if (first_calib == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	while (zcal_ctrl > 0 &&
	       zcal_ctrl < FIELD_MAX(MTK_PHY_RG_REXT_ZCAL_CTRL_MASK)) {
		zcal_ctrl += calibration_polarity;

		ret = airoha_cal_cycle(phydev, MDIO_MMD_VEND1, reg,
				       mask_gbe, zcal_ctrl << __ffs(mask_gbe));
		/* Exit if we either failed or succeeded compared to the
		 * first calibration result. (aka we finished fine tuning or
		 * we succeeded with calibration)
		 */
		if (ret != first_calib)
			break;
	}

	if (ret < 0) {
		printf("TX AMP calibration failed.\n");
		ret = -EINVAL;
		goto restore;
	}

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, reg,
		       mask_gbe | mask_tbt,
		       (zcal_ctrl << __ffs(mask_gbe)) |
		       (zcal_ctrl << __ffs(mask_tbt)));

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, reg_100,
		       mask_hbt | mask_tst,
		       (zcal_ctrl << __ffs(mask_hbt)) |
		       (zcal_ctrl << __ffs(mask_tst)));

	ret = 0;

restore:
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0x0);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_A, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_B, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_C, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN0_D, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_A, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_B, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_C, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DASN_DAC_IN1_D, 0);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x3e, 0xc000);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xdd, 0);

	/* Restore BMCR */
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, bmcr_tmp);

	/* Restore MDI */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x145, dev1e_145_tmp);

	return ret;
}

static int airoha_tx_r50_cal_sw(struct phy_device *phydev, u8 txg_calen_x)
{
	struct an7552_phy_shared *shared = phydev->priv;
	u16 dev1e_145_tmp, bmcr_tmp;
	int calibration_polarity;
	u8 zcal_ctrl = 32;
	int first_calib;
	u16 reg, shift;
	int ret;

	/* BG voltage output */
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0xc000);

	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			    MTK_PHY_RG_CAL_CKINV | MTK_PHY_RG_ANA_CALEN);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6,
			    0x10);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
		      MTK_PHY_RG_CAL_CKINV | MTK_PHY_RG_ANA_CALEN);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6,
		      0x10);

	/* Force 1G full duplex for calibration */
	bmcr_tmp = an7552_phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_FULLDPLX | BMCR_SPEED1000);

	/* Force MDI */
	dev1e_145_tmp = phy_read_mmd(phydev, MDIO_MMD_VEND1, 0x0145);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x0145, 0x1010);

	/* disable tx slew control */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x0185, 0x0000);

	switch (txg_calen_x) {
	case PAIR_A:
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG0,
				 MTK_PHY_RG_ZCALEN_A);
		break;
	case PAIR_B:
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG1,
				 MTK_PHY_RG_ZCALEN_B);
		break;
	case PAIR_C:
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG1,
				 MTK_PHY_RG_ZCALEN_C);
		break;
	case PAIR_D:
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG1,
				 MTK_PHY_RG_ZCALEN_D);
		break;
	default:
		return -EINVAL;
	}

	first_calib = airoha_p0_cal_cycle(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG5,
					  MTK_PHY_RG_REXT_ZCAL_CTRL_MASK, zcal_ctrl);

	if (first_calib < 0) {
		printf("TX r50 SW calibration failed.\n");
		return -EINVAL;
	}

	/* If TX r50 calibration failed:
	 * - increase dB until calibration succeed.
	 * If TX r50 calibration succeeded:
	 * - decrease dB until calibration fail to fine tune it.
	 */
	if (first_calib == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	while (zcal_ctrl > 0 &&
	       zcal_ctrl < FIELD_MAX(MTK_PHY_RG_REXT_ZCAL_CTRL_MASK)) {
		zcal_ctrl += calibration_polarity;

		ret = airoha_p0_cal_cycle(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG5,
					  MTK_PHY_RG_REXT_ZCAL_CTRL_MASK, zcal_ctrl);
		/* Exit if we either failed or succeeded compared to the
		 * first calibration result. (aka we finished fine tuning or
		 * we succeeded with calibration)
		 */
		if (ret != first_calib)
			break;
	}

	if (ret < 0) {
		printf("TX r50 SW calibration failed.\n");
		return -EINVAL;
	}

	if (zcal_ctrl == 0 ||
	    zcal_ctrl == FIELD_MAX(MTK_PHY_RG_REXT_ZCAL_CTRL_MASK))
		printf("TX r50 SW calibration saturation.\n");

	zcal_ctrl = shared->r50_cal_tbl[zcal_ctrl];

	shift = txg_calen_x == PAIR_A || txg_calen_x == PAIR_C ? 8 : 0;

	/* AN7552 has a seven-bit impedance code and per-byte enable. */
	reg = txg_calen_x <= PAIR_B ? 0x174 : 0x175;
	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, reg,
			     0xff << shift, (0x80 | zcal_ctrl) << shift);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0, 0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1, 0);

	/* Enable tx slew control */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x0185, 0x0001);

	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0x0);

	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6, 0x0);
	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG6, 0x0);

	/* Restore BMCR */
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, bmcr_tmp);

	/* Restore MDI */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x0145, dev1e_145_tmp);

	return ret < 0 ? ret : 0;
}

static int airoha_rx_offset_cal_sw(struct phy_device *phydev)
{
	/* Hw bypass, Fw cal */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x96, 0x8000);
	/* tx/rx_cal_criteria_value */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x37, 0x0033);

	/* rx offset cal by Hw setup */
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, 0x39,
			   BIT(14) | BIT(11));

	/* disable rtune calibration */
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2, 0x107,
			   BIT(12));

	/* bypass tx/rx dc offset cancellation process */
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, 0x171,
			 BIT(8) | BIT(7));

	/* rx offset calibration start */
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, 0x39,
			 BIT(13));

	/* rx offset calibration stop */
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, 0x39,
			   BIT(13));

	mdelay(10);

	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, 0x171,
			   BIT(8) | BIT(7));

	return 0;
}

static int airoha_cal_sw(struct phy_device *phydev, int cal_item,
			 u8 start_pair, u8 end_pair)
{
	u8 pair_n;
	int ret;

	for (pair_n = start_pair; pair_n <= end_pair; pair_n++) {
		switch (cal_item) {
		case REXT:
			ret = airoha_rext_cal_sw(phydev);
			break;
		case TX_OFFSET:
			ret = airoha_tx_offset_cal_sw(phydev, pair_n);
			break;
		case TX_AMP:
			ret = airoha_tx_amp_cal_sw(phydev, pair_n);
			break;
		case TX_R50:
			ret = airoha_tx_r50_cal_sw(phydev, pair_n);
			break;
		case RX_OFFSET:
			ret = airoha_rx_offset_cal_sw(phydev);
			break;
		default:
			return -EINVAL;
		}
		if (ret)
			return ret;
	}
	return 0;
}

static int airoha_start_cal(struct phy_device *phydev, int cal_item,
			    u8 start_pair, u8 end_pair)
{
	int ret;

	ret = airoha_cal_sw(phydev, cal_item, start_pair, end_pair);

	if (ret) {
		printf("cal %d failed\n", cal_item);
		return -EIO;
	}

	return 0;
}

static int airoha_phy_calib(struct phy_device *phydev)
{
	struct an7552_phy_shared *shared = phydev->priv;
	int ret;

	/* PreCalibrate Set */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x5c, 0x6666);

	if (!shared->rext_sw_calib_done) {
		ret = airoha_start_cal(phydev, REXT, NO_PAIR, NO_PAIR);
		if (ret)
			return ret;

		shared->rext_sw_calib_done = true;
	}

	ret = airoha_start_cal(phydev, TX_R50, PAIR_A, PAIR_D);
	if (ret)
		return ret;

	ret = airoha_start_cal(phydev, TX_OFFSET, PAIR_A, PAIR_D);
	if (ret)
		return ret;

	ret = airoha_start_cal(phydev, TX_AMP, PAIR_A, PAIR_D);
	if (ret)
		return ret;

	ret = airoha_start_cal(phydev, RX_OFFSET, NO_PAIR, NO_PAIR);
	if (ret)
		return ret;

	/* Gating, short with other pair */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x15, 0x0);

	airoha_p0_write_mmd(phydev, MDIO_MMD_VEND2, 0x100, 0x0);

	return 0;
}

static int an7552_phy_config_init(struct phy_device *phydev)
{
	static const u16 amp_regs[CALIB_CONST_TYPE_MAX] = {
		0x16, 0x18, 0x20, 0x22, 0x12, 0x17, 0x19, 0x21,
		0x16, 0x18, 0x12, 0x17, 0x174, 0x174, 0x175, 0x175,
	};
	static const u16 amp_masks[CALIB_CONST_TYPE_MAX] = {
		0x003f, 0x003f, 0x003f, 0x003f, 0xfc00, 0x3f00, 0x3f00, 0x3f00,
		0xfc00, 0x3f00, 0x003f, 0x003f, 0x7f00, 0x007f, 0x7f00, 0x007f,
	};
	struct an7552_phy_shared *shared = phydev->priv;
	ofnode node = phydev->node;
	u32 ohms;
	int port = phydev->addr - AIROHA_DEFAULT_PORT0_ADDR;
	int i, ret, val, comp;
	bool integrated;

	if (port < 0 || port >= 4)
		return -EINVAL;
	ohms = ofnode_read_u32_default(node, "airoha,mdi-resistor-ohms", 5);
	if (ohms != 0 && ohms != 5)
		return -EINVAL;
	integrated = ofnode_read_bool(node, "airoha,integrated-transformer");

	shared->mdi_resister_type = ohms ? MDI_5R : MDI_0R;
	shared->transformer_type[port] = integrated ? TXMR : DISCRETE;
	shared->r50_cal_tbl = an7552_zcal_to_r50ohm[!!ohms];

	for (i = 0; i < ARRAY_SIZE(an7552_ge_init_regs); i++) {
		const struct airoha_mmd_reg *reg = &an7552_ge_init_regs[i];

		ret = phy_write_mmd(phydev, reg->devad, reg->reg, reg->val);
		if (ret < 0)
			goto out;
	}
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0x52b5);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x11, 0x2422);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x12, 0x0008);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x10, 0x8000 | (1 << 11) | (0xf << 7) | (3 << 1));
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0);

	/* PreCalSet: use the AN7552 low-power and long-reach profile. */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x3c, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x3d, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x3e, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xc6, 0x53aa);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x268, 0x07f1);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x269, 0x1111);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x26a, 0);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x26b, 0x0074);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x26e, 0x00f6);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x26f, 0x6666);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x271, 0x2c02);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x272, 0x0c22);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x700, 0x0001);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x701, 0x0803);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x702, 0x01b6);
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0x703, 0x1111);
	for (i = 0x200; i <= 0x230; i += 2)
		phy_write_mmd(phydev, MDIO_MMD_VEND2, i, 0x2010);

	ret = airoha_phy_calib(phydev);
	if (ret)
		goto out;

	for (i = 0; i < CALIB_CONST_TYPE_MAX; i++) {
		u16 mask = amp_masks[i];

		val = phy_read_mmd(phydev, MDIO_MMD_VEND1, amp_regs[i]);
		if (val < 0) {
			ret = val;
			goto out;
		}
		comp = an7552_tx_comp[shared->transformer_type[port]]
				    [shared->mdi_resister_type][i][port];
		val = clamp_t(int, ((val & mask) >> __ffs(mask)) + comp,
			      0, mask >> __ffs(mask));
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, amp_regs[i],
				     mask, val << __ffs(mask));
		if (ret < 0)
			goto out;
	}

	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xe7,
		      ohms ? (integrated ? 0x6666 : 0x5555) : 0x2222);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xe9,
		      ohms && integrated ? 6 : 1);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xfe,
		      ohms && integrated ? 3 : 0);
	if (integrated && ohms) {
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x5c, 0x5555);
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x176, 0x5500);
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x177, 0x0055);
	}
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV, 0);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 1);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x3a18);
	an7552_phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0);
	ret = 0;
out:
	return ret ? ret : shared->error;
}

int an7552_switch_phy_init(struct udevice *mdio_dev)
{
	struct mdio_perdev_priv *mdio_priv;
	struct an7552_phy_shared shared = {};
	struct phy_device phydev = {
		.priv = &shared,
		.drv = &an7552_phy_access,
		.supported = PHY_GBIT_FEATURES,
		.advertising = PHY_GBIT_FEATURES,
		.autoneg = AUTONEG_ENABLE,
	};
	ofnode node;
	int ret, value;
	u32 addr;

	ret = device_probe(mdio_dev);
	if (ret)
		return ret;
	mdio_priv = dev_get_uclass_priv(mdio_dev);
	phydev.bus = mdio_priv->mii_bus;

	for (addr = 9; addr <= 12; addr++) {
		phydev.addr = addr;
		ret = an7552_phy_write(&phydev, MDIO_DEVAD_NONE, 0x1f, 0);
		if (ret)
			return ret;
		ret = an7552_phy_write(&phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);
		if (ret)
			return ret;
		ret = read_poll_timeout(an7552_phy_read, value,
					value < 0 || !(value & BMCR_RESET),
					1000, 500000, &phydev,
					MDIO_DEVAD_NONE, MII_BMCR);
		if (ret || value < 0)
			return ret ? ret : value;
	}

	dev_for_each_subnode(node, mdio_dev) {
		if (!ofnode_is_enabled(node))
			continue;
		ret = ofnode_read_u32(node, "reg", &addr);
		if (ret || addr < 9 || addr > 12)
			return -EINVAL;
		phydev.addr = addr;
		phydev.node = node;

		/* The SDK matches PHYSID2 only; do not fabricate the OUI. */
		value = phy_read(&phydev, MDIO_DEVAD_NONE, MII_PHYSID2);
		if (value < 0)
			return value;
		if (value != 0x0400)
			return -ENODEV;

		ret = an7552_phy_config_init(&phydev);
		if (ret)
			return ret;
		ret = phy_clear_bits(&phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_PDOWN);
		if (ret < 0)
			return ret;
		ret = genphy_config_aneg(&phydev);
		if (ret < 0)
			return ret;
	}

	return 0;
}
