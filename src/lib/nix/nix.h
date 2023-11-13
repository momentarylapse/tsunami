/*----------------------------------------------------------------------------*\
| Nix                                                                          |
| -> abstraction layer for api (graphics)                                      |
| -> OpenGL and DirectX9 supported                                             |
|   -> able to switch in runtime                                               |
| -> mathematical types and functions                                          |
|   -> vector, matrix, matrix3, quaternion, plane, color                       |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.10.27 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#pragma once


#include "../config.h"

#include "nix_config.h"
#include "nix_draw.h"
#include "nix_light.h"
#include "nix_vertexbuffer.h"
#include "nix_shader.h"
#include "nix_view.h"
#include "nix_textures.h"
#include "nix_framebuffer.h"
#include "nix_buffer.h"

namespace nix {

extern string version;

enum class CullMode;
enum class Orientation;
enum class AlphaMode;
enum class Alpha;
enum class StencilOp;


struct ShaderModule;

class Context {
public:
	~Context();
	Array<string> extensions;
	string version;
	string gl_version;
	string gl_renderer;
	string glsl_version;

	string vertex_module_default;
	Array<ShaderModule> shader_modules;

	int verbosity = 1;
	int current_program = 0;

	string shader_error;

	shared<Texture> tex_white = nullptr;
	FrameBuffer *default_framebuffer = nullptr;
	shared<Shader> default_2d;
	shared<Shader> default_3d;
	shared<Shader> default_load;
	Shader *_current_ = nullptr;
	VertexBuffer *vb_temp = nullptr;
	VertexBuffer *vb_temp_i = nullptr;


	xfer<Shader> load_shader(const Path &filename);
	xfer<Shader> create_shader(const string &source);

	int available_mem() const;
	int total_mem() const;

	static Context *CURRENT;
};

xfer<Context> init(const Array<string>& = {});
void kill(Context *ctx);
void flush();

void create_query_pool(int size);
void query_timestamp(int index);
Array<int64> get_timestamps(int first, int count);

enum class FogMode;

// engine properties
void _cdecl set_wire(bool enabled);
void _cdecl set_cull(CullMode mode);
void _cdecl set_front(Orientation front);
void _cdecl set_z(bool write, bool test);
void _cdecl _set_alpha(AlphaMode mode);
void _cdecl set_alpha(Alpha src, Alpha dst);
void _cdecl _set_alpha_mode(AlphaMode mode);
void _cdecl set_alpha_sd(Alpha src, Alpha dst);
void _cdecl set_alpha_split(Alpha color_src, Alpha color_dst, Alpha alpha_src, Alpha alpha_dst);
void _cdecl disable_alpha();
void _cdecl set_fog(FogMode mode, float start, float end, float density, const color &c);
void _cdecl enable_fog(bool enabled);
void _cdecl set_stencil(StencilOp op, unsigned long param=0);
void _cdecl set_offset(float offset);


extern rect target_rect;

};

#endif
