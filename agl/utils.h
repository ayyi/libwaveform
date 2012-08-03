#ifndef __gl_utils_h__
#define __gl_utils_h__

typedef struct _agl              AGl;

AGl*      agl_get_instance      ();
GLboolean agl_shaders_supported ();

struct _agl
{
	gboolean        pref_use_shaders;
	gboolean        use_shaders;
};

#endif //__gl_utils_h__
