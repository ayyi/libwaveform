
[CCode(cheader_filename = "agl/utils.h")]
namespace AGl
{
	public struct UniformInfo
	{
		public char*    name;
		public uint     size;
		public int      type;          // enum. GL_FLOAT or GL_INT
		public float    value[];
		public int      location;      // filled in by agl_uniforms_init()
	}

	public struct ShaderText
	{
		char*        vert;
		char*        frag;
	}

	[compact]
	public class Shader
	{
		char*           vertex_file;
		char*           fragment_file;
		uint32          program;       // compiled program
		UniformInfo*    uniforms;
		void            set_uniforms();
		ShaderText*     text;
	}

	[compact]
	public class AlphaMapShader : Shader {
		public struct u {
			uint32 fg_colour;
		}
		public u uniform;
	}

	[CCode(cname = "struct _agl")]
	public class Instance
	{
		public bool pref_use_shaders;
		public bool use_shaders;

		public struct Shaders
		{
			public AlphaMapShader texture;
		}

		public Shaders shaders;

	}

	Instance* get_instance   ();
	bool shaders_supported   ();
	void use_program         (Shader* shader);
}
