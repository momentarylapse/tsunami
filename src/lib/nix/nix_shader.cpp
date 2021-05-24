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

Path shader_dir;

const int TYPE_LAYOUT = -41;
const int TYPE_MODULE = -42;


Shader *Shader::default_2d = nullptr;
Shader *Shader::default_3d = nullptr;
Shader *Shader::_current_ = nullptr;
Shader *Shader::default_load = nullptr;

static shared_array<Shader> shaders;

int current_program = 0;

string shader_error;


UniformBuffer::UniformBuffer() {
	glGenBuffers(1, &buffer);
}

UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &buffer);
}

void UniformBuffer::__init__() {
	new(this) UniformBuffer();
}

void UniformBuffer::__delete__() {
	this->~UniformBuffer();
}

void UniformBuffer::update(void *data, int size) {
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void UniformBuffer::update_array(const DynamicArray &a) {
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, a.num * a.element_size, a.data, GL_DYNAMIC_DRAW);
}

void BindUniform(UniformBuffer *ub, int index) {
	//glUniformBlockBinding(program, index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, ub->buffer);
}


int create_empty_shader_program() {
	int gl_p = glCreateProgram();
	TestGLError("CreateProgram");
	if (gl_p <= 0)
		throw Exception("could not create gl shader program");
	return gl_p;
}

struct ShaderSourcePart {
	int type;
	string source;
};

struct ShaderMetaData {
	string version, name;
};

struct ShaderModule {
	ShaderMetaData meta;
	string source;
};
static Array<ShaderModule> shader_modules;

Array<ShaderSourcePart> get_shader_parts(const string &source) {
	Array<ShaderSourcePart> parts;
	int pos = 0;
	while (pos < source.num - 5) {
		int pos0 = source.find("<", pos);
		if (pos0 < 0)
			break;
		pos = pos0 + 1;
		int pos1 = source.find(">", pos0);
		if (pos1 < 0)
			break;

		string tag = source.sub(pos0 + 1, pos1);
		if ((tag.num > 64) or (tag.find("<") >= 0))
			continue;

		int pos2 = source.find("</" + tag + ">", pos1 + 1);
		if (pos2 < 0)
			continue;
		ShaderSourcePart p;
		p.source = source.sub(pos1 + 1, pos2);
		pos = pos2 + tag.num + 3;
		if (tag == "VertexShader") {
			p.type = GL_VERTEX_SHADER;
		} else if (tag == "FragmentShader") {
			p.type = GL_FRAGMENT_SHADER;
		} else if (tag == "ComputeShader") {
			p.type = GL_COMPUTE_SHADER;
		} else if (tag == "TessControlShader") {
			p.type = GL_TESS_CONTROL_SHADER;
		} else if (tag == "TessEvaluationShader") {
			p.type = GL_TESS_EVALUATION_SHADER;
		} else if (tag == "GeometryShader") {
			p.type = GL_GEOMETRY_SHADER;
		} else if (tag == "Module") {
			p.type = TYPE_MODULE;
		} else if (tag == "Layout") {
			p.type = TYPE_LAYOUT;
		} else {
			msg_error("unknown shader tag: '" + tag + "'");
			continue;
		}
		parts.add(p);
	}
	return parts;
}

string get_inside_of_tag(const string &source, const string &tag) {
	string r;
	int pos0 = source.find("<" + tag + ">");
	if (pos0 < 0)
		return "";
	pos0 += tag.num + 2;
	int pos1 = source.find("</" + tag + ">", pos0);
	if (pos1 < 0)
		return "";
	return source.sub(pos0, pos1);
}

string expand_shader_source(const string &source, ShaderMetaData &meta) {
	string r = source;
	while (true) {
		int p = r.find("#import", 0);
		if (p < 0)
			break;
		int p2 = r.find("\n", p);
		string imp = r.sub(p + 7, p2).replace(" ", "");
		//msg_error("import '" + imp + "'");

		bool found = false;
		for (auto &m: shader_modules)
			if (m.meta.name == imp) {
				//msg_error("FOUND");
				r = r.head(p) + "\n// <<\n" + m.source + "\n// >>\n" + r.sub(p2);
				found = true;
			}
		if (!found)
			throw Exception(format("shader import '%s' not found", imp));
	}

	string intro;
	if (meta.version != "")
		intro += "#version " + meta.version + "\n";
	if (r.find("GL_ARB_separate_shader_objects", 0) < 0)
		intro += "#extension GL_ARB_separate_shader_objects : enable\n";
	return intro + r;
}

int create_gl_shader(const string &_source, int type, ShaderMetaData &meta) {
	string source = expand_shader_source(_source, meta);
	if (source.num == 0)
		return -1;
	int gl_shader = glCreateShader(type);
	TestGLError("CreateShader create");
	if (gl_shader <= 0)
		throw Exception("could not create gl shader object");
	
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
	if (status != GL_TRUE) {
		shader_error.resize(16384);
		int size;
		glGetShaderInfoLog(gl_shader, shader_error.num, &size, (char*)shader_error.data);
		shader_error.resize(size);
		throw Exception("while compiling shader: " + shader_error);
	}
	return gl_shader;
}

ShaderMetaData parse_meta(string source) {
	ShaderMetaData m;
	for (auto &x: source.explode("\n")) {
		auto y = x.explode("=");
		if (y.num == 2) {
			string k = y[0].trim();
			string v = y[1].trim();
			if (k == "name")
				m.name = v;
			else if (k == "version")
				m.version = v;
		}
	}
	return m;
}

void Shader::update(const string &source) {
	auto parts = get_shader_parts(source);

	if (parts.num == 0)
		throw Exception("no shader tags found (<VertexShader>...</VertexShader> or <FragmentShader>...</FragmentShader>)");

	int prog = create_empty_shader_program();

	Array<int> shaders;
	ShaderMetaData meta;
	for (auto p: parts) {
		if (p.type == TYPE_MODULE) {
			ShaderModule m;
			m.source = p.source;
			m.meta = meta;
			shader_modules.add(m);
			msg_write("new module '" + m.meta.name + "'");
			return;
		} else if (p.type == TYPE_LAYOUT) {
			meta = parse_meta(p.source);
		} else {
			int shader = create_gl_shader(p.source, p.type, meta);
			shaders.add(shader);
			if (shader >= 0)
				glAttachShader(prog, shader);
			TestGLError("AddShader attach");
		}
	}

	int status;
	glLinkProgram(prog);
	TestGLError("AddShader link");
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	TestGLError("AddShader status");
	if (status != GL_TRUE) {
		shader_error.resize(16384);
		int size;
		glGetProgramInfoLog(prog, shader_error.num, &size, (char*)shader_error.data);
		shader_error.resize(size);
		throw Exception("while linking the shader program: " + shader_error);
	}

	for (int shader: shaders)
		glDeleteShader(shader);
	TestGLError("DeleteShader");

	program = prog;
	shader_error = "";

	find_locations();

	TestGLError("CreateShader");
}
Shader *Shader::create(const string &source) {
	shared<Shader> s = new Shader;
	s->update(source);
	shaders.add(s);
	return s.get();
}

void Shader::find_locations() {
	location[LOCATION_MATRIX_M] = get_location("matrix.model");
	location[LOCATION_MATRIX_V] = get_location("matrix.view");
	location[LOCATION_MATRIX_P] = get_location("matrix.project");

	if (location[LOCATION_MATRIX_M] < 0 and location[LOCATION_MATRIX_V] < 0 and location[LOCATION_MATRIX_P] < 0) {
		location[LOCATION_MATRIX_MVP] = get_location("mat_mvp");
		location[LOCATION_MATRIX_M] = get_location("mat_m");
		location[LOCATION_MATRIX_V] = get_location("mat_v");
		location[LOCATION_MATRIX_P] = get_location("mat_p");
		if (location[LOCATION_MATRIX_M] >= 0 or location[LOCATION_MATRIX_V] >= 0 or location[LOCATION_MATRIX_P] >= 0)
			msg_error("deprecated matrix linking: " + filename.str());
	}
	for (int i=0; i<NIX_MAX_TEXTURELEVELS; i++)
		location[LOCATION_TEX + i] = get_location("tex" + i2s(i));
	location[LOCATION_TEX_CUBE] = get_location("tex_cube");

	location[LOCATION_MATERIAL_ALBEDO] = get_location("material.albedo");
	location[LOCATION_MATERIAL_ROUGHNESS] = get_location("material.roughness");
	location[LOCATION_MATERIAL_METAL] = get_location("material.metal");
	location[LOCATION_MATERIAL_EMISSION] = get_location("material.emission");

	link_uniform_block("Matrix", 0);
	link_uniform_block("LightData", 1);
	link_uniform_block("Material", 2);
	link_uniform_block("Fog", 3);
}

Shader *Shader::load(const Path &filename) {
	if (filename.is_empty())
		return default_load;

	Path fn = shader_dir << filename;
	if (filename.is_absolute())
		fn = filename;
	for (Shader *s: weak(shaders))
		if ((s->filename == fn) and (s->program >= 0))
			return s;

	msg_write("loading shader: " + fn.str());

	try {
		string source = FileRead(fn);
		Shader *shader = Shader::create(source);
		if (shader)
			shader->filename = fn;

		return shader;
	} catch (Exception &e) {
		msg_error(e.message());
		return default_load;
	}
}

Shader::Shader() {
	filename = "-no file-";
	program = -1;
	for (int i=0; i<NUM_LOCATIONS; i++)
		location[i] = -1;
}

Shader::~Shader() {
	msg_write("delete shader: " + filename.str());
	if (program >= 0)
		glDeleteProgram(program);
	TestGLError("glDeleteProgram");
	program = -1;
}

void DeleteAllShaders() {
	return;
	shaders.clear();
	init_shaders();
}

void SetShader(Shader *s) {
	if (s == nullptr)
		s = Shader::default_3d;
	Shader::_current_ = s;
	current_program = s->program;
	glUseProgram(current_program);
	TestGLError("SetProgram");

	//s->set_default_data();
}

int Shader::get_location(const string &name) {
	return glGetUniformLocation(program, name.c_str());
}

bool Shader::link_uniform_block(const string &name, int binding) {
	int index = glGetUniformBlockIndex(program, name.c_str());
	if (index < 0)
		return false;
	glUniformBlockBinding(program, index, binding);
	return true;
}

void Shader::set_data(int location, const float *data, int size) {
	if (location < 0)
		return;
	//NixSetShader(this);
	if (size == sizeof(float)) {
		glUniform1f(location, *data);
	} else if (size == sizeof(float)*2) {
		glUniform2fv(location, 1, data);
	} else if (size == sizeof(float)*3) {
		glUniform3fv(location, 1, data);
	} else if (size == sizeof(float)*4) {
		glUniform4fv(location, 1, data);
	} else if (size == sizeof(float)*16) {
		glUniformMatrix4fv(location, 1, GL_FALSE, (float*)data);
	}
	TestGLError("SetShaderData");
}

void Shader::set_int(int location, int i) {
	if (location < 0)
		return;
	glUniform1i(location, i);
	TestGLError("SetShaderInt");
}

void Shader::set_float(int location, float f) {
	if (location < 0)
		return;
	glUniform1f(location, f);
	TestGLError("SetShaderFloat");
}

void Shader::set_color(int location, const color &c) {
	if (location < 0)
		return;
	glUniform4fv(location, 1, (float*)&c);
	TestGLError("SetShaderData");
}

void Shader::set_matrix(int location, const matrix &m) {
	if (location < 0)
		return;
	glUniformMatrix4fv(location, 1, GL_FALSE, (float*)&m);
	TestGLError("SetShaderData");
}

void Shader::set_default_data() {
	set_matrix(location[LOCATION_MATRIX_MVP], world_view_projection_matrix);
	set_matrix(location[LOCATION_MATRIX_M], world_matrix);
	set_matrix(location[LOCATION_MATRIX_V], view_matrix);
	set_matrix(location[LOCATION_MATRIX_P], projection_matrix);
	for (int i=0; i<NIX_MAX_TEXTURELEVELS; i++)
		set_int(location[LOCATION_TEX + i], i);
	if (tex_cube_level >= 0)
		set_int(location[LOCATION_TEX_CUBE], tex_cube_level);
	set_color(location[LOCATION_MATERIAL_ALBEDO], material.albedo);
	set_float(location[LOCATION_MATERIAL_ROUGHNESS], material.roughness);
	set_float(location[LOCATION_MATERIAL_METAL], material.metal);
	set_color(location[LOCATION_MATERIAL_EMISSION], material.emission);
}

void Shader::dispatch(int nx, int ny, int nz) {
	glUseProgram(program);
	glDispatchCompute(nx, ny, nz);
	
	TestGLError("Shader.dispatch");
}


void init_shaders() {
	try {

	Shader::default_3d = nix::Shader::create(
R"foodelim(<VertexShader>
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_pos; // camera space
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
	out_pos = (matrix.view * matrix.model * vec4(in_position, 1)).xyz;
}
</VertexShader>
<FragmentShader>
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;
struct Material { vec4 albedo, emission; float roughness, metal; };
/*layout(binding = 2)*/ uniform Material material;
struct Light { mat4 proj; vec4 pos, dir, color; float radius, theta, harshness; };
uniform int num_lights = 0;
/*layout(binding = 1)*/ uniform LightData { Light light[32]; };

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
uniform sampler2D tex0;
out vec4 out_color;

vec4 basic_lighting(Light l, vec3 n, vec4 tex_col) {
	float attenuation = 1.0;
	vec3 LD = (matrix.view * vec4(l.dir.xyz, 0)).xyz;
	vec3 LP = (matrix.view * vec4(l.pos.xyz, 1)).xyz;
	if (l.radius > 0) {
		LD = normalize(in_pos - LP);
		attenuation = min(l.radius / length(in_pos - LP), 1);
	}
	float d = max(-dot(n, LD), 0) * attenuation;
	vec4 color = material.albedo * material.roughness * l.color * (1 - l.harshness) / 2;
	color += material.albedo * l.color * l.harshness * d;
	color *= tex_col;
	if ((d > 0) && (material.roughness < 0.8)) {
		vec3 e = normalize(in_pos); // eye dir
		vec3 rl = reflect(LD, n);
		float ee = max(-dot(e, rl), 0);
		float shininess = 5 / (1.1 - material.roughness);
		color += (1 - material.roughness) * l.color * l.harshness * pow(ee, shininess);
	}
	return color;
}

void main() {
	vec3 n = normalize(in_normal);
	out_color = material.emission;
	vec4 tex_col = texture(tex0, in_uv);
	for (int i=0; i<num_lights; i++)
		out_color += basic_lighting(light[i], n, tex_col);
	out_color.a = material.albedo.a * tex_col.a;
}
</FragmentShader>)foodelim");
	Shader::default_3d->filename = "-default 3d-";



	Shader::default_2d = nix::Shader::create(
R"foodelim(<VertexShader>
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_uv = in_uv;
	out_color = in_color;
}

</VertexShader>
<FragmentShader>
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;
uniform sampler2D tex0;
out vec4 color;

void main() {
	color = texture(tex0, in_uv);
	color *= in_color;
}
</FragmentShader>)foodelim");
		Shader::default_2d->filename = "-default 2d-";
	} catch(Exception &e) {
		msg_error(e.message());
		throw e;
	}
	Shader::default_load = Shader::default_3d;
}

};


#endif

