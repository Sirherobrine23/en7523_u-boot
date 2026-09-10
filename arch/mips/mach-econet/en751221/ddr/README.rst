EN751221 DDR calibration source
===============================

The files under ``reconstructed/`` preserve the code, symbols and relocations
from the GPL DDR calibration objects as assembly source.  No prebuilt DDR
binary or object is required by the U-Boot build.

The original stage executes from FE SRAM at ``0x9fa32800``.  The build helper
uses the original object order (head, setup, start_spram, UART/timer/string,
calibration and finally SPRAM helpers), matching the linked vendor layout.

Merbanan independently recovered the six calibration translation units as C.
With the original BSP headers and GCC 4.9.3 they reproduce all corresponding
objects byte for byte and reproduce the 20336-byte vendor ``spram.img``.  Those
sources are preserved under ``reference/recovered-c/`` for review and for the
next conversion step; they are intentionally not build inputs yet because they
still depend on the original BSP ``dramc.h`` and ``asm/tc3162.h`` interfaces.

Use ``tools/build-econet-ddr.sh en751221`` with a big-endian GNU MIPS
toolchain to produce the temporary build artifact consumed by Binman.
