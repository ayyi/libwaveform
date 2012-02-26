
//#define mono_wav_fixture "/usr/share/games/alienarena/data1/sound/items/damage.wav"
#define mono_wav_fixture "/usr/share/sounds/alsa/Front_Center.wav"
#define mono_wav_fixture2 "/usr/lib/openoffice/basis3.3/share/gallery/sounds/train.wav"
//#define mono_wav_fixture "/usr/share/hydrogen/data/click.wav"
//#define stereo_wav_fixture "/usr/share/games/alienarena/data1/sound/music/dm-dynamo.ogg"
#define stereo_wav_fixture "/root/cache/src/xbmc-9.11/skin/PM3.HD/sounds/shutter.wav"
#define vst_plugin_fixture "SaffireEQMono"
#define MIN_TRACKS 1 //server crashes if no tracks?
#define bool gboolean
#define TIMER_CONTINUE TRUE
#define TIMER_STOP FALSE
//#define false FALSE
//#define true TRUE

extern int current_test;

struct _app
{
	gboolean       dbus;
	int            timeout;
	int            n_passed;
} app;

typedef void (*Test)    ();

void test_init          (gpointer tests[], int);
void next_test          ();
void test_finished_     ();

bool get_random_boolean ();
int  get_random_int     (int max);

void errprintf4         (char* format, ...);


#define START_TEST \
	static int step = 0;\
	if(!step){ \
		g_strlcpy(current_test_name, __func__, 64); \
		printf("running %i of %i: %s ...\n", current_test + 1, G_N_ELEMENTS(tests), __func__); \
	} \
	if(test_finished) return;

#define NEXT_CALLBACK(A, B, C) \
	step++; \
	void (*callback)() = callbacks[step]; \
	callback(A, B, C);

#define FINISH_TEST \
	printf("%s: finish\n", current_test_name); \
	test_finished = true; \
	passed = true; \
	test_finished_(); \
	return;

#define FINISH_TEST_TIMER_STOP \
	test_finished = true; \
	passed = true; \
	test_finished_(); \
	return TIMER_STOP;

#define FAIL_TEST(msg, ...) \
	{test_finished = true; \
	passed = false; \
	errprintf4(msg, ##__VA_ARGS__); \
	test_finished_(); \
	return; }

#define FAIL_TEST_TIMER(msg) \
	{test_finished = true; \
	passed = false; \
	printf("%s%s%s\n", red, msg, white); \
	test_finished_(); \
	return TIMER_STOP;}

#define assert(A, B, ...) \
	{bool __ok_ = (bool)A; \
	{if(!__ok_) gerr(B, ##__VA_ARGS__); } \
	{if(!__ok_) FAIL_TEST("assertion failed") }}

#define assert_and_stop(A, B, ...) \
	{bool __ok_ = (bool)A; \
	{if(!__ok_) gerr(B, ##__VA_ARGS__); } \
	{if(!__ok_) FAIL_TEST_TIMER("assertion failed") }}

#define FAIL_IF_ERROR \
	if(error && *error) FAIL_TEST((*error)->message);

