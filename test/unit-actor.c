/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2025-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | unit tests for WaveformActor                                         |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"

#include "ui/actor.c"

#include "test/common.h"
#include "test/unit-actor.h"

extern AGl _agl;

#define WAV "mono_0:10.wav"

typedef struct {
	Renderer renderer;
	int      load_blocks_calls[10];
	int      sp;
} MockRenderer;

static void
mock_low_new (WaveformActor* actor)
{
	if (!actor->waveform->priv->render_data[MODE_LOW])
		actor->waveform->priv->render_data[MODE_LOW] = WF_NEW(WaveformModeRender, .n_blocks = 1);
	if (!actor->waveform->priv->render_data[MODE_V_LOW])
		actor->waveform->priv->render_data[MODE_V_LOW] = WF_NEW(WaveformModeRender, .n_blocks = 1);
}

static void
mock_load_block (Renderer* renderer, WaveformActor* actor, int b)
{
	MockRenderer* mr = (MockRenderer*)renderer;
	mr->load_blocks_calls[((MockRenderer*)renderer)->sp++] = b;
}

static bool
mock_low_pre_render (Renderer* renderer, WaveformActor* actor)
{
	return true;
}

static bool
mock_low_render_block (Renderer* renderer, WaveformActor* actor, int b, bool is_first, bool is_last, double x)
{
	return true;
}

static void
mock_low_post_render (Renderer* renderer, WaveformActor* actor)
{
}

static void
mock_low_free_waveform (Renderer* renderer, Waveform* waveform)
{
}

MockRenderer mock_renderer_v_low = {{MODE_V_LOW, mock_low_new, mock_load_block, mock_low_pre_render, mock_low_render_block, mock_low_post_render, mock_low_free_waveform}};
MockRenderer mock_renderer = {{MODE_LOW, mock_low_new, mock_load_block, mock_low_pre_render, mock_low_render_block, mock_low_post_render, mock_low_free_waveform}};
MockRenderer mock_renderer_med = {{MODE_MED, mock_low_new, mock_load_block, mock_low_pre_render, mock_low_render_block, mock_low_post_render, mock_low_free_waveform}};
MockRenderer mock_renderer_hi = {{MODE_HI, mock_low_new, mock_load_block, mock_low_pre_render, mock_low_render_block, mock_low_post_render, mock_low_free_waveform}};

void
clear_mocks ()
{
	for (int i = MODE_V_LOW;i<=MODE_HI;i++)
		((MockRenderer*)modes[i].renderer)->sp = 0;
}


bool
array_equal (int actual[], int size1, int expected[], int size2)
{
	if (size1 != size2) {
		perr("size %i, expected %i", size1, size2);
		return false;
	}
	for (int i = 0; i < size1; i++) {
		if (actual[i] != expected[i]) {
			perr("mismatch at %i: %i", i, actual[i]);
			return false;
		}
	}
	return true;
}

MockRenderer* vlow;
MockRenderer* low;
MockRenderer* med;
MockRenderer* hi;

Waveform* waveform;
WaveformContext* context;
WaveformActor* wf_actor;
AGlScene scene = { .type = 4, .enable_animations = false };

int
setup ()
{
	modes[MODE_V_LOW].renderer = (Renderer*)&mock_renderer_v_low;
	modes[MODE_LOW].renderer = (Renderer*)&mock_renderer;
	modes[MODE_MED].renderer = (Renderer*)&mock_renderer_med;
	modes[MODE_HI].renderer = (Renderer*)&mock_renderer_hi;

	vlow = (MockRenderer*)modes[MODE_V_LOW].renderer;
	low = (MockRenderer*)modes[MODE_LOW].renderer;
	med = (MockRenderer*)modes[MODE_MED].renderer;
	hi = (MockRenderer*)modes[MODE_HI].renderer;

	agl = &_agl;
	_agl.use_shaders = true;
	scene.gl.glx.draw_idle = 1; // prevent idle from running

	g_autofree char* filename = find_wav(WAV);
	waveform = waveform_new(filename);

	context = wf_context_new(NULL);
	wf_actor = wf_context_add_new_actor(context, waveform);
	AGlActor* actor = (AGlActor*)wf_actor;

	actor->root = &scene;
	actor->parent = (AGlActor*)&scene;
	((AGlActor*)actor->root)->root = actor->root;
	context->root = (AGlActor*)&scene;
	context->root->region = (AGlfRegion){ .x2 = 100 };

	return 0;
}

void
teardown ()
{
	wf_context_free(context);
	g_object_unref(waveform);
}

void
test_load_missing_blocks ()
{
	START_TEST;

	char* stepname = NULL;
	void _step (const char* name) {
		stepname = (char*)name;
		clear_mocks();
	}

	AGlActor* actor = (AGlActor*)wf_actor;

	bool result = actor->paint(actor);
	assert(result == false, "expect false resullt");
#ifdef DEBUG
	assert(wf_actor->render_result == RENDER_RESULT_LOADING, "expected LOADING");
#endif

	bool loaded = waveform_load_sync(waveform);
	assert(loaded, "not loaded");
	result = actor->paint(actor);
	assert(result == false, "expect false resullt");
#ifdef DEBUG
	assert(wf_actor->render_result == RENDER_RESULT_NO_REGION, "expected NO_REGION");
#endif

	wf_actor_set_region(wf_actor, &(WfSampleRegion){ .len = waveform->n_frames});
	actor->region.x2 = RIGHT(actor).target_val.f = 100.;
	result = actor->paint(actor);
	assert(result == false, "expect false resullt");
#ifdef DEBUG
	assert(wf_actor->render_result == RENDER_RESULT_LOADING, "expected NO_REGION, got %i", wf_actor->render_result);
#endif
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called")

	_step("LOW");
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname)
	{
	int expected[] = {0};
	assert(array_equal(low->load_blocks_calls, low->sp, expected, G_N_ELEMENTS(expected)), "%s: blocks", stepname)
	}

	_step("V_LOW");
	// in V_LOW mode, blocks are not loaded until the peaks promise is resolved
	actor->region.x2 = RIGHT(actor).target_val.f = 2.;
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow blocks not called", stepname)
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "%s: low expected not called", stepname)
	am_promise_resolve(waveform->priv->peaks, NULL);
	_wf_actor_load_missing_blocks(wf_actor);
	int expected2[] = {0};
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, expected2, G_N_ELEMENTS(expected2)), "%s: vlow blocks", stepname)
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called")

	_step("V_LOW --> LOW");
	actor->region.x2 = 2.;
	RIGHT(actor).target_val.f = 100.;
	_wf_actor_load_missing_blocks(wf_actor);
	int expected1[] = {0};
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, expected1, G_N_ELEMENTS(expected1)), "%s: vlow blocks", stepname)
	assert(array_equal(low->load_blocks_calls, low->sp, expected1, G_N_ELEMENTS(expected1)), "%s: low blocks", stepname)

	_step("zoomed");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){ .len = waveform->n_frames / 2});
	actor->region.x2 = RIGHT(actor).target_val.f = 100.;
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname)
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "%s: low expected not called", stepname)
	{
	int expected[] = {0, 1, 2, 3};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: med blocks", stepname)
	}

	_step("zoomed with inset");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){waveform->n_frames / 4, waveform->n_frames / 2});
	actor->region.x2 = RIGHT(actor).target_val.f = 100.;
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname);
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "%s: low expected not called", stepname)
	{
	int expected[] = {1, 2, 3, 4, 5};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: array", stepname)
	}

	_step("zoomed at end");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){waveform->n_frames / 2, waveform->n_frames / 2});
	actor->region.x2 = RIGHT(actor).target_val.f = 100.;
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname);
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called")
	{
	int expected[] = {3, 4, 5, 6};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: array", stepname)
	}

	_step("zoomed (scaled mode)");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){ .len = waveform->n_frames / 2});
	actor->region.x2 = RIGHT(actor).target_val.f = 100.;
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);
	wf_context_set_zoom(context, 1.);
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname)
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called")
	{
	int expected[] = {0, 1, 2, 3};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "array")
	}

	_step("zoomed with inset (scaled mode)");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){waveform->n_frames / 4, waveform->n_frames / 2});
	actor->region.x2 = RIGHT(actor).target_val.f = 100.;
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);
	wf_context_set_zoom(context, 1.);
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname);
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "%s: low expected not called", stepname)
	{
	int expected[] = {1, 2, 3, 4, 5};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: array", stepname)
	}

	_step("zoomed at end (scaled mode)");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){waveform->n_frames / 2, waveform->n_frames / 2});
	actor->region.x2 = RIGHT(actor).target_val.f = 100.;
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);
	wf_context_set_zoom(context, 1.);
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname);
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called")
	{
	int expected[] = {3, 4, 5, 6};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: array", stepname)
	}

	_step("crop to viewport (med left)");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = -100, .len = 200});
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname);
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "%s: low expected not called", stepname)
	{
	int expected[] = {3, 4, 5, 6};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: med blocks", stepname)
	}

	_step("crop to viewport (right)");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = 0, .len = 200});
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname);
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "%s: low expected not called", stepname);
	{
	int expected[] = {0, 1, 2, 3};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: med blocks", stepname);
	}

	_step("crop to viewport (middle)");
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = -50, .len = 200});
	_wf_actor_load_missing_blocks(wf_actor);
	assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "%s: vlow expected not called", stepname);
	assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "%s: low expected not called", stepname);
	{
	int expected[] = {1, 2, 3, 4, 5};
	assert(array_equal(med->load_blocks_calls, med->sp, expected, G_N_ELEMENTS(expected)), "%s: med  blocks", stepname);
	}

	FINISH_TEST;
}

void
test_load_missing_blocks_hi_left ()
{
	START_TEST;
	clear_mocks();
	AGlActor* actor = (AGlActor*)wf_actor;

	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = -20000, .len = 20100});
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);

	_wf_actor_load_missing_blocks(wf_actor);

	bool is_loaded (gpointer _)
	{
		int expected[] = {6, 6};
		return array_equal(hi->load_blocks_calls, hi->sp, expected, G_N_ELEMENTS(expected));
	}

	void (on_loaded) (gpointer _)
	{
		assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "vlow expected not called");
		assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called");
		assert(array_equal(med->load_blocks_calls, med->sp, NULL, 0), "med expected not called");

		int expected[] = {6, 6};
		assert(array_equal(hi->load_blocks_calls, hi->sp, expected, G_N_ELEMENTS(expected)), "hi blocks");

		FINISH_TEST;
	}
	wait_for (is_loaded, on_loaded, NULL);
}

void
test_load_missing_blocks_hi_right ()
{
	START_TEST;

	clear_mocks();
	AGlActor* actor = (AGlActor*)wf_actor;

	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .len = 20100});
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);

	_wf_actor_load_missing_blocks(wf_actor);

	bool is_loaded (gpointer _)
	{
		int expected[] = {0, 0};
		return array_equal(hi->load_blocks_calls, hi->sp, expected, G_N_ELEMENTS(expected));
	}

	void (on_loaded) (gpointer _)
	{
		assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "vlow expected not called");
		assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called");
		assert(array_equal(med->load_blocks_calls, med->sp, NULL, 0), "med expected not called");

		int expected[] = {0, 0};
		assert(array_equal(hi->load_blocks_calls, hi->sp, expected, G_N_ELEMENTS(expected)), "hi blocks");

		FINISH_TEST;
	}
	wait_for (is_loaded, on_loaded, NULL);
}

void
test_load_missing_blocks_hi_middle ()
{
	START_TEST;

	clear_mocks();
	AGlActor* actor = (AGlActor*)wf_actor;

	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = -10000, .len = 20100});
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);

	_wf_actor_load_missing_blocks(wf_actor);

	bool is_loaded (gpointer _)
	{
		int expected[] = {3, 3};
		return array_equal(hi->load_blocks_calls, hi->sp, expected, G_N_ELEMENTS(expected));
	}

	void (on_loaded) (gpointer _)
	{
		assert(array_equal(vlow->load_blocks_calls, vlow->sp, NULL, 0), "vlow expected not called");
		assert(array_equal(low->load_blocks_calls, low->sp, NULL, 0), "low expected not called");
		assert(array_equal(med->load_blocks_calls, med->sp, NULL, 0), "med expected not called");

		int expected[] = {3, 3};
		assert(array_equal(hi->load_blocks_calls, hi->sp, expected, G_N_ELEMENTS(expected)), "hi blocks");

		FINISH_TEST;
	}
	wait_for (is_loaded, on_loaded, NULL);
}

void
test_px_2_frames ()
{
	START_TEST;
	AGlActor* actor = (AGlActor*)wf_actor;

	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = 0, .len = 100});
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);

	double zoom ()
	{
		return  context->scaled
			? context->zoom->value.f / context->samples_per_pixel
			: agl_actor__width(actor) / wf_actor->region.len;
	}

	#define EXPECT_FRAME(PX, EXPECTED) \
		r = px_2_f (wf_actor, zoom(), PX); \
		assert(EXPECTED - r <= 1, "expected %"PRIu64" got %"PRIu64, EXPECTED, r);

	uint64_t r = px_2_f (wf_actor, zoom(), 0);
	assert(r == 0, "expected %i got %"PRIu64, 0, r);
	r = px_2_f (wf_actor, zoom(), 100);
	assert(waveform->n_frames - r <= 1, "expected %"PRIu64" got %"PRIu64, waveform->n_frames, r);

	// region inset
	wf_actor_set_region(wf_actor, &(WfSampleRegion){10, waveform->n_frames - 10});
	context->scaled = false;
	EXPECT_FRAME(0, 10UL);
	EXPECT_FRAME(100, waveform->n_frames);

	// region inset, scaled
	wf_actor_set_region(wf_actor, &(WfSampleRegion){10, waveform->n_frames - 10});
	context->scaled = true;
	EXPECT_FRAME(0, 10UL);
	EXPECT_FRAME(100, waveform->n_frames + 10);

	// small region
	context->scaled = false;
	wf_actor_set_region(wf_actor, &(WfSampleRegion){waveform->n_frames / 4, waveform->n_frames / 2});
	EXPECT_FRAME(0, waveform->n_frames / 4);
	EXPECT_FRAME(100, waveform->n_frames * 3 / 4);

	context->scaled = true;

	// actor offset
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = 10, .len = 100});
	r = px_2_f (wf_actor, zoom(), 10);
	assert(r == 0, "expected %i got %"PRIu64, 0, r);
	r = px_2_f (wf_actor, zoom(), 110);
	assert(waveform->n_frames - r <= 1, "expected %"PRIu64" got %"PRIu64, waveform->n_frames, r);

	// actor negative offset
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = -10, .len = 100});
	r = px_2_f (wf_actor, zoom(), -10);
	assert(r == 0, "expected %i got %"PRIu64, 0, r);
	r = px_2_f (wf_actor, zoom(), 90);
	assert(waveform->n_frames - r <= 1, "expected %"PRIu64" got %"PRIu64, waveform->n_frames, r);

	// wide actor, scaled mode
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = -10, .len = 120});
	r = px_2_f (wf_actor, zoom(), -10);
	assert(r == 0, "expected %i got %"PRIu64, 0, r);
	assert(context->scaled, "scaled");
	r = px_2_f (wf_actor, zoom(), 110);
	assert(waveform->n_frames * 6 / 5 - r <= 1, "expected %"PRIu64" got %"PRIu64, waveform->n_frames * 6 / 5, r);

	// wide actor, non scaled mode
	context->scaled = false;
	r = px_2_f (wf_actor, zoom(), -10);
	assert(r == 0, "expected %i got %"PRIu64, 0, r);
	r = px_2_f (wf_actor, zoom(), 110);
	assert(waveform->n_frames - r <= 1, "expected %"PRIu64" got %"PRIu64, waveform->n_frames * 6 / 5, r);

	// zoomed
	context->scaled = true;
	context->zoom->value.f = 2.;
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .len = 100});
	r = px_2_f (wf_actor, zoom(), 0);
	assert(r == 0, "expected %i got %"PRIu64, 0, r);
	r = px_2_f (wf_actor, zoom(), 100);
	assert(waveform->n_frames / 2 - r <= 1, "expected %"PRIu64" got %"PRIu64, waveform->n_frames / 2, r);

	FINISH_TEST;
}

void
test_frames_2_px ()
{
	START_TEST;
	AGlActor* actor = (AGlActor*)wf_actor;

	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, waveform->n_frames});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = 0, .len = 100});
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);

	double zoom ()
	{
		return  context->scaled
			? context->zoom->value.f / context->samples_per_pixel
			: agl_actor__width(actor) / wf_actor->region.len;
	}

	#define EXPECT_PX(FRAME, EXPECTED) \
		r = f_2_px (wf_actor, zoom(), FRAME); \
		assert(r == EXPECTED, "expected %.0f got %.0f", EXPECTED, r);

	#define EXPECT_PX_APPROX(FRAME, EXPECTED) \
		r = f_2_px (wf_actor, zoom(), FRAME); \
		assert(r - EXPECTED < 0.001, "expected %.0f got %.0f", EXPECTED, r);

	float r;

	context->scaled = false;
	EXPECT_PX(0, 0.);
	EXPECT_PX(waveform->n_frames, 100.);

	// region inset
	wf_actor_set_region(wf_actor, &(WfSampleRegion){10, waveform->n_frames - 10});
	context->scaled = false;
	EXPECT_PX(10, 0.);
	EXPECT_PX(waveform->n_frames, 100.);
	EXPECT_PX_APPROX(0, -100. * (10. / waveform->n_frames));

	// short region
	wf_actor_set_region(wf_actor, &(WfSampleRegion){waveform->n_frames / 4, waveform->n_frames / 2});
	context->scaled = false;
	EXPECT_PX(waveform->n_frames / 4, 0.);
	EXPECT_PX(waveform->n_frames * 3 / 4, 100.);

	// short region, scaled
	context->scaled = true;
	assert(context->zoom->value.f == 2., "zoom");
	EXPECT_PX(waveform->n_frames / 4, 0.);
	EXPECT_PX(waveform->n_frames * 3 / 4, 100.);
	EXPECT_PX(waveform->n_frames, 150.);

	context->zoom->value.f = 4.;
	EXPECT_PX(waveform->n_frames * 3 / 4, 200.);

	// blocks
	context->zoom->value.f = 1.;
	wf_actor_set_region(wf_actor, &(WfSampleRegion){0, WF_SAMPLES_PER_TEXTURE});
	wf_actor_set_rect(wf_actor, &(WfRectangle){ .left = 0, .len = 100});
	context->samples_per_pixel = (float)wf_actor->region.len / agl_actor__width(actor);
	EXPECT_PX(WF_SAMPLES_PER_TEXTURE, 100.);

	FINISH_TEST;
}
