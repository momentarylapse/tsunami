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

#include "nix_common.h"


extern unsigned int VertexArrayID;

namespace nix{

string version = "0.13.4.0";


// libraries (in case Visual C++ is used)
//#pragma comment(lib,"opengl32.lib")
//#pragma comment(lib,"glu32.lib")
//#pragma comment(lib,"glut32.lib")
//#pragma comment(lib,"glaux.lib ")
//#ifdef NIX_ALLOW_VIDEO_TEXTURE
//	#pragma comment(lib,"strmiids.lib")
//	#pragma comment(lib,"vfw32.lib")
//#endif



/*
libraries to link:

-lwinmm
-lcomctl32
-lkernel32
-luser32
-lgdi32
-lwinspool
-lcomdlg32
-ladvapi32
-lshell32
-lole32
-loleaut32
-luuid
-lodbc32
-lodbccp32
-lwinmm
-lgdi32
-lwsock32
-lopengl32
-lglu32
-lvfw_avi32
-lvfw_ms32
*/


void TestGLError(const char *pos) {
#if 1
	int err = glGetError();
	if (err == GL_NO_ERROR)
	{}//msg_write("GL_NO_ERROR");
	else if (err == GL_INVALID_ENUM)
		msg_error("GL_INVALID_ENUM at " + string(pos));
	else if (err == GL_INVALID_VALUE)
		msg_error("GL_INVALID_VALUE at " + string(pos));
	else if (err == GL_INVALID_OPERATION)
		msg_error("GL_INVALID_OPERATION at " + string(pos));
	else if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
		msg_error("GL_INVALID_FRAMEBUFFER_OPERATION at " + string(pos));
	else
		msg_error(i2s(err) + " at " + string(pos));
#endif
}

// things'n'stuff
int FontGlyphWidth[256];


//int device_width, device_height;
int target_width, target_height;						// render target size (window/texture)
rect target_rect;
bool Fullscreen;
callback_function *RefillAllVertexBuffers = NULL;

int MaxVideoTextureSize=256;

bool CullingInverted;

Fog fog;




//#define ENABLE_INDEX_BUFFERS

// shader files
int glShaderCurrent = 0;


void MatrixOut(matrix &m) {
	msg_write("MatrixOut");
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._00,m._01,m._02,m._03));
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._10,m._11,m._12,m._13));
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._20,m._21,m._22,m._23));
	msg_write(format("	%f:2	%f:2	%f:2	%f:2",m._30,m._31,m._32,m._33));
}

void mout(matrix &m) {
	msg_write(format("		%f	%f	%f	%f",m._00,m._01,m._02,m._03));
	msg_write(format("		%f	%f	%f	%f",m._10,m._11,m._12,m._13));
	msg_write(format("		%f	%f	%f	%f",m._20,m._21,m._22,m._23));
	msg_write(format("		%f	%f	%f	%f",m._30,m._31,m._32,m._33));
}

void Init() {
	//if (Usable)
	//	return;

	msg_write("Nix");
	msg_right();
	msg_write("[" + version + "]");

	msg_write(string("OpenGL: ") + (char*)glGetString(GL_VERSION));
	msg_write(string("Renderer: ") + (char*)glGetString(GL_RENDERER));
	msg_write(string("GLSL: ") + (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	

	Fullscreen = false; // before nix is started, we're hopefully not in fullscreen mode

	// reset data
	RefillAllVertexBuffers = NULL;


	// default values of the engine
	view_matrix = matrix::ID;
	projection_matrix = matrix::ID;
	CullingInverted = false;



	init_textures();
	init_shaders();
	init_vertex_buffers();

	SetCull(CULL_DEFAULT);
	SetWire(false);
	SetAlpha(ALPHA_NONE);
	SetMaterial(White, White, White, 0, color(0.1f, 0.1f, 0.1f, 0.1f));
	CullingInverted = false;
	SetProjectionPerspective();
	SetZ(true, true);
	SetShader(default_shader_3d);

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	FrameBuffer::DEFAULT->width = vp[2];
	FrameBuffer::DEFAULT->height = vp[3];


	msg_ok();
	msg_left();
}

void Kill() {
	msg_write("nix.kill");
	KillDeviceObjects();
	glDeleteVertexArrays(1, &VertexArrayID);
}

// erlaubt dem Device einen Neustart
void KillDeviceObjects()
{
	// textures
	ReleaseTextures();
}

void ReincarnateDeviceObjects()
{
	// textures
	ReincarnateTextures();
	if (RefillAllVertexBuffers)
		RefillAllVertexBuffers();
}




// shoot down windows
void KillWindows()
{
#ifdef OS_WINDOWS
	HANDLE t;
	OpenProcessToken(	GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&t);
	_TOKEN_PRIVILEGES tp;
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid); 
	tp.PrivilegeCount=1;
	tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(t,FALSE,&tp,0,NULL,0);
	InitiateSystemShutdown(NULL,(win_str)hui_tchar_str("Resistance is futile!"),10,TRUE,FALSE);
	//ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,0);
#endif
}



void SetWire(bool wire) {
	if (wire) {
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glDisable(GL_CULL_FACE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		glEnable(GL_CULL_FACE);
	}
	TestGLError("SetWire");
}

void SetCull(int mode) {
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	if (mode == CULL_NONE)
		glDisable(GL_CULL_FACE);
	if (mode == CULL_CCW)
		glCullFace(GL_FRONT);
	if (mode == CULL_CW)
		glCullFace(GL_BACK);
	TestGLError("SetCull");
}

void SetZ(bool write, bool test) {
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
	TestGLError("SetZ");
}

void SetOffset(float offset) {
	if (fabs(offset) > 0.01f) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(offset, offset);
	} else {
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
	}
}

void SetAlpha(int mode)
{
	//glDisable(GL_ALPHA_TEST);
	switch (mode){
		case ALPHA_NONE:
			glDisable(GL_BLEND);
			TestGLError("SetAlpha b");
			break;
		case ALPHA_COLOR_KEY_HARD:
		case ALPHA_COLOR_KEY_SMOOTH:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			/*glEnable(GL_ALPHA_TEST);
			if (mode==AlphaColorKeyHard)
				glAlphaFunc(GL_GEQUAL,0.5f);
			else
				glAlphaFunc(GL_GEQUAL,0.04f);*/
			break;
		case ALPHA_MATERIAL:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
	}
	TestGLError("SetAlpha");
}

void SetAlphaM(int mode)
{	SetAlpha(mode);	}

unsigned int OGLGetAlphaMode(int mode) {
	if (mode == ALPHA_ZERO)
		return GL_ZERO;
	if (mode == ALPHA_ONE)
		return GL_ONE;
	if (mode == ALPHA_SOURCE_COLOR)
		return GL_SRC_COLOR;
	if (mode == ALPHA_SOURCE_INV_COLOR)
		return GL_ONE_MINUS_SRC_COLOR;
	if (mode == ALPHA_SOURCE_ALPHA)
		return GL_SRC_ALPHA;
	if (mode == ALPHA_SOURCE_INV_ALPHA)
		return GL_ONE_MINUS_SRC_ALPHA;
	if (mode == ALPHA_DEST_COLOR)
		return GL_DST_COLOR;
	if (mode == ALPHA_DEST_INV_COLOR)
		return GL_ONE_MINUS_DST_COLOR;
	if (mode == ALPHA_DEST_ALPHA)
		return GL_DST_ALPHA;
	if (mode == ALPHA_DEST_INV_ALPHA)
		return GL_ONE_MINUS_DST_ALPHA;
	// GL_SRC_ALPHA_SATURATE
	return GL_ZERO;
}

void SetAlpha(int src,int dst) {
	glEnable(GL_BLEND);
	//glDisable(GL_ALPHA_TEST);
	glBlendFunc(OGLGetAlphaMode(src),OGLGetAlphaMode(dst));
	TestGLError("SetAlphaII");
}

void SetAlphaSD(int src,int dst)
{	SetAlpha(src,dst);	}

void SetAlpha(float factor) {
	msg_error("deprecated... SetAlpha(factor)");
	/*glDisable(GL_ALPHA_TEST);
	float di[4];
	glGetMaterialfv(GL_FRONT,GL_DIFFUSE,di);
	di[3]=factor;
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,di);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	TestGLError("SetAlphaF");*/
}

void SetStencil(int mode, unsigned long param)
{
	glStencilMask(0xffffffff);
	
	if (mode == STENCIL_NONE){
		glDisable(GL_STENCIL_TEST);
	}else if (mode == STENCIL_RESET){
		glClearStencil(param);
		glClear(GL_STENCIL_BUFFER_BIT);
	}else if ((mode == STENCIL_INCREASE) or (mode == STENCIL_DECREASE) or (mode == STENCIL_DECREASE_NOT_NEGATIVE) or (mode == STENCIL_SET)){
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS,param,0xffffffff);
		if (mode == STENCIL_INCREASE)
			glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
		else if ((mode == STENCIL_DECREASE) or (mode == STENCIL_DECREASE_NOT_NEGATIVE))
			glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
		else if (mode == STENCIL_SET)
			glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
	}else if ((mode == STENCIL_MASK_EQUAL) or (mode == STENCIL_MASK_NOT_EQUAL) or (mode == STENCIL_MASK_LESS_EQUAL) or (mode == STENCIL_MASK_LESS) or (mode == STENCIL_MASK_GREATER_EQUAL) or (mode == STENCIL_MASK_GREATER)){
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		if (mode == STENCIL_MASK_EQUAL)
			glStencilFunc(GL_EQUAL,param,0xffffffff);
		else if (mode == STENCIL_MASK_NOT_EQUAL)
			glStencilFunc(GL_NOTEQUAL,param,0xffffffff);
		else if (mode == STENCIL_MASK_LESS_EQUAL)
			glStencilFunc(GL_LEQUAL,param,0xffffffff);
		else if (mode == STENCIL_MASK_LESS)
			glStencilFunc(GL_LESS,param,0xffffffff);
		else if (mode == STENCIL_MASK_GREATER_EQUAL)
			glStencilFunc(GL_GEQUAL,param,0xffffffff);
		else if (mode == STENCIL_MASK_GREATER)
			glStencilFunc(GL_GREATER,param,0xffffffff);
	}
	TestGLError("SetStencil");
}

// mode=FogLinear:			start/end
// mode=FogExp/FogExp2:		density
void SetFog(int mode,float start,float end,float density,const color &c)
{
	fog.density = density;
	fog._color = c;
	//TestGLError("SetFog");
}

void EnableFog(bool Enabled)
{
	fog.density = 0;
}


};

#endif
