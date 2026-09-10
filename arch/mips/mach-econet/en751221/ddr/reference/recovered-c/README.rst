EN751221 recovered DDR calibration C
====================================

These six C translation units were recovered by Merbanan from the EN7512 DDR
calibration objects.  With the original BSP headers and the bundled Buildroot
GCC 4.9.3/binutils 2.24 toolchain they reproduce all calibration objects byte
for byte.  Linking the rebuilt objects reproduces the complete vendor
``spram.img`` as well.

The recovered sources are retained here as review/reference material while the
normal U-Boot DDR build continues to use ``../../reconstructed/*.S``.  The C
files still include the original BSP ``dramc.h`` and ``asm/tc3162.h`` APIs; do
not make them build inputs until those dependencies have been replaced by a
self-contained U-Boot register interface.

Reference artifacts from the recovery work::

  spram.img size:   20336 bytes
  spram.img SHA256: 792068a4573b20cb8c39cb27df06c45683c11034e6fd07da24ddf7084828faa5
  boot.out size:    35630 bytes
  boot.out SHA256:  ec4a5bb71439d0627c48f69259bcb70feb8e482dccaaeb1048e9375b5c3262bf

The recovered calibration C files are:

* ``main.c``
* ``dramc.c``
* ``dramc_dq_dqs_cal.c``
* ``dramc_dle_cal.c``
* ``dramc_dqs_gw_cal.c``
* ``en7512_dramc_init.c``
