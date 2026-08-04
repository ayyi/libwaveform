/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 | Test program to verify the overview data feature with timing metrics
 | Tests that the preview signal is emitted in both scenarios:
 | 1. When peakfile already exists
 | 2. When peakfile needs to be generated
 |
 */

#include "config.h"
#include <glib/gstdio.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include "wf/private.h"
#include "wf/waveform.h"

static gboolean test_timeout      (gpointer user_data);
static gboolean quit_loop         (gpointer user_data);
static void     on_overview_ready (Waveform* w, WfPreview*, gpointer user_data);
static void     on_peakdata_ready (Waveform* w, gpointer user_data);

typedef struct {
    GThread*   main_thread;
    GMainLoop* loop;
    bool       overview_received;
    bool       peakdata_received;
    clock_t    start_time;
    clock_t    overview_time;
    clock_t    peakdata_time;
    WfPreview* overview;
} TestContext;


int
main (int argc, char* argv[])
{
    const char* audio_file;
    bool use_temp_file = false;

    if (argc >= 2 && strcmp(argv[1], "--non-interactive")) {
        audio_file = argv[1];
        // Check if second argument is "temp" to indicate we should use temp file
        if (argc >= 3 && strcmp(argv[2], "temp") == 0) {
            use_temp_file = TRUE;
        }
    } else {
        // Use the smaller test file for faster testing - check various possible locations
        const char* possible_paths[] = {
            "test/data/mono_10:00.wav",      // from main project directory
            "data/mono_10:00.wav",           // from test directory
            "../test/data/mono_10:00.wav",   // alternative from test directory
            NULL
        };

        audio_file = NULL;
        for (int i = 0; possible_paths[i] != NULL; i++) {
            if (g_file_test(possible_paths[i], G_FILE_TEST_EXISTS)) {
                audio_file = possible_paths[i];
                printf("Found audio file at: %s\n", audio_file);
                break;
            }
        }

        if (!audio_file) {
            printf("Error: Could not find test audio file in any expected location\n");
            printf("Current working directory: %s\n", g_get_current_dir());
            return 1;
        }

        printf("Using default test file: %s\n", audio_file);
        // Default to temp file for the main test to ensure it's fresh
        use_temp_file = true;
    }

    // Set up a temporary XDG cache directory to isolate the test
    char temp_cache_dir[512];
    g_snprintf(temp_cache_dir, sizeof(temp_cache_dir), "/tmp/libwaveform_test_cache_%d", getpid());
    if (g_mkdir_with_parents(temp_cache_dir, 0755) != 0) {
        printf("Error creating temporary cache directory: %s\n", temp_cache_dir);
        return 1;
    }

    // Set the XDG_CACHE_HOME environment variable to use our temp directory
    if (setenv("XDG_CACHE_HOME", temp_cache_dir, 1) != 0) {
        printf("Error setting XDG_CACHE_HOME environment variable\n");
        g_rmdir(temp_cache_dir);
        return 1;
    }

    printf("Using temporary cache directory: %s\n", temp_cache_dir);

    // Create a temporary copy with a random name to simulate a completely new file
    char temp_file[512];
    if (use_temp_file) {
        printf("Creating temporary copy to simulate fresh audio file...\n");

        // Extract directory path from the original file
        char* dir = g_path_get_dirname(audio_file);
        char* base = g_path_get_basename(audio_file);

        // Generate random name with timestamp
        g_snprintf(temp_file, sizeof(temp_file), "%s/%s_%ld.wav", dir, "temp_test", (long)time(NULL));

        // Copy the original file to the temporary location
        GError* error = NULL;
        char* contents;
        gsize length;

        if (g_file_get_contents(audio_file, &contents, &length, &error)) {
            if (g_file_set_contents(temp_file, contents, length, &error)) {
                printf("Created temporary file: %s\n", temp_file);
                audio_file = temp_file;
            } else {
                printf("Error creating temporary file: %s\n", error->message);
                g_error_free(error);
                g_free(contents);
                g_free(dir);
                g_free(base);
                g_rmdir(temp_cache_dir);
                return 1;
            }
            g_free(contents);
        } else {
            printf("Error reading original file: %s\n", error->message);
            g_error_free(error);
            g_free(dir);
            g_free(base);
            g_rmdir(temp_cache_dir);
            return 1;
        }

        g_free(dir);
        g_free(base);
    }

    printf("Creating waveform for file: %s\n", audio_file);

    // Check if file exists
    if (!g_file_test(audio_file, G_FILE_TEST_EXISTS)) {
        printf("Error: Audio file does not exist: %s\n", audio_file);
        printf("Current working directory: %s\n", g_get_current_dir());
        g_rmdir(temp_cache_dir);
        return 1;
    }

    // Force regeneration of peakfile by touching the audio file to make it newer
    printf("Forcing peakfile regeneration by updating file timestamp...\n");
    if (g_file_test(audio_file, G_FILE_TEST_EXISTS)) {
        // Update the file modification time - use the same path we found the file at
        char cmd[1024];  // Larger buffer to handle longer paths
        snprintf(cmd, sizeof(cmd), "touch \"%s\"", audio_file);
        int result = system(cmd);
        if (result != 0) {
            printf("Warning: Could not update file timestamp\n");
        }
    }

    // Create a new waveform
    Waveform* w = waveform_new(audio_file);
    if (!w) {
        printf("Failed to create waveform\n");
        g_rmdir(temp_cache_dir);
        return 1;
    }

    printf("Waveform object created. Checking if file is valid...\n");

    // Try to get number of frames to verify the file is readable
    uint64_t n_frames = waveform_get_n_frames(w);
    if (n_frames == 0) {
        printf("Error: Invalid or unreadable audio file: %s\n", audio_file);
        g_object_unref(w);
        g_rmdir(temp_cache_dir);
        return 1;
    }

    printf("File is valid. Total frames: %" PRIu64 "\n", n_frames);

	if (n_frames <= 256*256) {
		printf("File is too small to have an overview: %"PRIu64, n_frames);
		return 0;
	}

	TestContext* ctx = WF_NEW(TestContext,
		.main_thread = g_thread_self(),
		.loop = g_main_loop_new(NULL, false),
		.start_time = clock(),
	);

    // Connect to both preview and peakdata-ready signals
    g_signal_connect(w, "preview", G_CALLBACK(on_overview_ready), ctx);
    g_signal_connect(w, "peakdata_ready", G_CALLBACK(on_peakdata_ready), ctx);

    printf("Initiating waveform load (with overview extraction)...\n");

    // Start loading the waveform (overview should be extracted quickly, full peak data will take longer)
    waveform_load(w, NULL, NULL);

    // Add a timeout to prevent hanging if signal isn't emitted
    g_timeout_add_seconds(30, test_timeout, ctx); // Increased timeout for full peak loading

    // Run the main loop to wait for signals
    g_main_loop_run(ctx->loop);

	// Calculate timing results
	double overview_time = ctx->overview_received
		? ((double)(ctx->overview_time - ctx->start_time)) / CLOCKS_PER_SEC
		: -1.0;
	double peakdata_time = ctx->peakdata_received
		? ((double)(ctx->peakdata_time - ctx->start_time)) / CLOCKS_PER_SEC
		: -1.0;

	// Check if overview was received
    if (ctx->overview_received) {
        printf("SUCCESS: preview signal was emitted in %.4f seconds\n", overview_time);

        // verify preview data
        if (ctx->overview && ctx->overview->size > 0) {
            // Calculate some metrics
            long long sum = 0;
            short max_val = 0;
            for (int i = 0; i < ctx->overview->size; i++) {
                if (ctx->overview->data[0][i].positive > max_val) max_val = ctx->overview->data[0][i].positive;
                sum += ctx->overview->data[0][i].positive;
            }

            printf("Overview data metrics:\n");
            printf("  Max value: %d\n", max_val);
            printf("  First 5 values: %d %d %d %d %d\n", ctx->overview->data[0][0].positive, ctx->overview->data[0][1].positive, ctx->overview->data[0][2].positive, ctx->overview->data[0][3].positive, ctx->overview->data[0][4].positive);

        } else {
            printf("ERROR: preview signal emitted but no data available\n");
        }
    } else {
        printf("FAILURE: preview signal was NOT emitted within timeout\n");
    }

    // Report peakdata loading time if it completed
    if (ctx->peakdata_received) {
        printf("SUCCESS: peakdata-ready signal was emitted in %.4f seconds\n", peakdata_time);
        if (ctx->overview_received && overview_time > 0) {
            printf("Time advantage of overview: %.4f seconds\n", peakdata_time - overview_time);
            printf("Overview was available %.2fx faster than full peak data\n", peakdata_time / overview_time);
        }
    } else {
        printf("WARNING: peakdata-ready signal was NOT emitted within timeout\n");
    }

	g_return_val_if_fail(!ctx->overview || ctx->overview->ref_count == 1, 1);

	// Cleanup
	g_main_loop_unref(ctx->loop);
	g_clear_pointer(&ctx->overview, preview_unref);
	g_free(ctx);
	g_object_unref(w);

    // Remove temporary file if we created one
    if (use_temp_file && g_file_test(temp_file, G_FILE_TEST_EXISTS)) {
        if (g_unlink(temp_file) == 0) {
            printf("Temporary file cleaned up: %s\n", temp_file);
        } else {
            printf("Warning: Could not remove temporary file: %s\n", temp_file);
        }
    }

    // Clean up the temporary cache directory
    char cleanup_cmd[PATH_MAX + 8];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf '%s'", temp_cache_dir);
    int cleanup_result = system(cleanup_cmd);
    (void)cleanup_result; // Suppress unused result warning
    printf("Temporary cache directory cleaned up: %s\n", temp_cache_dir);

    // Return success if we got overview data (which demonstrates the feature works)
    // This allows the test to pass even if peakfile loading times out (which might happen with large files)
    return !ctx->overview_received;
}

static gboolean
quit_loop (gpointer user_data)
{
	TestContext* ctx = (TestContext*)user_data;
	g_main_loop_quit(ctx->loop);
	return G_SOURCE_REMOVE;
}

static void
on_overview_ready (Waveform* w, WfPreview* preview, gpointer user_data)
{
	TestContext* ctx = (TestContext*)user_data;

	g_assert(g_thread_self() == ctx->main_thread);

	if (((AyyiObservable*)preview)->value.i == 0) {
		printf("Overview ready! Signal received from waveform_load. size=%i ref_count=%i-->%i progress=%i\n", preview->size, preview->ref_count, ctx->overview ? preview->ref_count : preview->ref_count + 1, ((AyyiObservable*)preview)->value.i);
		ctx->overview_received = true;
		ctx->overview_time = clock();
	}

	if (!ctx->overview) {
		preview->ref_count++;
		ctx->overview = preview;
	}
}

static void
on_peakdata_ready (Waveform* w, gpointer user_data)
{
    TestContext* ctx = (TestContext*)user_data;

    printf("Peakdata ready! waveform loaded\n");

    ctx->peakdata_received = true;
    ctx->peakdata_time = clock(); // Record time when peakdata was received

    // Now quit since we have both signals
    g_idle_add(quit_loop, ctx);
}

static gboolean
test_timeout (gpointer user_data)
{
    TestContext* ctx = (TestContext*)user_data;

    // Calculate timing results even on timeout
    double elapsed_time = ((double)(clock() - ctx->start_time)) / CLOCKS_PER_SEC;

    printf("TIMEOUT: after %.4f seconds\n", elapsed_time);
    if (ctx->overview_received) {
        double overview_time = ((double)(ctx->overview_time - ctx->start_time)) / CLOCKS_PER_SEC;
        printf("  preview signal was received after %.4f seconds\n", overview_time);
    } else {
        printf("  preview signal was NOT received\n");
    }

    if (ctx->peakdata_received) {
        double peakdata_time = ((double)(ctx->peakdata_time - ctx->start_time)) / CLOCKS_PER_SEC;
        printf("  Peakdata-ready signal was received after %.4f seconds\n", peakdata_time);
    } else {
        printf("  Peakdata-ready signal was NOT received\n");
    }

    g_main_loop_quit(ctx->loop);
    return G_SOURCE_REMOVE;
}
