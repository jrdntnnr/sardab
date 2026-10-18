/* SPDX-License-Identifier: GPL-3.0-only */
/* Real scan/ADC code against memory-backed GPIO and converter registers. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../firmware/hal/fm1_time.h"
#include "../firmware/hal/fm1_gpio.h"
static uint32_t regs[16][64], clock_, adc_con, adc_res, wla;
#undef FM1_PR
#define FM1_PR(p,r) regs[(p)&15u][((r)/4u)&63u]
static uint32_t ticks(void) { return clock_ += 24; }
#define fm1_ticks ticks
#define FM1_ADC_CON adc_con
#define FM1_ADC_RES adc_res
#define FM1_WLA_CON0 wla
#define FM1_KEY_VELOCITY 1
#include "../firmware/hal/fm1_input.h"
static void finish(uint32_t v)
{
    adc_res=v; adc_con|=128u; fm1_kv_finish(); assert(!fm1_kv.pending && !adc_con);
}
int main(void)
{
    fm1_input_init(); fm1_adc_init(); fm1_adc_owned=1;
    /* FIXED: scan owns the ADC but neither row drivers nor PA10 change. */
    uint32_t dir=FM1_PR(FM1_PA,FM1_DIR), hd=FM1_PR(FM1_PA,FM1_HD0);
    kv_config=0; fm1_kv_start(0); assert(fm1_kv.channel==FM1_ADC_MASTER && !fm1_kv.row);
    uint32_t con=adc_con; assert(fm1_adc_read(FM1_ADC_MASTER)==-1 && adc_con==con);
    finish(700); assert(fm1_adc_read(FM1_ADC_MASTER)==700 && !adc_con);
    fm1_kv_start(1); finish(620); assert(fm1_adc_read(FM1_ADC_BATT)==620);
    assert(dir==FM1_PR(FM1_PA,FM1_DIR) && hd==FM1_PR(FM1_PA,FM1_HD0));
    /* Direct key: weak driver restored before the digital scan sees the row. */
    kv_config=33; fm1_kv.phase=0; kv_contact(9,1);
    fm1_kv_start(0); /* col0 advances the phase; choose a nonzero PA7 column next */
    finish(700); fm1_kv.phase=0; kv_contact(10,1); fm1_kv_start(1);
    assert(fm1_kv.row==3 && fm1_kv.driven && !(FM1_PR(FM1_PA,FM1_DIR)&(1u<<7)));
    finish(650); assert(FM1_PR(FM1_PA,FM1_DIR)&(1u<<7)); assert(!(FM1_PR(FM1_PA,FM1_HD0)&(1u<<7)));
    fm1_kv_start(1); finish(640); assert(kv_capture[10].landing==(KV_READY|640));
    /* Timeout: pins restored, no fabricated capture, next conversion still works. */
    kv_contact(10,0); kv_contact(10,1); fm1_kv_start(1); adc_con&=~128u;
    fm1_kv_finish(); assert(fm1_kv.timeouts==1 && !kv_capture[10].landing);
    assert(FM1_PR(FM1_PA,FM1_DIR)&(1u<<7));
    /* Top probe: PA10 becomes an input only during conversion; digital input
     * buffers are never disabled, so OCT +/- cannot become phantom presses. */
    uint32_t die=FM1_PR(FM1_PA,FM1_DIE); fm1_kv.phase=2; fm1_kv_start(1);
    assert(fm1_kv.probe && fm1_kv.row==2 && (FM1_PR(FM1_PA,FM1_DIR)&(1u<<10)));
    finish(770); assert(!(FM1_PR(FM1_PA,FM1_DIR)&(1u<<10)) && (FM1_PR(FM1_PA,FM1_DIR)&(1u<<6)));
    assert(die==FM1_PR(FM1_PA,FM1_DIE));
    /* Switching OFF with a conversion in flight still restores its pins. */
    fm1_kv_start(1); kv_config=0; finish(800); fm1_kv_start(0);
    assert(!fm1_kv.row && !kv_hw_on); finish(700);
    puts("velocity HAL: ADC ownership/cache, direct drive, probe, timeout and mode-switch restoration pass");
}
