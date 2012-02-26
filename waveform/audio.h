#ifndef __wf_audio_h__
#define __wf_audio_h__

#define MAX_TIERS 8 //this is related to WF_PEAK_RATIO: WF_PEAK_RATIO = 2 ^ MAX_TIERS.
struct _audio_data {
	int                n_blocks;          // the size of the buf array
	WfBuf16**          buf16;             // pointers to arrays of blocks, one per block.
	int                n_tiers_present;
};

#endif //__wf_audio_h__
