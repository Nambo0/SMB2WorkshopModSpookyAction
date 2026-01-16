#include "challenge_timer.h"

#include "../internal/patch.h"
#include "../internal/tickable.h"
#include "../utils/vecutil.h"
#include "../mkb/mkb.h"

namespace challenge_timer {

// Patch is enabled by default
TICKABLE_DEFINITION((
        .name = "challenge-timer",
        .description = "Challenge timer",
        .enabled = true,
        .init_main_loop = init_main_loop,
        .init_main_game = init_main_game,
        .tick = tick, ))

// On spin-in tramp
// static patch::Tramp<decltype(&mkb::g_reset_ball)> s_g_reset_ball_tramp;
static patch::Tramp<decltype(&mkb::create_player_num_and_ready_sprites)> s_create_player_num_and_ready_sprites_tramp;
// On load-in tramp
static patch::Tramp<decltype(&mkb::load_stagedef)> s_load_stagedef_tramp;
// On goal tramp
static patch::Tramp<decltype(&mkb::create_goal_and_jump_to_stage_sprites)> s_create_goal_and_jump_to_stage_sprites_tramp;
// On fallout tramp
// static patch::Tramp<decltype(&mkb::create_fallout_or_bonus_finish_sprite)> s_create_fallout_or_bonus_finish_sprite_tramp;

// Extra init tramp
// void smd_game_extra_init(void);
// static patch::Tramp<decltype(&mkb::create_go_to_extra_or_master_stages_sprites)> s_create_go_to_extra_or_master_stages_sprites_tramp;


char timer_text_buffer[32];

u16 timer_frames = 0;
u16 display_timer = 0;

bool is_first_load = true;

void tick() {
    // bool paused_now = *reinterpret_cast<u32*>(0x805BC474) & 8;
    if (mkb::main_mode == mkb::MD_GAME && mkb::sub_mode == mkb::SMD_GAME_PLAY_MAIN && mkb::main_game_mode == mkb::CHALLENGE_MODE) {
        timer_frames++;
        is_first_load = false;
    }

    // Update display
    if (display_timer > 0) display_timer -= 1;                     // Timer counting down

    // Debug banana display
    // mkb::balls[mkb::curr_player_idx].banana_count = timer_frames / 6;

    // Disable warp goal stage skip
    // REQUIRES SPECIFIC ADJACENT STAGE IDS (91-100 etc)
    /* THIS CODE DOESNT WORK
    mkb::mode_info.g_some_stage_jump_distance = 1;
    if (mkb::mode_info.entered_goal_type == 1 || mkb::mode_info.entered_goal_type == 2) {
        mkb::mode_info.cm_stage_id = mkb::current_stage_id + 1;
        if(mkb::current_stage_id  % 10 != 0) {
            mkb::mode_info.cm_course_stage_num = mkb::current_stage_id % 10 + 1;
        }
    } */
}

static void set_sprite_timer_text_name(mkb::Sprite* sprite) {
    mkb::sprintf(timer_text_buffer, "In-Game Time: %d:%02d.%02d", timer_frames / 3600, (timer_frames / 60) % 60, timer_frames % 60);
    mkb::strcpy(sprite->text, timer_text_buffer);
}

void timer_text_sprite_tick(u8* status, mkb::Sprite* sprite) {
    // New timer text being displayed
    if (display_timer == 300) {
        set_sprite_timer_text_name(sprite);
    }
    // Handle sprite alpha
    switch (display_timer) {
        case 0: {
            sprite->alpha = 0;
            break;
        }
        case 1 ... 20: {
            sprite->alpha = display_timer * 0.05;
            break;
        }
        case 21 ... 280: {
            sprite->alpha = 1;
            break;
        }
        case 281 ... 300: {
            sprite->alpha = (300 - display_timer) * 0.05;
            break;
        }
    }
}

void create_timer_text_sprite() {
    mkb::Sprite* sprite = mkb::create_sprite();
    if (sprite != (mkb::Sprite*) 0x0) {
        sprite->pos.x = 60.0 + 20;
        sprite->pos.y = 420.0 - 40;
        sprite->font = mkb::FONT_ASC_24x24;
        sprite->alignment = mkb::ALIGN_CENTER_RIGHT;
        sprite->mult_color.red = 0xff;
        sprite->mult_color.green = 0x99;
        sprite->mult_color.blue = 0x00;
        sprite->font = mkb::FONT_ASC_24x24;
        sprite->fpara1 = 1.0;
        sprite->fpara2 = 0.0;
        sprite->alpha = 0.0;
        sprite->width = 0.6;
        sprite->height = 0.6;
        sprite->field21_0x20 = 1.0;
        sprite->field22_0x24 = 1.0;
        sprite->g_counter = 300;
        sprite->g_flags1 = sprite->g_flags1 | 0xa1000000;
        sprite->widescreen_translation_x = 0x60;
        sprite->tick_func = timer_text_sprite_tick;
        set_sprite_timer_text_name(sprite);
    }
    mkb::Sprite* sprite_shadow = mkb::create_sprite();
    if (sprite_shadow != nullptr) {
        sprite_shadow->pos.x = 60.0 + 20 + 2;
        sprite_shadow->pos.y = 420.0 - 40 + 2;
        sprite_shadow->font = mkb::FONT_ASC_24x24;
        sprite_shadow->alignment = mkb::ALIGN_CENTER_RIGHT;
        sprite_shadow->depth = 0.11;
        sprite_shadow->fpara1 = 0.45;
        sprite_shadow->fpara2 = 0.0;
        sprite_shadow->alpha = 0.0;
        (sprite_shadow->mult_color).red = 0x00;
        (sprite_shadow->mult_color).green = 0x00;
        (sprite_shadow->mult_color).blue = 0x00;
        sprite_shadow->width = 0.6;
        sprite_shadow->height = 0.6;
        sprite_shadow->field21_0x20 = 0.45;
        sprite_shadow->field22_0x24 = 0;
        sprite_shadow->g_counter = 300;
        sprite_shadow->g_flags1 = sprite->g_flags1 | 0xa1000000;
        sprite_shadow->widescreen_translation_x = 0x60;
        sprite_shadow->tick_func = timer_text_sprite_tick;
        set_sprite_timer_text_name(sprite);
    }
    sprite = mkb::create_sprite();
    if (sprite != (mkb::Sprite*) 0x0) {
        sprite->type = mkb::SPRT_BMP;
        sprite->pos.x = 60.0;
        sprite->pos.y = 420.0 - 40;
        sprite->alignment = mkb::ALIGN_CENTER;
        sprite->bmp = 0x59; // used to be 5e
        sprite->alpha = 0.0;
        sprite->g_counter = 300;
        sprite->g_flags1 = 0x1000000;
        sprite->width = 0.25;
        sprite->height = 0.25;
        sprite->widescreen_translation_x = 0x60;
        sprite->tick_func = timer_text_sprite_tick;
        mkb::strcpy(sprite->text, "timer text icon");
    }
    return;
}


void on_spin_in() {
    if(!is_first_load) timer_frames += 2*60;
    if(mkb::cm_player_progress[mkb::curr_player_idx].curr_stage.stage_course_num == 1) {
        timer_frames = 0;
    }
}

void on_stage_load(u32 stage_id) {
    if (mkb::cm_player_progress[mkb::curr_player_idx].curr_stage.stage_course_num == 0) {
        timer_frames = 0; // OLD: 4.0 seconds added for first stage spin-in
    }
    /* else {
        timer_frames += 339; // 7.65 - 2 seconds added per stage transition after the first
        // 7.65s = 459 (no retry)
    } LONGER TIMER*/
    is_first_load = true;
}

void on_goal() {
    // Display in-game timer
    if (mkb::cm_player_progress[mkb::curr_player_idx].curr_stage.stage_course_num == 10 && mkb::main_game_mode == mkb::CHALLENGE_MODE) {
        create_timer_text_sprite();
        mkb::call_SoundReqID_arg_0(0x11d);// SMB1 best score sound
        display_timer = 300;
        mkb::cm_player_progress[mkb::curr_player_idx].curr_stage.stage_course_num = 1;
        
        // Jam postgoal progression timer, so they get stuck on the postgoal anim
        // This is done because there's a really crappy console-only crash on difficulty end cutscene
        patch::write_nop(reinterpret_cast<void*>(0x808f572c));
    }
}

/*void on_extra_init() {
    mkb::main_mode_request = mkb::MD_SEL;
    mkb::sub_mode_request = mkb::SMD_SEL_NGC_REINIT;
}*/

/*void on_fallout() {
    timer_frames += 70;
}*/

void init_main_game() {
}

void init_main_loop() {
    // Hook spin in & load functions

    patch::hook_function(s_create_player_num_and_ready_sprites_tramp, mkb::create_player_num_and_ready_sprites, [](s32 param_1) {
        s_create_player_num_and_ready_sprites_tramp.dest(param_1);
        on_spin_in();
    });
    patch::hook_function(s_load_stagedef_tramp, mkb::load_stagedef, [](u32 stage_id) {
        s_load_stagedef_tramp.dest(stage_id);
        on_stage_load(stage_id);
    });
    patch::hook_function(s_create_goal_and_jump_to_stage_sprites_tramp, mkb::create_goal_and_jump_to_stage_sprites, [](s32 g_num_frames) {
        s_create_goal_and_jump_to_stage_sprites_tramp.dest(g_num_frames);
        on_goal();
    });
    // NOT USED
    /*patch::hook_function(s_g_reset_ball_tramp, mkb::g_reset_ball, [](mkb::Ball* in_ball) {
        s_g_reset_ball_tramp.dest(in_ball);
        on_spin_in();
    });*/
    /*patch::hook_function(s_create_fallout_or_bonus_finish_sprite_tramp, mkb::create_fallout_or_bonus_finish_sprite, [](s32 param_1) {
        on_fallout();
        s_create_fallout_or_bonus_finish_sprite_tramp.dest(param_1);
    });*/

    /*patch::hook_function(s_create_go_to_extra_or_master_stages_sprites_tramp, mkb::create_go_to_extra_or_master_stages_sprites, []() {
        s_create_go_to_extra_or_master_stages_sprites_tramp.dest();
        on_extra_init();
    });*/
}

}// namespace challenge_timer