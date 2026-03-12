#include "font_secondary.h"

#include "../internal/patch.h"
#include "../internal/tickable.h"
#include "../mkb/mkb.h"
#include "../utils/ppcutil.h"

namespace font_secondary {

int secondary_color = 0xff8000;

// Patch slot holding secondary color for font-color
TICKABLE_DEFINITION((
        .name = "font-secondary",
        .description = "Font Secondary",
        .active_value = secondary_color,
        .lower_bound = 0x000000,
        .upper_bound = 0xffffff,
        .init_main_loop = init,))

int get_color() {
    return secondary_color;
}

void init() {
    secondary_color = *active_tickable_ptr->active_value;
}

}// namespace font_secondary