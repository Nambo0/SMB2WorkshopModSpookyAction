#include "font_color.h"

#include "../internal/patch.h"
#include "../internal/tickable.h"
#include "../mkb/mkb.h"
#include "../utils/ppcutil.h"
#include "font_primary.h"
#include "font_secondary.h"

namespace font_color {

TICKABLE_DEFINITION((
        .name = "custom-font-color",
        .description = "Font Color",
        .init_main_loop = init,
        .init_main_game = init_main_game,))

// void textdraw_set_mul_color(uint param_1);
static patch::Tramp<decltype(&mkb::textdraw_set_mul_color)> s_textdraw_set_mul_color_tramp;

void replace_lis_color(int reg, u32 loc, u32 color) {
    patch::write_word(reinterpret_cast<void*>(loc), PPC_INSTR_LIS(reg, (color >> 16) & 0xff));// RR
    patch::write_word(reinterpret_cast<void*>(loc + 4), PPC_INSTR_ORI(reg, (color & 0xffff)));// GGBB
}

void replace_3pt_color(u32 loc, u32 color) {
    patch::write_word(reinterpret_cast<void*>(loc), 0x38000000 + ((color >> 16) & 0xff));   // RR
    patch::write_word(reinterpret_cast<void*>(loc + 8), 0x38000000 + ((color >> 8) & 0xff));// GG
    patch::write_word(reinterpret_cast<void*>(loc + 16), 0x38000000 + (color & 0xff));      // BB
}

u32 form_color(u8 red, u8 blue, u8 green) {
    return (red << 16) + (blue << 8) + green;
}

u32 convert_color(u32 color) {
    u8 red = (color >> 16) & 0xff;
    u8 green = (color >> 8) & 0xff;
    u8 blue = color & 0xff;
    u32 new_color = color;
    if (blue == 0 && red > 0 && green > 0 && red >= green) {// Orange & yellow (b=0, r>=g>0)
        u8 red_1 = (font_primary::get_color() >> 16) & 0xff;
        u8 green_1 = (font_primary::get_color() >> 8) & 0xff;
        u8 blue_1 = font_primary::get_color() & 0xff;
        u8 red_2 = (font_secondary::get_color() >> 16) & 0xff;
        u8 green_2 = (font_secondary::get_color() >> 8) & 0xff;
        u8 blue_2 = font_secondary::get_color() & 0xff;
        float percent_primary = (1 - ((float)(red - green) / (red/2)));
        new_color = form_color((u8)(((float)red/0xff) * ((red_1 * percent_primary) + (red_2 * (1-percent_primary)))),
                               (u8)(((float)red/0xff) * ((green_1 * percent_primary) + (green_2 * (1-percent_primary)))),
                               (u8)(((float)red/0xff) * ((blue_1 * percent_primary) + (blue_2 * (1-percent_primary)))));
    }
    return new_color;
}

void init() {
    patch::hook_function(s_textdraw_set_mul_color_tramp, mkb::textdraw_set_mul_color, [](u32 color) {
        s_textdraw_set_mul_color_tramp.dest(convert_color(color));
    });
}

void init_main_game() {
    replace_3pt_color(0x80338c84, font_primary::get_color());// Stage number RGB color
    replace_3pt_color(0x80338e74, font_primary::get_color());// Stage name RGB color
    replace_3pt_color(0x80338A84, font_primary::get_color());// EX color
    replace_3pt_color(0x8032caa0, convert_color(0xffc000));// READY text color
    replace_3pt_color(0x8032d944, font_secondary::get_color());// GO text color
    replace_3pt_color(0x8032bbb8, font_primary::get_color());// STAGE/WORLD spin in color
    replace_3pt_color(0x8032E91C, font_secondary::get_color());// FALLOUT color
}

}// namespace font_color