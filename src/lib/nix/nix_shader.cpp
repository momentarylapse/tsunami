/*----------------------------------------------------------------------------*\
| Nix shader                                                                   |
| -> shader files                                                              |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix{

string shader_dir;


Shader *default_shader_2d = NULL;
Shader *default_shader_3d = NULL;
Shader *current_shader = NULL;
Shader *override_shader = NULL;

static Array<Shader*> shaders;

int current_program = 0;
extern matrix projection_matrix2d;

string shader_error;



int create_empty_shader_program()
{
	int gl_p = glCreateProgram();
	TestGLError("CreateProgram");
	if (gl_p <= 0){
		msg_error("could not create gl shader program");
		return -1;
	}
	return gl_p;
}

struct ShaderSourcePart
{
	int type;
	string source;
};

Array<ShaderSourcePart> get_shader_parts(const string &source)
{
	Array<ShaderSourcePart> parts;
	int pos = 0;
	while (pos < source.num - 5){
		int pos0 = source.find("<", pos);
		if (pos0 < 0)
			break;
		pos = pos0 + 1;
		int pos1 = source.find(">", pos0);
		if (pos1 < 0)
			break;

		string tag = source.substr(pos0 + 1, pos1 - pos0 - 1);
		if ((tag.num > 64) or (tag.find("<") >= 0))
			continue;

		int pos2 = source.find("</" + tag + ">", pos1 + 1);
		if (pos2 < 0)
			continue;
		ShaderSourcePart p;
		p.source = source.substr(pos1 + 1, pos2 - pos1 - 1);
		pos = pos2 + tag.num + 3;
		if (tag == "VertexShader")
			p.type = GL_VERTEX_SHADER;
		else if (tag == "FragmentShader")
			p.type = GL_FRAGMENT_SHADER;
		else if (tag == "ComputeShader")
			p.type = GL_COMPUTE_SHADER;
		else if (tag == "TessComputeShader")
			p.type = GL_TESS_CONTROL_SHADER;
		else if (tag == "TessEvaluationShader")
			p.type = GL_TESS_EVALUATION_SHADER;
		else if (tag == "GeometryShader")
			p.type = GL_GEOMETRY_SHADER;
		else{
			msg_error("unknown shader tag: '" + tag + "'");
			continue;
		}
		parts.add(p);
	}
	return parts;
}

string get_inside_of_tag(const string &source, const string &tag)
{
	string r;
	int pos0 = source.find("<" + tag + ">");
	if (pos0 < 0)
		return "";
	pos0 += tag.num + 2;
	int pos1 = source.find("</" + tag + ">", pos0);
	if (pos1 < 0)
		return "";
	return source.substr(pos0, pos1 - pos0);
}

int create_gl_shader(const string &source, int type)
{
	if (source.num == 0)
		return -1;
	int gl_shader = glCreateShader(type);
	TestGLError("CreateShader create");
	if (gl_shader <= 0){
		shader_error = "could not create gl shader object";
		msg_error(shader_error);
		return -1;
	}
	
	const char *pbuf[2];

	pbuf[0] = source.c_str();
	glShaderSource(gl_shader, 1, pbuf, NULL);
	TestGLError("CreateShader source");

	glCompileShader(gl_shader);
	TestGLError("CreateShader compile");

	int status;
	glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &status);
	TestGLError("CreateShader status");
	//msg_write(status);
	if (status != GL_TRUE){
		shader_error.resize(16384);
		int size;
		glGetShaderInfoLog(gl_shader, shader_error.num, &size, (char*)shader_error.data);
		shader_error.resize(size);
		msg_error("while compiling shader: " + shader_error);
		return -1;
	}
	return gl_shader;
}

Shader *CreateShader(const string &source)
{
	auto parts = get_shader_parts(source);

	if (parts.num == 0){
		shader_error = "no shader tags found (<VertexShader>...</VertexShader> or <FragmentShader>...</FragmentShader>)";
		msg_error(shader_error);
		return NULL;
	}

	int prog = create_empty_shader_program();
	if (prog < 0)
		return NULL;

	Array<int> shaders;
	for (auto p: parts){
		int shader = create_gl_shader(p.source, p.type);
		if ((shader < 0) and (p.source.num > 0))
			return NULL;
		shaders.add(shader);
		if (shader >= 0)
			glAttachShader(prog, shader);
		TestGLError("AddShader attach");
	}

	int status;
	glLinkProgram(prog);
	TestGLError("AddShader link");
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	TestGLError("AddShader status");
	if (status != GL_TRUE){
		shader_error.resize(16384);
		int size;
		glGetProgramInfoLog(prog, shader_error.num, &size, (char*)shader_error.data);
		shader_error.resize(size);
		msg_error("while linking the shader program: " + shader_error);
		msg_left();
		return NULL;
	}

	for (int shader: shaders)
		glDeleteShader(shader);
	TestGLError("DeleteShader");

	Shader *s = new Shader;
	s->program = prog;
	shader_error = "";

	s->find_locations();

	TestGLError("CreateShader");
	return s;
}

void Shader::find_locations()
{
	location[LOCATION_MATRIX_MVP] = get_location("mat_mvp");
	location[LOCATION_MATRIX_M] = get_location("mat_m");
	location[LOCATION_MATRIX_V] = get_location("mat_v");
	location[LOCATION_MATRIX_P] = get_location("mat_p");
	location[LOCATION_MATRIX_P2D] = get_location("mat_p2d");
	for (int i=0; i<NIX_MAX_TEXTURELEVELS; i++)
		location[LOCATION_TEX + i] = get_location("tex" + i2s(i));
	location[LOCATION_TEX_CUBE] = get_location("tex_cube");
	location[LOCATION_CAM_POS] = get_location("CameraPosition");
	location[LOCATION_CAM_DIR] = get_location("CameraDirection");

	location[LOCATION_MATERIAL_AMBIENT] = get_location("material.ambient");
	location[LOCATION_MATERIAL_DIFFUSIVE] = get_location("material.diffusive");
	location[LOCATION_MATERIAL_SPECULAR] = get_location("material.specular");
	location[LOCATION_MATERIAL_SHININESS] = get_location("material.shininess");
	location[LOCATION_MATERIAL_EMISSION] = get_location("material.emission");

	location[LOCATION_LIGHT_COLOR] = get_location("light.color");
	location[LOCATION_LIGHT_AMBIENT] = get_location("light.ambient");
	location[LOCATION_LIGHT_SPECULAR] = get_location("light.specular");
	location[LOCATION_LIGHT_POS] = get_location("light.pos");
	location[LOCATION_LIGHT_RADIUS] = get_location("light.radius");

	location[LOCATION_FOG_COLOR] = get_location("fog.color");
	location[LOCATION_FOG_DENSITY] = get_location("fog.density");
}

Shader *LoadShader(const string &filename)
{
	if (filename.num == 0){
		default_shader_3d->reference_count ++;
		return default_shader_3d;
	}

	string fn = shader_dir + filename;
	for (Shader *s: shaders)
		if ((s->filename == fn) and (s->program >= 0)){
			s->reference_count ++;
			return s;
		}

	msg_write("loading shader: " + fn);
	msg_right();

	try{
	string source = FileRead(fn);
	Shader *shader = CreateShader(source);
	if (shader)
		shader->filename = fn;

	msg_left();
	return shader;
	}catch(FileError &e){
		msg_error(e.message());
		default_shader_3d->reference_count ++;
		return default_shader_3d;
	}
}

Shader::Shader()
{
	shaders.add(this);
	reference_count = 1;
	filename = "-no file-";
	program = -1;
	for (int i=0; i<NUM_LOCATIONS; i++)
		location[i] = -1;
}

Shader::~Shader()
{
	msg_write("delete shader: " + filename);
	glDeleteProgram(program);
	TestGLError("NixUnrefShader");
	program = -1;
	filename = "";
}

void Shader::unref()
{
	reference_count --;
	if ((reference_count <= 0) and (program >= 0)){
		if ((this == default_shader_3d) or (this == default_shader_2d))
			return;
		glDeleteProgram(program);
		TestGLError("NixUnrefShader");
		program = -1;
		filename = "";
	}
}

void DeleteAllShaders()
{
	return;
	for (Shader *s: shaders)
		delete(s);
	shaders.clear();
	init_shaders();
}

void SetShader(Shader *s)
{
	if (override_shader)
		s = override_shader;
	if (s == NULL)
		s = default_shader_3d;
	current_shader = s;
	current_program = s->program;
	glUseProgram(current_program);
	TestGLError("SetProgram");

	//s->set_default_data();
}

void SetOverrideShader(Shader *s)
{
	override_shader = s;
}

int Shader::get_location(const string &var_name)
{
	int loc = glGetUniformLocation(program, var_name.c_str());
	return loc;
}

void Shader::set_data(int location, const float *data, int size)
{
	if (location < 0)
		return;
	//NixSetShader(this);
	if (size == sizeof(float)){
		glUniform1f(location, *data);
	}else if (size == sizeof(float)*3){
		glUniform3fv(location, 1, data);
	}else if (size == sizeof(float)*4){
		glUniform4fv(location, 1, data);
	}
	TestGLError("SetShaderData");
}

void Shader::set_int(int location, int i)
{
	if (location < 0)
		return;
	glUniform1i(location, i);
	TestGLError("SetShaderInt");
}

void Shader::set_float(int location, float f)
{
	if (location < 0)
		return;
	glUniform1f(location, f);
	TestGLError("SetShaderFloat");
}

void Shader::set_color(int location, const color &c)
{
	if (location < 0)
		return;
	glUniform4fv(location, 1, (float*)&c);
	TestGLError("SetShaderData");
}

void Shader::set_matrix(int location, const matrix &m)
{
	if (location < 0)
		return;
	glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&m);
	TestGLError("SetShaderData");
}

void Shader::get_data(const string &var_name, void *data, int size)
{
	msg_todo("NixGetShaderData for OpenGL");
}

void Shader::set_default_data()
{
	set_matrix(location[LOCATION_MATRIX_MVP], world_view_projection_matrix);
	set_matrix(location[LOCATION_MATRIX_M], world_matrix);
	set_matrix(location[LOCATION_MATRIX_V], view_matrix);
	set_matrix(location[LOCATION_MATRIX_P], projection_matrix);
	set_matrix(location[LOCATION_MATRIX_P2D], projection_matrix2d);
	for (int i=0; i<NIX_MAX_TEXTURELEVELS; i++)
		set_int(location[LOCATION_TEX + i], i);
	if (tex_cube_level >= 0)
		set_int(location[LOCATION_TEX_CUBE], tex_cube_level);
	set_color(location[LOCATION_MATERIAL_AMBIENT], material.ambient);
	set_color(location[LOCATION_MATERIAL_DIFFUSIVE], material.diffusive);
	set_color(location[LOCATION_MATERIAL_SPECULAR], material.specular);
	set_data(location[LOCATION_MATERIAL_SHININESS], &material.shininess, 4);
	set_color(location[LOCATION_MATERIAL_EMISSION], material.emission);

	set_data(location[LOCATION_LIGHT_AMBIENT], &lights[0].ambient, 4);
	set_color(location[LOCATION_LIGHT_COLOR], lights[0].diffusive);
	set_data(location[LOCATION_LIGHT_SPECULAR], &lights[0].specular, 4);
	vector dir = view_matrix.transform_normal(lights[0].pos);
	set_data(location[LOCATION_LIGHT_POS], &dir.x, 3*4);
	set_data(location[LOCATION_LIGHT_RADIUS], &lights[0].radius, 4);

	set_color(location[LOCATION_FOG_COLOR], fog._color);
	set_data(location[LOCATION_FOG_DENSITY], &fog.density, 4);
}

void Shader::dispatch(int nx, int ny, int nz) {
	glUseProgram(program);
	glDispatchCompute(nx, ny, nz);
	
	TestGLError("Shader.dispatch");
}


void init_shaders()
{

	default_shader_3d = nix::CreateShader(
		"<VertexShader>\n"
		"#version 330 core\n"
		"uniform mat4 mat_mvp;\n"
		"uniform mat4 mat_m;\n"
		"uniform mat4 mat_v;\n"
		"layout(location = 0) in vec3 inPosition;\n"
		"layout(location = 1) in vec3 inNormal;\n"
		"layout(location = 2) in vec2 inTexCoord;\n"
		"out vec3 fragmentNormal;\n"
		"out vec2 fragmentTexCoord;\n"
		"out vec3 fragmentPos; // camera space\n"
		"void main(){\n"
		"	gl_Position = mat_mvp * vec4(inPosition,1);\n"
		"	fragmentNormal = (mat_v * mat_m * vec4(inNormal,0)).xyz;\n"
		"	fragmentTexCoord = inTexCoord;\n"
		"	fragmentPos = (mat_v * mat_m * vec4(inPosition,1)).xyz;\n"
		"}\n"
		"</VertexShader>\n"
		"<FragmentShader>\n"
		"#version 330 core\n"
		"struct Fog{ vec4 color; float density; };\n"
		"struct Material{ vec4 ambient, diffusive, specular, emission; float shininess; };\n"
		"struct Light{ vec4 color; vec3 pos; float radius, ambient, specular; };\n"
		"uniform Material material;\n"
		"uniform Light light;\n"
		"uniform Fog fog;\n"
		"in vec3 fragmentNormal;\n"
		"in vec2 fragmentTexCoord;\n"
		"in vec3 fragmentPos;\n"
		"uniform sampler2D tex0;\n"
		"out vec4 color;\n"
		"void main(){\n"
		"	vec3 n = normalize(fragmentNormal);\n"
		"	vec3 l = light.pos;\n"
		"	float d = max(-dot(n, l), 0);\n"
		"	color = material.emission;\n"
		"	color += material.ambient * light.color * light.ambient;\n"
		"	color += material.diffusive * light.color * d;\n"
		"	vec4 tex_col = texture(tex0, fragmentTexCoord);\n"
		"	color *= tex_col;\n"
		"	if ((d > 0)&&(material.shininess > 1)){\n"
		"		vec3 e = normalize(fragmentPos); // eye dir\n"
		"		vec3 rl = reflect(l, n);\n"
		"		float ee = max(-dot(e, rl), 0);\n"
		"		color += material.specular * light.color * light.specular * pow(ee, material.shininess);\n"
		"	}\n"
		"	float t = exp(-fragmentPos.z * fog.density);\n"
		"	color = (1 - t) * fog.color + t * color;\n"
		"	color.a = material.diffusive.a * tex_col.a;\n"
		"}\n"
		"</FragmentShader>");



	default_shader_2d = nix::CreateShader(
		"<VertexShader>\n"
		"#version 330 core\n"
		"\n"
		"uniform mat4 mat_p2d;\n"
		"\n"
		"layout(location = 0) in vec3 inPosition;\n"
		"layout(location = 2) in vec2 inTexCoord;\n"
		"\n"
		"out vec2 fragmentTexCoord;\n"
		"\n"
		"void main(){\n"
		"	gl_Position = mat_p2d * vec4(inPosition,1);\n"
		"	fragmentTexCoord = inTexCoord;\n"
		"}\n"
		"\n"
		"</VertexShader>\n"
		"<FragmentShader>\n"
		"#version 330 core\n"
		"struct Material{ vec4 ambient, diffusive, specular, emission; float shininess; };\n"
		"uniform Material material;\n"
		"in vec2 fragmentTexCoord;\n"
		"uniform sampler2D tex0;\n"
		"out vec4 color;\n"
		"void main(){\n"
		"	color = texture(tex0, fragmentTexCoord);\n"
		"	color *= material.emission;\n"
		"}\n"
		"</FragmentShader>");

	default_shader_3d->reference_count ++;
	default_shader_2d->reference_count ++;
}

};


#endif

