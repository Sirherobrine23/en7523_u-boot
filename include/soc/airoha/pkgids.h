// SPDX-License-Identifier: GPL-2.0

#ifndef __AIROHA_CHIP_ID_H_
#define __AIROHA_CHIP_ID_H_

#include <linux/bitfield.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/regmap.h>
#include <linux/types.h>
#include <soc/airoha/scu-regmap.h>

#define AIROHA_PKG_ID_NAME(_id) [(_id)] = #_id

#define AIROHA_NP_SCU_BASE		0x1fb00000
#define AIROHA_NP_SCU_SIZE		0x960
#define AIROHA_CHIP_SCU_BASE		0x1fa20000
#define AIROHA_CHIP_SCU_SIZE		0x360
#define AIROHA_EFUSE_BASE		0x1fbf8000

#define AIROHA_NP_SCU_PDIDR		0x05c
#define AIROHA_NP_SCU_HIR		0x064
#define AIROHA_NP_SCU_SCREG_WR1	0x284
#define AIROHA_NP_SCU_MT751020_CFG	0x0f8

#define AIROHA_CHIP_SCU_751627_PKG	0x174
#define AIROHA_CHIP_SCU_7526C_PKG	0x1ec

#define AIROHA_EFUSE_VERIFY_DATA0	0x214
#define AIROHA_EFUSE_VERIFY_DATA1	0x218

#define AIROHA_NP_SCU_HIR_MASK		GENMASK(31, 16)
#define AIROHA_NP_SCU_PDIDR_MASK	GENMASK(15, 0)
#define AIROHA_NP_SCU_PACKAGE_ID_MASK	GENMASK(3, 0)
#define AIROHA_NP_SCU_PACKAGE_ID_EXT	BIT(7)

#define AIROHA_CHIP_SCU_751627_QFP	BIT(15)
#define AIROHA_CHIP_SCU_7526C_FP	BIT(10)

#define AIROHA_EFUSE_PKG_7526C_MASK		0x3c
#define AIROHA_EFUSE_PKG_MASK_751627		0xc0000
#define AIROHA_EFUSE_REMARK_BIT_751627		BIT(0)
#define AIROHA_EFUSE_PKG_REMARK_SHIFT_751627	2
#define AIROHA_EFUSE_PKG_MASK			GENMASK(5, 0)
#define AIROHA_EFUSE_REMARK_BIT			BIT(6)
#define AIROHA_EFUSE_PKG_REMARK_SHIFT		7
#define AIROHA_EFUSE_DDR3_BIT			BIT(23)
#define AIROHA_EFUSE_DDR3_REMARK_BIT		BIT(24)

enum airoha_pkg {
	/* AN7583 */
	AN7583_PKG = 0x10,

	/* AM7552 */
	AN7552_PKG = 0xf,

	/* EN7581 */
	EN7581_PKG = 0xe,

	/* EN7523 */
	EN7523_PKG = 0xc,

	/* EN7528 */
	EN7528_PKG = 0xb,

	/* EN7580 */
	EN7580_PKG = 0xa,

	/* EN7516, EN7527 */
	EN751627_PKG = 0x9,

	/* EN7526c, EN7522 */
	EN7526C_PKG = 0x8,

	/* EN7512, EN7521 */
	EN751221_PKG = 0x7,

	/* MT7505 */
	MT7505_PKG = 0x6,

	/* MT7510, MT7520 */
	MT751020_PKG = 0x5,
};

/*
 * Raw MIPS eFuse package encodings from the vendor SDK. These values are
 * family-relative and therefore may overlap.
 */
enum airoha_mips_efuse_pkg_id {
	AIROHA_EFUSE_EN7527H = 0x0,
	AIROHA_EFUSE_EN7527G = 0x0,
	AIROHA_EFUSE_EN7561G = 0xc0000,
	AIROHA_EFUSE_EN7516G = 0x80000,

	AIROHA_EFUSE_EN7526F = 0x00,
	AIROHA_EFUSE_EN7521F = 0x10,
	AIROHA_EFUSE_EN7521S = 0x20,
	AIROHA_EFUSE_EN7512 = 0x04,
	AIROHA_EFUSE_EN7526D = 0x01,
	AIROHA_EFUSE_EN7526FT = 0x11,
	AIROHA_EFUSE_EN7513 = 0x05,
	AIROHA_EFUSE_EN7526G = 0x02,
	AIROHA_EFUSE_EN7521G = 0x12,
	AIROHA_EFUSE_EN7513G = 0x06,
	AIROHA_EFUSE_EN7586 = 0x0a,
};

enum airoha_pkg_ids {
	/*EN7523*/
	EN7529DU,
	EN7529DT,
	EN7529CU,
	EN7562DU,
	EN7562DT,
	EN7562CU,
	EN7523GU,
	EN7523DU,
	EN7529GTH,
	EN7562GTH,
	EN7523SU,
	EN7529GTS,
	EN7562GTS,
	EN7529IT,
	EN7529CT,
	EN7562CT,
	EN7523DT,
	EN7529DTM,
	EN7562DTM,
	EN7529ITM,
	EN7529CTM,
	EN7562CTM,
	EN7523DTM,

	/*EN7528*/
	EN7528HU,
	EN7528DU,
	EN7561DU,
	EN7526FH,
	EN7521G,

	/* EN7580 */
	EN7580GT,
	EN7580ST,
	EN7580GAT,
	EN7565,
	EN7580,

	/* EN7516 */
	EN7516G,

	/* EN7527 */
	EN7527G,
	EN7561G,
	EN751627,

	/* EN7512 */
	EN7512,
	EN7513,
	EN7513G,

	/* EN7521, EN7521FC */
	EN7521FCUD,
	EN7521F,
	EN7521S,
	EN7526D,
	EN7526F,
	EN7526G,
	EN7526FT,
	EN7526FP,
	EN7526FT_C,
	EN751221,

	/* MT7520 */
	MT7520S,
	MT7520,
	MT7520G,
	MT7525,
	MT7525G,

	/* AN7581 */
	AN7581GT,
	AN7566GT,
	AN7581PT,
	AN7581ST,
	AN7551PT,
	AN7581CT,
	AN7581DT,
	AN7581FG,
	AN7581FP,
	AN7581FD,
	AN7551GT,
	AN7566PT,
	AN7581IT,
	AN7581SIT,

	/* AN7552 */
	AN7552CT,
	AN7552ST,
	AN7552FT,
	AN7563CT,
	AN7563PT,

	/* AN7583 */
	AN7583GT,
	AN7583GIT,
	AN7583CT,
	AN7583DT,
	RESERVED_PKGID_4,
	AN7583ST,
	AN9510GT,
	RESERVED_PKGID_7,
	AN7553GT,
	AN7553CT,
	AN7567GT,
	AN7567CT,
	AN7583ET,
	AN7583EIT,
	RESERVED_PKGID_14,
	RESERVED_PKGID_15,
	AN7583FG,
	RESERVED_PKGID_17,
	AN7583FP,
	AN7583FD,
	RESERVED_PKGID_20,
	AN7583FS,
	AN7583FF,

	/* MIPS variants missing from the vendor chipId_t table */
	EN7561HU,
	EN7528GT_EN7580,
	EN7527H,
	EN7586,
	MT7510,
	MT7511,

	END_PACKAGE_ID = 0xFFFFFFFF,
};

/* Compatibility with vendor/legacy detector identifiers. */
#define EN7526FH_EN7528DU	EN7526FH
#define EN7521G_EN7528DU	EN7521G
#define EN7526FHEN7528DU	EN7526FH
#define EN7521GEN7528DU	EN7521G

static const char *const airoha_pkg_id_names[] = {
	/* EN7523 */
	AIROHA_PKG_ID_NAME(EN7529DU),
	AIROHA_PKG_ID_NAME(EN7529DT),
	AIROHA_PKG_ID_NAME(EN7529CU),
	AIROHA_PKG_ID_NAME(EN7562DU),
	AIROHA_PKG_ID_NAME(EN7562DT),
	AIROHA_PKG_ID_NAME(EN7562CU),
	AIROHA_PKG_ID_NAME(EN7523GU),
	AIROHA_PKG_ID_NAME(EN7523DU),
	AIROHA_PKG_ID_NAME(EN7529GTH),
	AIROHA_PKG_ID_NAME(EN7562GTH),
	AIROHA_PKG_ID_NAME(EN7523SU),
	AIROHA_PKG_ID_NAME(EN7529GTS),
	AIROHA_PKG_ID_NAME(EN7562GTS),
	AIROHA_PKG_ID_NAME(EN7529IT),
	AIROHA_PKG_ID_NAME(EN7529CT),
	AIROHA_PKG_ID_NAME(EN7562CT),
	AIROHA_PKG_ID_NAME(EN7523DT),
	AIROHA_PKG_ID_NAME(EN7529DTM),
	AIROHA_PKG_ID_NAME(EN7562DTM),
	AIROHA_PKG_ID_NAME(EN7529ITM),
	AIROHA_PKG_ID_NAME(EN7529CTM),
	AIROHA_PKG_ID_NAME(EN7562CTM),
	AIROHA_PKG_ID_NAME(EN7523DTM),

	/* EN7528 */
	AIROHA_PKG_ID_NAME(EN7528HU),
	AIROHA_PKG_ID_NAME(EN7528DU),
	AIROHA_PKG_ID_NAME(EN7561DU),
	AIROHA_PKG_ID_NAME(EN7526FH),
	AIROHA_PKG_ID_NAME(EN7521G),

	/* EN7580 */
	AIROHA_PKG_ID_NAME(EN7580GT),
	AIROHA_PKG_ID_NAME(EN7580ST),
	AIROHA_PKG_ID_NAME(EN7580GAT),
	AIROHA_PKG_ID_NAME(EN7565),
	AIROHA_PKG_ID_NAME(EN7580),

	/* EN7516 */
	AIROHA_PKG_ID_NAME(EN7516G),

	/* EN7527 */
	AIROHA_PKG_ID_NAME(EN7527G),
	AIROHA_PKG_ID_NAME(EN7561G),
	AIROHA_PKG_ID_NAME(EN751627),

	/* EN7512 */
	AIROHA_PKG_ID_NAME(EN7512),
	AIROHA_PKG_ID_NAME(EN7513),
	AIROHA_PKG_ID_NAME(EN7513G),

	/* EN7521 / EN7526 */
	AIROHA_PKG_ID_NAME(EN7521FCUD),
	AIROHA_PKG_ID_NAME(EN7521F),
	AIROHA_PKG_ID_NAME(EN7521S),
	AIROHA_PKG_ID_NAME(EN7526D),
	AIROHA_PKG_ID_NAME(EN7526F),
	AIROHA_PKG_ID_NAME(EN7526G),
	AIROHA_PKG_ID_NAME(EN7526FT),
	AIROHA_PKG_ID_NAME(EN7526FP),
	AIROHA_PKG_ID_NAME(EN7526FT_C),
	AIROHA_PKG_ID_NAME(EN751221),

	/* MT7520 */
	AIROHA_PKG_ID_NAME(MT7520S),
	AIROHA_PKG_ID_NAME(MT7520),
	AIROHA_PKG_ID_NAME(MT7520G),
	AIROHA_PKG_ID_NAME(MT7525),
	AIROHA_PKG_ID_NAME(MT7525G),

	/* AN7581 */
	AIROHA_PKG_ID_NAME(AN7581GT),
	AIROHA_PKG_ID_NAME(AN7566GT),
	AIROHA_PKG_ID_NAME(AN7581PT),
	AIROHA_PKG_ID_NAME(AN7581ST),
	AIROHA_PKG_ID_NAME(AN7551PT),
	AIROHA_PKG_ID_NAME(AN7581CT),
	AIROHA_PKG_ID_NAME(AN7581DT),
	AIROHA_PKG_ID_NAME(AN7581FG),
	AIROHA_PKG_ID_NAME(AN7581FP),
	AIROHA_PKG_ID_NAME(AN7581FD),
	AIROHA_PKG_ID_NAME(AN7551GT),
	AIROHA_PKG_ID_NAME(AN7566PT),
	AIROHA_PKG_ID_NAME(AN7581IT),
	AIROHA_PKG_ID_NAME(AN7581SIT),

	/* AN7552 */
	AIROHA_PKG_ID_NAME(AN7552CT),
	AIROHA_PKG_ID_NAME(AN7552ST),
	AIROHA_PKG_ID_NAME(AN7552FT),
	AIROHA_PKG_ID_NAME(AN7563CT),
	AIROHA_PKG_ID_NAME(AN7563PT),

	/* AN7583 */
	AIROHA_PKG_ID_NAME(AN7583GT),
	AIROHA_PKG_ID_NAME(AN7583GIT),
	AIROHA_PKG_ID_NAME(AN7583CT),
	AIROHA_PKG_ID_NAME(AN7583DT),
	AIROHA_PKG_ID_NAME(AN7583ST),
	AIROHA_PKG_ID_NAME(AN9510GT),
	AIROHA_PKG_ID_NAME(AN7553GT),
	AIROHA_PKG_ID_NAME(AN7553CT),
	AIROHA_PKG_ID_NAME(AN7567GT),
	AIROHA_PKG_ID_NAME(AN7567CT),
	AIROHA_PKG_ID_NAME(AN7583ET),
	AIROHA_PKG_ID_NAME(AN7583EIT),
	AIROHA_PKG_ID_NAME(AN7583FG),
	AIROHA_PKG_ID_NAME(AN7583FP),
	AIROHA_PKG_ID_NAME(AN7583FD),
	AIROHA_PKG_ID_NAME(AN7583FS),
	AIROHA_PKG_ID_NAME(AN7583FF),

	/* MIPS variants */
	AIROHA_PKG_ID_NAME(EN7561HU),
	AIROHA_PKG_ID_NAME(EN7528GT_EN7580),
	AIROHA_PKG_ID_NAME(EN7527H),
	AIROHA_PKG_ID_NAME(EN7586),
	AIROHA_PKG_ID_NAME(MT7510),
	AIROHA_PKG_ID_NAME(MT7511),
};

static inline const char *airoha_pkg_id_name(enum airoha_pkg_ids id)
{
	if (id >= END_PACKAGE_ID ||
	    (unsigned int)id >= ARRAY_SIZE(airoha_pkg_id_names) ||
	    !airoha_pkg_id_names[id])
		return "unknown";
	return airoha_pkg_id_names[id];
}

static inline const char *
airoha_pkg_id_range_name(u32 pkgid, enum airoha_pkg_ids first,
			 enum airoha_pkg_ids last)
{
	u32 id;

	if (pkgid > (u32)(last - first))
		return NULL;

	id = first + pkgid;
	if (id >= ARRAY_SIZE(airoha_pkg_id_names) ||
	    !airoha_pkg_id_names[id])
		return NULL;

	return airoha_pkg_id_names[id];
}

static inline u32 airoha_efuse_pkgid(u32 value)
{
	if (value & AIROHA_EFUSE_REMARK_BIT)
		value >>= AIROHA_EFUSE_PKG_REMARK_SHIFT;

	return value & AIROHA_EFUSE_PKG_MASK;
}

static inline u32 airoha_efuse_pkgid_751627(u32 value)
{
	if (value & AIROHA_EFUSE_REMARK_BIT_751627)
		value >>= AIROHA_EFUSE_PKG_REMARK_SHIFT_751627;

	return value & AIROHA_EFUSE_PKG_MASK_751627;
}

static inline bool airoha_efuse_is_ddr3(u32 value)
{
	if (value & AIROHA_EFUSE_REMARK_BIT)
		return !!(value & AIROHA_EFUSE_DDR3_REMARK_BIT);

	return !!(value & AIROHA_EFUSE_DDR3_BIT);
}

static inline bool airoha_efuse_is_enp_mod(u32 value)
{
	if (value & BIT(3))
		return !!(value & BIT(5));

	return !!(value & BIT(1));
}

static inline bool airoha_efuse_is_ens_mod(u32 value)
{
	if (value & BIT(3))
		return !!(value & BIT(6));

	return !!(value & BIT(2));
}

static inline enum airoha_pkg airoha_pkg_from_id(u32 id)
{
	switch (id) {
	case AN7583_PKG:
	case 0x7583:
	case 0x9510:
	case 0x7553:
	case 0x7567:
		return AN7583_PKG;
	case AN7552_PKG:
	case 0x7552:
	case 0x7563:
		return AN7552_PKG;
	case EN7581_PKG:
	case 0x7581:
	case 0x7566:
	case 0x7551:
		return EN7581_PKG;
	case EN7523_PKG:
	case 0x7523:
	case 0x7529:
	case 0x7562:
		return EN7523_PKG;
	case EN7528_PKG:
	case 0x7528:
	case 0x7561:
		return EN7528_PKG;
	case EN7580_PKG:
	case 0x7580:
	case 0x7565:
		return EN7580_PKG;
	case EN751627_PKG:
	case 0x7516:
	case 0x7527:
		return EN751627_PKG;
	case EN7526C_PKG:
	case 0x7522:
		return EN7526C_PKG;
	case EN751221_PKG:
	case 0x7512:
	case 0x7513:
	case 0x7521:
	case 0x7526:
		return EN751221_PKG;
	case MT751020_PKG:
	case 0x7510:
	case 0x7520:
	case 0x7525:
		return MT751020_PKG;
	default:
		return 0;
	}
}

static inline const char *airoha_pkg_family_name(enum airoha_pkg pkg)
{
	switch (airoha_pkg_from_id(pkg)) {
	case AN7583_PKG:
		return "AN7583";
	case AN7552_PKG:
		return "AN7552";
	case EN7581_PKG:
		return "EN7581";
	case EN7523_PKG:
		return "EN7523";
	case EN7528_PKG:
		return "EN7528";
	case EN7580_PKG:
		return "EN7580";
	case EN751627_PKG:
		return "EN7516/EN7527";
	case EN7526C_PKG:
		return "EN7526C/EN7522";
	case EN751221_PKG:
		return "EN7512/EN7521";
	case MT751020_PKG:
		return "MT7510/MT7520";
	default:
		return NULL;
	}
}

static inline const char *
airoha_soc_variant_name(enum airoha_pkg pkg, u32 pkgid)
{
	switch (airoha_pkg_from_id(pkg)) {
	case AN7583_PKG:
		return airoha_pkg_id_range_name(pkgid, AN7583GT, AN7583FF);
	case AN7552_PKG:
		return airoha_pkg_id_range_name(pkgid, AN7552CT, AN7563PT);
	case EN7581_PKG:
		return airoha_pkg_id_range_name(pkgid, AN7581GT, AN7581SIT);
	case EN7523_PKG:
		return airoha_pkg_id_range_name(pkgid, EN7529DU, EN7523DTM);
	case EN7528_PKG:
		switch (pkgid) {
		case 0x0:
			return airoha_pkg_id_name(EN7528HU);
		case 0x1:
			return airoha_pkg_id_name(EN7528DU);
		case 0x2:
			return airoha_pkg_id_name(EN7561DU);
		case 0x3:
			return airoha_pkg_id_name(EN7526FH);
		case 0x4:
			return airoha_pkg_id_name(EN7561HU);
		case 0x7:
			return airoha_pkg_id_name(EN7521G);
		default:
			return NULL;
		}
	case EN7580_PKG:
		switch (pkgid) {
		case 0x0:
			return airoha_pkg_id_name(EN7580GT);
		case 0x1:
			return airoha_pkg_id_name(EN7580ST);
		case 0x2:
			return airoha_pkg_id_name(EN7580GAT);
		case 0x3:
			return airoha_pkg_id_name(EN7565);
		case 0x4:
			return airoha_pkg_id_name(EN7528GT_EN7580);
		default:
			return NULL;
		}
	/*
	 * EN7516/EN7527, EN7526C/EN7522 and EN7512/EN7521 do not
	 * have a linear SCREG_WR1 package-id encoding. Decode them from
	 * the MIPS eFuse registers with airoha_soc_variant_name_mips().
	 */
	case EN751627_PKG:
	case EN7526C_PKG:
	case EN751221_PKG:
	case MT751020_PKG:
		return NULL;
	default:
		return NULL;
	}
}

static inline const char *
airoha_soc_variant_name_efuse(enum airoha_pkg pkg, u32 efuse0, u32 efuse1,
			      u32 chip_scu_174, u32 chip_scu_1ec)
{
	u32 efuse_pkg;

	switch (airoha_pkg_from_id(pkg)) {
	case EN751627_PKG:
		efuse_pkg = airoha_efuse_pkgid_751627(efuse0);

		if (efuse_pkg == AIROHA_EFUSE_EN7516G)
			return airoha_pkg_id_name(EN7516G);

		if (efuse_pkg == AIROHA_EFUSE_EN7561G &&
		    !(chip_scu_174 & AIROHA_CHIP_SCU_751627_QFP))
			return airoha_pkg_id_name(EN7561G);

		if (efuse_pkg == AIROHA_EFUSE_EN7527H) {
			if (chip_scu_174 & AIROHA_CHIP_SCU_751627_QFP)
				return airoha_pkg_id_name(EN7527H);

			return airoha_pkg_id_name(EN7527G);
		}

		return NULL;

	case EN7526C_PKG:
		efuse_pkg = airoha_efuse_pkgid(efuse0) &
			     AIROHA_EFUSE_PKG_7526C_MASK;

		if (efuse_pkg == AIROHA_EFUSE_EN7521F) {
			if (airoha_efuse_is_ddr3(efuse0))
				return airoha_pkg_id_name(EN7521FCUD);

			return airoha_pkg_id_name(EN7521F);
		}

		if (efuse_pkg == AIROHA_EFUSE_EN7521S)
			return airoha_pkg_id_name(EN7521S);

		if (efuse_pkg == AIROHA_EFUSE_EN7526F) {
			bool ft_c;

			ft_c = efuse0 & AIROHA_EFUSE_REMARK_BIT ?
			       !!(efuse1 & BIT(10)) :
			       !!(efuse1 & BIT(9));

			if (ft_c)
				return airoha_pkg_id_name(EN7526FT_C);

			if (chip_scu_1ec & AIROHA_CHIP_SCU_7526C_FP)
				return airoha_pkg_id_name(EN7526FP);

			return airoha_pkg_id_name(EN7526F);
		}

		return NULL;

	case EN751221_PKG:
		switch (airoha_efuse_pkgid(efuse0)) {
		case AIROHA_EFUSE_EN7526F:
			return airoha_pkg_id_name(EN7526F);
		case AIROHA_EFUSE_EN7526D:
			return airoha_pkg_id_name(EN7526D);
		case AIROHA_EFUSE_EN7526G:
			return airoha_pkg_id_name(EN7526G);
		case AIROHA_EFUSE_EN7512:
			return airoha_pkg_id_name(EN7512);
		case AIROHA_EFUSE_EN7513:
			return airoha_pkg_id_name(EN7513);
		case AIROHA_EFUSE_EN7513G:
			return airoha_pkg_id_name(EN7513G);
		case AIROHA_EFUSE_EN7586:
			return airoha_pkg_id_name(EN7586);
		case AIROHA_EFUSE_EN7521F:
			return airoha_pkg_id_name(EN7521F);
		case AIROHA_EFUSE_EN7526FT:
			return airoha_pkg_id_name(EN7526FT);
		case AIROHA_EFUSE_EN7521G:
			return airoha_pkg_id_name(EN7521G);
		case AIROHA_EFUSE_EN7521S:
			return airoha_pkg_id_name(EN7521S);
		default:
			return NULL;
		}
	default:
		return NULL;
	}
}

static inline const char *
airoha_soc_variant_name_mt751020(u32 np_scu_cfg, u32 efuse0)
{
	bool enp = airoha_efuse_is_enp_mod(efuse0);
	bool ens = airoha_efuse_is_ens_mod(efuse0);

	switch (np_scu_cfg & 0x3) {
	case 0x0:
		return airoha_pkg_id_name(enp ? MT7510 : MT7511);
	case 0x2:
		if (ens)
			return airoha_pkg_id_name(MT7520S);
		return airoha_pkg_id_name(enp ? MT7520 : MT7525);
	case 0x3:
		return airoha_pkg_id_name(enp ? MT7520G : MT7525G);
	default:
		return NULL;
	}
}

static inline const char *
airoha_soc_variant_name_mips(enum airoha_pkg pkg, u32 pkgid,
			     u32 np_scu_cfg, u32 chip_scu_174,
			     u32 chip_scu_1ec, u32 efuse0, u32 efuse1)
{
	switch (airoha_pkg_from_id(pkg)) {
	case EN751627_PKG:
	case EN7526C_PKG:
	case EN751221_PKG:
		return airoha_soc_variant_name_efuse(pkg, efuse0, efuse1,
						      chip_scu_174,
						      chip_scu_1ec);
	case MT751020_PKG:
		return airoha_soc_variant_name_mt751020(np_scu_cfg, efuse0);
	default:
		return airoha_soc_variant_name(pkg, pkgid);
	}
}

static inline const char *airoha_soc_name(enum airoha_pkg pkg, u32 pkgid)
{
	const char *name;

	name = airoha_soc_variant_name(pkg, pkgid);
	if (name)
		return name;

	name = airoha_pkg_family_name(pkg);
	if (name)
		return name;

	return "unknown";
}

static inline const char *
airoha_soc_name_from_regs(u32 hir, u32 pkgid, u32 pdidr)
{
	enum airoha_pkg hir_pkg = airoha_pkg_from_id(hir);
	enum airoha_pkg pdidr_pkg = airoha_pkg_from_id(pdidr);
	const char *name;

	name = airoha_soc_variant_name(hir_pkg, pkgid);
	if (name)
		return name;

	name = airoha_soc_variant_name(pdidr_pkg, pkgid);
	if (name)
		return name;

	name = airoha_pkg_family_name(hir_pkg);
	if (name)
		return name;

	name = airoha_pkg_family_name(pdidr_pkg);
	if (name)
		return name;

	return "unknown";
}

static inline const char *
airoha_soc_name_from_mips_regs(u32 hir, u32 pkgid, u32 pdidr,
			       u32 np_scu_cfg, u32 chip_scu_174,
			       u32 chip_scu_1ec, u32 efuse0, u32 efuse1)
{
	enum airoha_pkg hir_pkg = airoha_pkg_from_id(hir);
	enum airoha_pkg pdidr_pkg = airoha_pkg_from_id(pdidr);
	const char *name;

	name = airoha_soc_variant_name_mips(hir_pkg, pkgid, np_scu_cfg,
					     chip_scu_174, chip_scu_1ec,
					     efuse0, efuse1);
	if (name)
		return name;

	name = airoha_soc_variant_name_mips(pdidr_pkg, pkgid, np_scu_cfg,
					     chip_scu_174, chip_scu_1ec,
					     efuse0, efuse1);
	if (name)
		return name;

	name = airoha_pkg_family_name(hir_pkg);
	if (name)
		return name;

	name = airoha_pkg_family_name(pdidr_pkg);
	if (name)
		return name;

	return "unknown";
}

static inline u32 airoha_pkgid_from_screg(u32 value)
{
	u32 pkgid = FIELD_GET(AIROHA_NP_SCU_PACKAGE_ID_MASK, value);

	if (value & AIROHA_NP_SCU_PACKAGE_ID_EXT)
		pkgid |= BIT(4);

	return pkgid;
}

/*
 * Direct-MMIO helper for MIPS boot code/early platform code.
 *
 * In U-Boot on MIPS the caller can pass uncached KSEG1 mappings, e.g.
 * (void __iomem *)CKSEG1ADDR(AIROHA_NP_SCU_BASE).
 */
static inline const char *
airoha_soc_name_from_mips_mem(void __iomem *np_scu, void __iomem *chip_scu,
			      void __iomem *efuse)
{
	u32 hir, pdidr, pkgid, np_scu_cfg = 0;
	u32 chip_scu_174 = 0, chip_scu_1ec = 0;
	u32 efuse0 = 0, efuse1 = 0;
	enum airoha_pkg pkg;

	hir = FIELD_GET(AIROHA_NP_SCU_HIR_MASK,
			readl(np_scu + AIROHA_NP_SCU_HIR));
	pdidr = FIELD_GET(AIROHA_NP_SCU_PDIDR_MASK,
			  readl(np_scu + AIROHA_NP_SCU_PDIDR));
	pkgid = airoha_pkgid_from_screg(readl(np_scu +
					      AIROHA_NP_SCU_SCREG_WR1));

	pkg = airoha_pkg_from_id(hir);
	if (!pkg)
		pkg = airoha_pkg_from_id(pdidr);

	switch (pkg) {
	case EN751627_PKG:
		if (!chip_scu || !efuse)
			return airoha_pkg_family_name(pkg);
		chip_scu_174 = readl(chip_scu + AIROHA_CHIP_SCU_751627_PKG);
		efuse0 = readl(efuse + AIROHA_EFUSE_VERIFY_DATA0);
		break;
	case EN7526C_PKG:
		if (!chip_scu || !efuse)
			return airoha_pkg_family_name(pkg);
		chip_scu_1ec = readl(chip_scu + AIROHA_CHIP_SCU_7526C_PKG);
		efuse0 = readl(efuse + AIROHA_EFUSE_VERIFY_DATA0);
		efuse1 = readl(efuse + AIROHA_EFUSE_VERIFY_DATA1);
		break;
	case EN751221_PKG:
		if (!efuse)
			return airoha_pkg_family_name(pkg);
		efuse0 = readl(efuse + AIROHA_EFUSE_VERIFY_DATA0);
		break;
	case MT751020_PKG:
		if (!efuse)
			return airoha_pkg_family_name(pkg);
		np_scu_cfg = readl(np_scu + AIROHA_NP_SCU_MT751020_CFG);
		efuse0 = readl(efuse + AIROHA_EFUSE_VERIFY_DATA0);
		break;
	default:
		break;
	}

	return airoha_soc_name_from_mips_regs(hir, pkgid, pdidr, np_scu_cfg,
					      chip_scu_174, chip_scu_1ec,
					      efuse0, efuse1);
}

/*
 * Package IDs are family-relative values stored by the bootloader in the
 * NP-SCU watchdog-reset scratch register 1. A value of zero is valid.
 */
static inline u32 get_pkgid(void)
{
	struct regmap *np_scu = airoha_get_scu_regmap();
	u32 value;
	int err;

	err = regmap_read(np_scu, AIROHA_NP_SCU_SCREG_WR1, &value);
	if (err)
		return END_PACKAGE_ID;

	return airoha_pkgid_from_screg(value);
}

static inline u32 get_pkgid_mem(void __iomem *np_scu)
{
	return airoha_pkgid_from_screg(readl(np_scu +
					     AIROHA_NP_SCU_SCREG_WR1));
}

/* HIR identifies the SoC family, for example EN7523_PKG (0x0c). */
static inline enum airoha_pkg get_pkg(void)
{
	struct regmap *np_scu = airoha_get_scu_regmap();
	u32 value;
	int err;

	err = regmap_read(np_scu, AIROHA_NP_SCU_HIR, &value);
	if (err)
		return 0;

	return FIELD_GET(AIROHA_NP_SCU_HIR_MASK, value);
}

static inline u32 get_pdidr(void)
{
	struct regmap *np_scu = airoha_get_scu_regmap();
	u32 value;
	int err;

	err = regmap_read(np_scu, AIROHA_NP_SCU_PDIDR, &value);
	if (err)
		return 0;

	return FIELD_GET(AIROHA_NP_SCU_PDIDR_MASK, value);
}

static inline enum airoha_pkg get_pkg_mem(void __iomem *np_scu)
{
	return FIELD_GET(AIROHA_NP_SCU_HIR_MASK,
			 readl(np_scu + AIROHA_NP_SCU_HIR));
}

static inline u32 get_pdidr_mem(void __iomem *np_scu)
{
	return FIELD_GET(AIROHA_NP_SCU_PDIDR_MASK,
			 readl(np_scu + AIROHA_NP_SCU_PDIDR));
}

#endif
