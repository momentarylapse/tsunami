/*----------------------------------------------------------------------------*\
| Nix                                                                          |
| -> abstraction layer for api (graphics, sound, networking)                   |
| -> OpenGL and DirectX9 supported                                             |
|   -> able to switch in runtime                                               |
| -> mathematical types and functions                                          |
|   -> vector, matrix, matrix3, quaternion, plane, color                       |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.10.03 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"

#include "nix_config.h"
#include "nix_common.h"
#include "../os/msg.h"


namespace nix{

string version = "0.14.1.0";
// currently, requiring OpenGL 4.5


void TestGLError(const char *pos) {
#if 1
	int err = glGetError();
	if (err == GL_NO_ERROR)
		return;

	string t;
	if (err == GL_INVALID_ENUM)
		t = "GL_INVALID_ENUM at " + string(pos);
	else if (err == GL_INVALID_VALUE)
		t = "GL_INVALID_VALUE at " + string(pos);
	else if (err == GL_INVALID_OPERATION)
		t = "GL_INVALID_OPERATION at " + string(pos);
	else if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
		t = "GL_INVALID_FRAMEBUFFER_OPERATION at " + string(pos);
	else
		t = i2s(err) + " at " + string(pos);
	msg_error(t);
	throw Exception("OpenGL error: " + t);
#endif
}


//int device_width, device_height;
int target_width, target_height; // render target size (window/texture) but relative to viewport!
rect target_rect;

Context* Context::CURRENT = nullptr;

xfer<Shader> Context::load_shader(const Path &filename) {
	return Shader::load(this, filename);
}
xfer<Shader> Context::create_shader(const string &source) {
	return Shader::create(this, source);
}

Fog fog;




//#define ENABLE_INDEX_BUFFERS



void MatrixOut(mat4 &m) {
	msg_write("MatrixOut");
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._00,m._01,m._02,m._03));
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._10,m._11,m._12,m._13));
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._20,m._21,m._22,m._23));
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._30,m._31,m._32,m._33));
}

void mout(mat4 &m) {
	msg_write(format("		%f	%f	%f	%f",m._00,m._01,m._02,m._03));
	msg_write(format("		%f	%f	%f	%f",m._10,m._11,m._12,m._13));
	msg_write(format("		%f	%f	%f	%f",m._20,m._21,m._22,m._23));
	msg_write(format("		%f	%f	%f	%f",m._30,m._31,m._32,m._33));
}

// https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param) {
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		}
		return "?";
	}();

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		}
		return "?";
	}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		}
		return "?";
	}();

	msg_error(format("%s, %s, %s, %d: %s", src_str, type_str, severity_str, id, message));
}

xfer<Context> init(const Array<string>& flags) {
	auto ctx = new Context;

	Context::CURRENT = ctx;

#if HAS_LIB_GLEW
	glewInit();
#endif

	if (!sa_contains(flags, "silent"))
		ctx->verbosity = 1;
	if (sa_contains(flags, "verbose"))
		ctx->verbosity = 2;

	if (ctx->verbosity >= 1) {
		msg_write("nix");
		msg_right();
		msg_write("[" + version + "]");
	}

	ctx->version = version;
	ctx->gl_version = (char *) glGetString(GL_VERSION);
	ctx->gl_renderer = (char *) glGetString(GL_RENDERER);
	ctx->glsl_version = (char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

	if (ctx->verbosity >= 1) {
		msg_write("OpenGL: " + ctx->gl_version);
		msg_write("Renderer: " + ctx->gl_renderer);
		msg_write("GLSL: " + ctx->glsl_version);
	}

#ifdef OS_WINDOWS
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		msg_error((const char*)glewGetErrorString(err));
	}
#endif

	int num_extension = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_extension);
	for (int i = 0; i < num_extension; i++) {
		ctx->extensions.add((char *) glGetStringi(GL_EXTENSIONS, i));
	}


	// default values of the engine
	model_matrix = mat4::ID;
	view_matrix = mat4::ID;
	projection_matrix = mat4::ID;

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	glDebugMessageCallback(message_callback, nullptr);


	init_textures(ctx);
	init_shaders(ctx);
	init_vertex_buffers(ctx);

	set_front(Orientation::CW);
	set_cull(CullMode::BACK);
	set_wire(false);
	disable_alpha();
	set_material(White, 0.5f, 0, color(0.1f, 0.1f, 0.1f, 0.1f));
	set_projection_perspective();
	set_z(true, true);
	set_shader(ctx->default_3d.get());

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	ctx->default_framebuffer = new FrameBuffer(0, vp[2], vp[3]);

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

#if HAS_LIB_GLEW
	ctx->supports_mesh_shaders = glewIsSupported("GL_NV_mesh_shader");
#endif

	if (ctx->verbosity >= 2)
		msg_write("mesh shader support: " + str(ctx->supports_mesh_shaders));

	if (ctx->verbosity >= 1) {
		msg_ok();
		msg_left();
	}
	return ctx;
}

Context::~Context() {
	msg_write("nix.kill");
}

void kill(Context *ctx) {
	delete ctx;
}

void flush() {
	glFlush();
	glFinish();
}


static Array<unsigned int> time_queries;

void create_query_pool(int size) {
	time_queries.resize(size);
	glGenQueries(size, &time_queries[0]);
}

void query_timestamp(int index) {
	glQueryCounter(time_queries[index], GL_TIMESTAMP);
}


Array<int64> get_timestamps(int first, int count) {
	Array<int64> result;
	result.resize(count);
	for (int i=0; i<count; i++)
		glGetQueryObjecti64v(time_queries[first + i], GL_QUERY_RESULT, (GLint64*)&result[i]);
	return result;
}



#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

int Context::total_mem() const {
	if (sa_contains(extensions, "GL_NVX_gpu_memory_info")) {
		GLint total_mem_kb = 0;
		glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);
		int err = glGetError();
		if (err == GL_NO_ERROR)
			return total_mem_kb;
	}
	return -1;
}

int Context::available_mem() const {
	if (sa_contains(extensions, "GL_NVX_gpu_memory_info")) {
		GLint cur_avail_mem_kb = 0;
		glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);
		int err = glGetError();
		if (err == GL_NO_ERROR)
			return cur_avail_mem_kb;
	}
	return -1;
}



// shoot down windows
void KillWindows()
{
/*#ifdef OS_WINDOWS
	HANDLE t;
	OpenProcessToken(	GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&t);
	_TOKEN_PRIVILEGES tp;
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid); 
	tp.PrivilegeCount=1;
	tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(t,FALSE,&tp,0,NULL,0);
	InitiateSystemShutdown(NULL,(win_str)hui_tchar_str("Resistance is futile!"),10,TRUE,FALSE);
	//ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,0);
#endif*/
}



void set_wire(bool wire) {
	if (wire) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}
}

void set_cull(CullMode mode) {
	if (mode == CullMode::NONE) {
		glDisable(GL_CULL_FACE);
	} else if (mode == CullMode::FRONT) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} else if (mode == CullMode::BACK) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}

void set_front(Orientation front) {
	if (front == Orientation::CW)
		glFrontFace(GL_CW);
	else
		glFrontFace(GL_CCW);
}

void set_z(bool write, bool test) {
	if (test) {
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		if (write)
			glDepthMask(1);
		else
			glDepthMask(0);
	} else {
		if (write) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glDepthMask(1);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}
}

void set_offset(float offset) {
	if (fabs(offset) > 0.01f) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(offset, offset);
	} else {
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
	}
}

// deprecated...
void _set_alpha(AlphaMode mode) {
	//glDisable(GL_ALPHA_TEST);
	switch (mode) {
		case AlphaMode::NONE:
			glDisable(GL_BLEND);
			break;
		case AlphaMode::COLOR_KEY:
		case AlphaMode::COLOR_KEY_HARD:
		case AlphaMode::COLOR_KEY_SMOOTH:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			/*glEnable(GL_ALPHA_TEST);
			if (mode==AlphaColorKeyHard)
				glAlphaFunc(GL_GEQUAL,0.5f);
			else
				glAlphaFunc(GL_GEQUAL,0.04f);*/
			break;
		case AlphaMode::MATERIAL:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case AlphaMode::ADD:
			break;
	}
}

void _set_alpha_mode(AlphaMode mode) {
	_set_alpha(mode);
}

unsigned int OGLGetAlphaMode(Alpha mode) {
	if (mode == Alpha::ZERO)
		return GL_ZERO;
	if (mode == Alpha::ONE)
		return GL_ONE;
	if (mode == Alpha::SOURCE_COLOR)
		return GL_SRC_COLOR;
	if (mode == Alpha::SOURCE_INV_COLOR)
		return GL_ONE_MINUS_SRC_COLOR;
	if (mode == Alpha::SOURCE_ALPHA)
		return GL_SRC_ALPHA;
	if (mode == Alpha::SOURCE_INV_ALPHA)
		return GL_ONE_MINUS_SRC_ALPHA;
	if (mode == Alpha::DEST_COLOR)
		return GL_DST_COLOR;
	if (mode == Alpha::DEST_INV_COLOR)
		return GL_ONE_MINUS_DST_COLOR;
	if (mode == Alpha::DEST_ALPHA)
		return GL_DST_ALPHA;
	if (mode == Alpha::DEST_INV_ALPHA)
		return GL_ONE_MINUS_DST_ALPHA;
	// GL_SRC_ALPHA_SATURATE
	return GL_ZERO;
}

void set_alpha(Alpha src, Alpha dst) {
	glEnable(GL_BLEND);
	//glDisable(GL_ALPHA_TEST);
	glBlendFunc(OGLGetAlphaMode(src), OGLGetAlphaMode(dst));
}

void set_alpha_sd(Alpha src,Alpha dst) {
	set_alpha(src, dst);
}

void _cdecl set_alpha_split(Alpha color_src, Alpha color_dst, Alpha alpha_src, Alpha alpha_dst) {
	glEnable(GL_BLEND);
	glBlendFuncSeparate(OGLGetAlphaMode(color_src), OGLGetAlphaMode(color_dst), OGLGetAlphaMode(alpha_src), OGLGetAlphaMode(alpha_dst));
}

void disable_alpha() {
	glDisable(GL_BLEND);
	//glDisable(GL_ALPHA_TEST);
}


void set_stencil(StencilOp mode, unsigned long param) {
	glStencilMask(0xffffffff);
	
	if (mode == StencilOp::NONE) {
		glDisable(GL_STENCIL_TEST);
	} else if (mode == StencilOp::RESET) {
		glClearStencil(param);
		glClear(GL_STENCIL_BUFFER_BIT);
	} else if ((mode == StencilOp::INCREASE) or (mode == StencilOp::DECREASE) or (mode == StencilOp::DECREASE_NOT_NEGATIVE) or (mode == StencilOp::SET)) {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS,param,0xffffffff);
		if (mode == StencilOp::INCREASE)
			glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
		else if ((mode == StencilOp::DECREASE) or (mode == StencilOp::DECREASE_NOT_NEGATIVE))
			glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
		else if (mode == StencilOp::SET)
			glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
	} else if ((mode == StencilOp::MASK_EQUAL) or (mode == StencilOp::MASK_NOT_EQUAL) or (mode == StencilOp::MASK_LESS_EQUAL) or (mode == StencilOp::MASK_LESS) or (mode == StencilOp::MASK_GREATER_EQUAL) or (mode == StencilOp::MASK_GREATER)) {
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		if (mode == StencilOp::MASK_EQUAL)
			glStencilFunc(GL_EQUAL,param,0xffffffff);
		else if (mode == StencilOp::MASK_NOT_EQUAL)
			glStencilFunc(GL_NOTEQUAL,param,0xffffffff);
		else if (mode == StencilOp::MASK_LESS_EQUAL)
			glStencilFunc(GL_LEQUAL,param,0xffffffff);
		else if (mode == StencilOp::MASK_LESS)
			glStencilFunc(GL_LESS,param,0xffffffff);
		else if (mode == StencilOp::MASK_GREATER_EQUAL)
			glStencilFunc(GL_GEQUAL,param,0xffffffff);
		else if (mode == StencilOp::MASK_GREATER)
			glStencilFunc(GL_GREATER,param,0xffffffff);
	}
}

// mode=FogLinear:			start/end
// mode=FogExp/FogExp2:		density
void set_fog(FogMode mode,float start,float end,float density,const color &c) {
	fog.density = density;
	fog._color = c;
}

void enable_fog(bool Enabled)
{
	fog.density = 0;
}


};

#endif
