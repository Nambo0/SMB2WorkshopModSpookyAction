#include "story_continuous_music.h"

#include "internal/assembly.h"
#include "internal/patch.h"
#include "internal/tickable.h"

namespace story_continuous_music {

TICKABLE_DEFINITION((
        .name = "story-mode-music-fix",
        .description = "Continuous story mode music",
        .init_main_loop = init_main_loop, ))

//s32 SoftStreamStart(u32 g_looping_state, BgmTrack  g_bgm_id, u32 param_3);
patch::Tramp<decltype(&mkb::SoftStreamStart)> s_SoftStreamStart_tramp;
// void g_something_with_music_fading(void);
patch::Tramp<decltype(&mkb::g_crossfade_music)> s_g_crossfade_music_tramp;

// void g_something_with_stopping_music_or_sfx(BgmTrack  param_1);
patch::Tramp<decltype(&mkb::g_something_with_stopping_music_or_sfx)> s_g_something_with_stopping_music_or_sfx_tramp;
// undefined4 play_track_and_fade_out_other_tracks(undefined4 param_1, undefined4 param_2, byte volume);
patch::Tramp<decltype(&mkb::play_track_and_fade_out_other_tracks)> s_play_track_and_fade_out_other_tracks_tramp;

mkb::BgmTrack looping_track = 0;

s32 SoftStreamStart(u32 g_looping_state, mkb::BgmTrack  g_bgm_id, u32 param_3) {
    if(g_bgm_id == mkb::BGM_YUENCHI) { // if par2 playing
        looping_track = g_bgm_id;       // loop par2
    }
    if(looping_track == 0) { // No looping theme
        return s_SoftStreamStart_tramp.dest(g_looping_state, g_bgm_id, param_3);
    }
    else {                 // Loop currently set theme
        return s_SoftStreamStart_tramp.dest(g_looping_state, looping_track, param_3);
    }
}


void g_something_with_stopping_music_or_sfx(mkb::BgmTrack param_1) {
    //if(looping_track == 0) s_g_something_with_stopping_music_or_sfx_tramp.dest(param_1); // this one was off
    s_g_something_with_stopping_music_or_sfx_tramp.dest(param_1);
}

void g_crossfade_music() {
    if(looping_track == 0) s_g_crossfade_music_tramp.dest(); // no loop, run normally
    // Nothing happens if looping (dont stop the music!)
}

mkb::undefined4 play_track_and_fade_out_other_tracks(mkb::undefined4 param_1, mkb::undefined4 param_2, mkb::byte volume) {
    if(looping_track == 0) return s_play_track_and_fade_out_other_tracks_tramp.dest(param_1, param_2, volume);
    else return 0;
}


// Hooks right before the call to SoftStreamStart, then nops the
// "Stage Select" music fadeout
// Modifies the 1st parameter to SoftStreamStart following the goal sequence
// affecting whether or not the music restarts/changes. Only modifies this when
// the submode indicates we're currently on a stage, or if we're on the 'Retry' screen.
void init_main_loop() {
    
    patch::write_branch_bl(reinterpret_cast<void*>(0x802a5c34),
                           reinterpret_cast<void*>(main::story_mode_music_hook));
    patch::write_nop(reinterpret_cast<void*>(0x80273aa0));

    patch::hook_function(s_SoftStreamStart_tramp, mkb::SoftStreamStart,
                         SoftStreamStart);
    patch::hook_function(s_g_crossfade_music_tramp, mkb::g_crossfade_music,
                         g_crossfade_music);
    patch::hook_function(s_g_something_with_stopping_music_or_sfx_tramp, mkb::g_something_with_stopping_music_or_sfx,
                         g_something_with_stopping_music_or_sfx);
    patch::hook_function(s_play_track_and_fade_out_other_tracks_tramp, mkb::play_track_and_fade_out_other_tracks,
                         play_track_and_fade_out_other_tracks);
    
}

}// namespace story_continuous_music
