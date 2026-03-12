#include "font_primary.h"

#include "../internal/patch.h"
#include "../internal/tickable.h"
#include "../mkb/mkb.h"
#include "../utils/ppcutil.h"

namespace font_primary {

int primary_color = 0xffff00;

// Patch slot holding primary color for font-color
TICKABLE_DEFINITION((
        .name = "font-primary",
        .description = "Font Primary",
        .active_value = primary_color,
        .lower_bound = 0x000000,
        .upper_bound = 0xffffff,
        .init_main_loop = init,))

int get_color() {
    return primary_color;
}

void init() {
    primary_color = *active_tickable_ptr->active_value;
}

}// namespace font_primary