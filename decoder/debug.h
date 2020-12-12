#ifndef __decoder_debug_h___
#define __decoder_debug_h___

extern int wf_debug;
#define PF0 printf("%s\n", __func__)
#define dbg(A, STR, ...) ({if(A <= wf_debug) printf("%s: "STR"\n", __func__, ##__VA_ARGS__);})
#define perr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#ifndef gwarn
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__)
#endif
#ifndef pwarn
#define pwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__)
#endif

#endif
