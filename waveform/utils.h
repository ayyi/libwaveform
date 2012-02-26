#ifndef __waveform_utils_h__
#define __waveform_utils_h__
#ifndef __utils_c__
extern int wf_debug;
#endif

#ifndef true
#define true TRUE
#define false FALSE
#endif
#ifndef __ayyi_utils_h__
#ifdef __wf_private__
#ifndef bool
#define bool gboolean
#endif
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)
#endif
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define gerr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#define perr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#define PF {if(wf_debug) printf("%s()...\n", __func__);}
#define PF0 {printf("%s()...\n", __func__);}
#define PF2 {if(wf_debug > 1) printf("%s()...\n", __func__);}
#define IDLE_STOP FALSE
#define IDLE_CONTINUE TRUE
#define TIMER_STOP FALSE
#define TIMER_CONTINUE TRUE
#ifndef g_free0
#define g_free0(var) ((var == NULL) ? NULL : (var = (g_free(var), NULL)))
#endif
#ifndef g_list_free0
#define g_list_free0(var) ((var == NULL) ? NULL : (var = (g_list_free (var), NULL)))
#define call(FN, A, ...) if(FN) (FN)(A, ##__VA_ARGS__)
#endif

void       wf_debug_printf         (const char* func, int level, const char* format, ...);
void       deinterleave            (float* src, float** dest, uint64_t n_frames);
void       deinterleave16          (short* src, short** dest, uint64_t n_frames);
int        wf_power_of_two         (int);
float      wf_int2db               (short);

#endif

#endif //__waveform_utils_h__
