EN751627 DDR training payload
=============================

``en751627_ddr.bin`` is the LOAD image from the GPL-released EN7516/EN7527
DDR calibration ELF used by the VX830v SDK::

  unopen_img/UNION_EN7516_7915D_demo/ddr_cal_en7512/output/boot.out

The source ELF is MIPS32r2 big-endian, loads at ``0x9fa30000`` and enters at
``0x9fa30280``.  Its LOAD segment has a file size of ``0xd050`` bytes.  The
stage returns through the address saved in NP-SCU ``SCREG_WR0`` (0xbfb00280),
which is the handoff used by the EcoNet TPL wrapper.

SHA256 of the extracted LOAD bytes::

  a8ca71b613f1857741dde066ebbc5de16baa90ec5203e27d07a5d1705fcf123e

This is kept as a GPL build artefact until the corresponding EN751627 DDR
objects are reconstructed into source, like the EN7528 and EN7580 paths.
