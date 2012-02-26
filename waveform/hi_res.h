#include "waveform/actor.h"

int draw_wave_buffer(int mode, int startx,
	float swidth, float sheight,
	int dwidth, int dheight,
	int offx, int offy,
	int force, float* din,
	uint32_t rgba);

//void     draw_wave_buffer2 (Waveform*, WfSampleRegion, int startx, float swidth, float sheight, int dwidth, int dheight, int offx, int offy, float* din, uint32_t rgba);
void     draw_wave_buffer2     (Waveform*, WfSampleRegion, float v_gain, float dwidth, int dheight, int offx, int offy, float* d_in, uint32_t rgba);
void     draw_wave_buffer_hi   (Waveform*, WfSampleRegion, WfRectangle*, Peakbuf*, int chan, float v_gain, uint32_t rgba);
void     draw_wave_buffer_v_hi (Waveform*, WfSampleRegion, WfRectangle*, WfViewPort*, WfBuf16*, int chan, float v_gain, uint32_t rgba);
gboolean draw_waveform         (WaveformActor*, WfSampleRegion, int width, float height, int offx, int offy, int mode, uint32_t rgba);
