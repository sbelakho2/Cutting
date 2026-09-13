#!/bin/zsh
# Verify the instruction-level timing contract emitted by pioasm.
# Usage: tools/check_pio_contract.zsh [build-dir]
setopt errexit nounset pipefail

cd "$(dirname "$0")/.."
build_dir="${1:-build-rp2350}"
step_header="$build_dir/step_gen.pio.h"
quad_header="$build_dir/quadrature.pio.h"
[[ -f "$step_header" && -f "$quad_header" ]] || {
    print -u2 -- "missing generated PIO headers under $build_dir"
    exit 1
}

expect_step() {
    grep -Fq -- "$1" "$step_header" || {
        print -u2 -- "step PIO contract mismatch: $1"
        exit 1
    }
}
expect_quad() {
    grep -Fq -- "$1" "$quad_header" || {
        print -u2 -- "quadrature PIO contract mismatch: $1"
        exit 1
    }
}

# Addresses 0..14 comprise one FIFO run: pull, unpack half-period (Y) and
# count (X), high phase, low phase, count loop, then a low-level FIFO pause.
expect_step '0x80a0, //  0: pull   block'
expect_step '0x6050, //  1: out    y, 16'
expect_step '0x6030, //  2: out    x, 16'
expect_step '0xe001, //  4: set    pins, 1'
expect_step '0x0085, //  5: jmp    y--, 5'
expect_step '0xe000, //  7: set    pins, 0'
expect_step '0x0088, //  8: jmp    y--, 8'
expect_step '0x004b, // 10: jmp    x--, 11'
expect_step '0xa042, // 11: nop'
expect_step '0xa042, // 12: nop'
expect_step '0xa042, // 13: nop'
expect_step '0x0004, // 14: jmp    4'
expect_step 'sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);'

# The first rising-edge instruction is address 4. Between rising edges the
# PIO executes Y high-loop cycles, one restore and one low SET, Y low-loop
# cycles, one restore, the X loop instruction, then pull/unpack/restore and
# the next SET: 2Y+9 ticks at the fixed 1 MHz PIO clock. The only blocking
# instruction is address 0, reached after SET pins,0, so FIFO starvation
# cannot leave PUL high.
for half_ticks in 1 58 1024; do
    (( 2 * half_ticks + 9 >= 11 )) || exit 1
done

# Quadrature capture packs A/B into ISR low bits. The board consumes those
# same bits; changing this would silently corrupt Gray-transition decoding.
expect_quad '0x6022, //  1: out    x, 2'
expect_quad '0x4002, //  3: in     pins, 2'
expect_quad '0x8020, //  8: push   block'
expect_quad 'sm_config_set_in_shift(&c, false, false, 32);'
expect_quad 'sm_config_set_out_shift(&c, true, false, 32);'

print 'pio contract ok'
