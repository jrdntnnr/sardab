/* SPDX-License-Identifier: GPL-3.0-only */
/* Drive physical menu gestures and compare the displayed value to a fresh draw.
 * In particular, TOUCH settings must change visibly without leaving their row. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static uint16_t before[240 * 240], after[240 * 240];

int main(void)
{
    int bad = 0;
    for (unsigned large = 0; large < 2; large++)
        for (unsigned row = 0; row < MI_COUNT; row++) {
            if (!menu_valued(row)) continue;
            for (unsigned knob = EN_K1; knob <= EN_K4; knob++) {
                ui_power_on();
                ui_prefs = PREF_ANIM_OFF | (large ? PREF_LARGE : 0);
                hold(B_HOME); /* open and release HOME before editing */
                ui.menu_sel = row;
                ui.force = 1;
                frame();
                memcpy(before, host_screen, sizeof before);
                unsigned old = menu_get(row);
                int direction = menu_step(row, 1) != old ? 1 : -1;
                turn(knob, direction);
                unsigned changed = menu_get(row);
                memcpy(after, host_screen, sizeof after);
                ui.force = 1;
                ui_draw();
                if (changed == old || !memcmp(before, after, sizeof after) ||
                    memcmp(after, host_screen, sizeof after)) {
                    printf("FAIL menu row %s knob %u size %u: value %u->%u, visible %d, fresh %d\n",
                           MI_NAME[row], knob - EN_K1 + 1, large, old, changed,
                           memcmp(before, after, sizeof after) != 0,
                           memcmp(after, host_screen, sizeof after) == 0);
                    bad++;
                }
            }
        }
    ui_power_on();
    hold(B_HOME);
    turn(EN_ALGO, -1); /* DISPLAY wraps to TOUCH */
    bad += check("TOUCH: first row is VELOCITY FIXED", ui.menu_sel == MI_VMODE && menu_get(MI_VMODE) == 0);
    turn(EN_K1, 1);
    bad += check("TOUCH: knob enables SENSE", menu_get(MI_VMODE) == 1 && (kv_config & 1));
    press(B_OCTDN);
    bad += check("TOUCH: OCT- disables SENSE", menu_get(MI_VMODE) == 0 && !(kv_config & 1));
    memcpy(before, host_screen, sizeof before);
    press(B_OCTUP);
    bad += check("TOUCH: OCT+ visibly enables SENSE", menu_get(MI_VMODE) == 1 && memcmp(before, host_screen, sizeof before));
    press(B_HOME);
    persist_t saved = {0};
    settings_export(&saved);
    ui_power_on();
    bad += check("TOUCH: saved SENSE survives settings reload", settings_import(&saved, sizeof saved) && (kv_config & 1));
    if (!bad) puts("menu values: every editable row responds and redraws with all four knobs, both text sizes PASS");
    return bad != 0;
}
