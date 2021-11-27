" Vim syntax file
" Language:     C waveform

" wf/waveform.h
syn keyword waveformType     Waveform
syn keyword waveformType     WfSampleRegion
syn keyword waveformType     WfSampleRegionf
syn keyword waveformConstant WF_PEAK_RATIO
syn keyword waveformMacro    TYPE_WAVEFORM
syn keyword waveformMacro    waveform_unref0
syn keyword waveformFunction waveform_load_new
syn keyword waveformFunction waveform_set_peak_loader
syn keyword waveformFunction waveform_get_n_frames
syn keyword waveformFunction waveform_get_n_channels
syn keyword waveformFunction waveform_get_type
syn keyword waveformFunction waveform_new
syn keyword waveformFunction waveform_construct
syn keyword waveformFunction waveform_load
syn keyword waveformFunction waveform_load_sync
syn keyword waveformFunction waveform_set_file
syn keyword waveformFunction waveform_load_peak
syn keyword waveformFunction waveform_peak_is_loaded
syn keyword waveformFunction waveform_load_rms_file
syn keyword waveformFunction waveform_load_audio
syn keyword waveformFunction waveform_load_audio_sync
syn keyword waveformFunction waveform_find_max_audio_level

" ui/actor.h
syn keyword waveformType     WaveformActor
syn keyword waveformType     WaveformActorClass
syn keyword waveformMacro    WF_ACTOR_PX_PER_FRAME
syn keyword waveformFunction wf_actor_get_class
syn keyword waveformFunction wf_actor_set_waveform
syn keyword waveformFunction wf_actor_set_waveform_sync
syn keyword waveformFunction wf_actor_set_region
syn keyword waveformFunction wf_actor_set_rect
syn keyword waveformFunction wf_actor_set_colour
syn keyword waveformFunction wf_actor_set_full
syn keyword waveformFunction wf_actor_fade_out
syn keyword waveformFunction wf_actor_fade_in
syn keyword waveformFunction wf_actor_set_vzoom
syn keyword waveformFunction wf_actor_get_z
syn keyword waveformFunction wf_actor_set_z
syn keyword waveformFunction wf_actor_get_viewport
syn keyword waveformFunction wf_actor_frame_to_x
syn keyword waveformFunction wf_actor_clear

" ui/context.h
syn keyword waveformType     WaveformContext
syn keyword waveformFunction wf_context_new
syn keyword waveformFunction wf_context_new_sdl
syn keyword waveformFunction wf_context_free
syn keyword waveformFunction wf_context_add_new_actor
syn keyword waveformFunction wf_context_set_viewport
syn keyword waveformFunction wf_context_set_rotation
syn keyword waveformFunction wf_context_get_zoom
syn keyword waveformFunction wf_context_set_zoom
syn keyword waveformFunction wf_context_set_scale
syn keyword waveformFunction wf_context_set_start
syn keyword waveformFunction wf_context_set_gain
syn keyword waveformFunction wf_context_queue_redraw
syn keyword waveformFunction wf_canvas_load_texture_from_alphabuf
syn keyword waveformFunction wf_context_frame_to_x

" ui/view_plus.h
syn keyword waveformFunction waveform_view_plus_get_type
syn keyword waveformFunction waveform_view_plus_set_gl
syn keyword waveformFunction waveform_view_plus_new
syn keyword waveformFunction waveform_view_plus_load_file
syn keyword waveformFunction waveform_view_plus_set_waveform
syn keyword waveformFunction waveform_view_plus_get_zoom
syn keyword waveformFunction waveform_view_plus_set_zoom
syn keyword waveformFunction waveform_view_plus_set_start
syn keyword waveformFunction waveform_view_plus_set_region
syn keyword waveformFunction waveform_view_plus_set_colour
syn keyword waveformFunction waveform_view_plus_set_show_rms
syn keyword waveformFunction waveform_view_plus_add_layer
syn keyword waveformFunction waveform_view_plus_get_layer
syn keyword waveformFunction waveform_view_plus_remove_layer
syn keyword waveformFunction waveform_view_plus_get_context
syn keyword waveformFunction waveform_view_plus_get_actor

syn keyword waveformDebug    PF

" Default highlighting
if version >= 508 || !exists("did_waveform_syntax_inits")
  if version < 508
    let did_waveform_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif
  HiLink waveformType        Type
  HiLink waveformFunction    Function
  HiLink waveformMacro       Macro
  HiLink waveformConstant    Constant
  HiLink waveformBoolean     Boolean
  HiLink waveformDebug       Debug
  delcommand HiLink
endif


