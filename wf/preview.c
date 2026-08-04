/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2025-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */


/*
 * Extract overview data directly from a file and save to peakfile
 * This can be called from worker threads without requiring a full Waveform object
 */
bool
preview_extract_from_file (const char* filename, const char* peakfile, WfPreview* overview)
{
	// Open the audio file for reading
	WfDecoder d = {{0,}};
	if (!ad_open(&d, filename)) {
		if (wf_debug) pwarn("Failed to open audio file for overview extraction: %s", filename);
		return false;
	}

	uint64_t total_frames = d.info.frames;

	int n_channels = MIN(d.info.channels, WF_MAX_CH);

	// Calculate how to distribute 1024 samples across the entire file
	// Each overview point will represent a segment of the audio file
	double frames_per_overview_point = (double)total_frames / 1024.0;

	// Buffer to read audio frames - we'll read 16 frames per overview point as suggested
	const int SAMPLES_PER_OVERVIEW = 16;
	WfBuf16 read_buf = {
		.size = SAMPLES_PER_OVERVIEW,
		.buf[0] = g_malloc0(SAMPLES_PER_OVERVIEW * sizeof(short)),
		.buf[1] = d.info.channels > 1 ? g_malloc0(SAMPLES_PER_OVERVIEW * sizeof(short)) : NULL,
	};

	// Sample across the entire file
	for (int i = 0; i < PREVIEW_SIZE; i++) {
		// Calculate the start frame for this overview sample
		uint64_t start_frame = (uint64_t)(i * frames_per_overview_point);

		// Seek to the appropriate frame
		if (ad_seek(&d, start_frame) < 0) {
			continue;
		}

		// Read SAMPLES_PER_OVERVIEW frames of audio data
		ssize_t frames_read = ad_read_short(&d, &read_buf);
		if (frames_read <= 0) {
			continue;
		}

		// Find the maximum absolute value from the 16 frames across all channels
		WfPeak max[WF_STEREO] = {0};
		int c;
		for (int f = 0; f < frames_read; f++) {
			for (c = 0; c < n_channels; c++) {
				int16_t val = read_buf.buf[c][f];
				if (val > max[c].positive) max[c].positive = val;
				else if (val < max[c].negative) max[c].negative = val;
			}
		}

		for (int c = 0; c < n_channels; c++) {
			overview->data[c][i] = max[c];
		}
	}
	overview->size = PREVIEW_SIZE;

	// Clean up
	for (int c = 0; c < d.info.channels && c < WF_MAX_CH; c++) {
		if (read_buf.buf[c]) g_free(read_buf.buf[c]);
	}
	ad_close(&d);

	return true;
}


void
preview_unref (WfPreview* o)
{
	g_return_if_fail(o);
	dbg(1, "%i-->%i", o->ref_count, o->ref_count - 1);

	if (--o->ref_count == 0) {
		_g_source_remove0(o->idle_id);
		o->size = 0;
		call(o->clear_render_data, o);
		ayyi_observable_free((AyyiObservable*)o);
	}
}


typedef struct {
   AyyiObservableFn fn;
   gpointer         user;
} Subscription;


/*
 *  Sets the observable value with the callbacks being in the main thread, and throttled
 */
void
preview_set (WfPreview* preview, int value)
{
	// in worker thread

	AyyiObservable* o = (AyyiObservable*)preview;

	o->value = (AyyiVal){ .s = {
		.val = value,
		.prev = ((AyyiObservable*)preview)->value.s.prev
	}};

	if (!value) {
		gboolean preview_set_idle0 (void* _preview)
		{
			// in main thread

			WfPreview* preview = _preview;
			preview->waveform->priv->preview = preview;

			g_signal_emit_by_name(preview->waveform, "preview", preview);

			return G_SOURCE_REMOVE;
		}
		g_idle_add_full(G_PRIORITY_HIGH, preview_set_idle0, preview, NULL);
		return;
	}

	gboolean preview_set__idle (void* _preview)
	{
		// in main thread

		WfPreview* preview = _preview;
		AyyiObservable* o = (AyyiObservable*)preview;
		AyyiVal val = o->value; // copy to ensure the value we are using does not get updated while running the function

		for (GList* l = o->subscriptions; l; l=l->next) {
			Subscription* subscription = l->data;
			subscription->fn(o, val, subscription->user);
		}

		o->value.s.prev = val.s.val;
		return G_SOURCE_REMOVE;
	}

	void callback_done (gpointer preview)
	{
		((WfPreview*)preview)->idle_id = 0;
	}

	if (!preview->idle_id)
		preview->idle_id = g_idle_add_full(G_PRIORITY_HIGH_IDLE, preview_set__idle, preview, callback_done);
}
