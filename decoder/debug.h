#ifndef __decoder_debug_h___
#define __decoder_debug_h___

extern int wf_debug;
#define PF0 printf("%s\n", __func__)
#define dbg(A, STR, ...) ({if(A <= wf_debug) printf(STR"\n", ##__VA_ARGS__);})
#define perr(A, ...) {}
#define gwarn(A, ...) {}

#endif
