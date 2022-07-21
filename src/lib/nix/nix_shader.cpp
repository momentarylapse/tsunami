/*----------------------------------------------------------------------------*\
| Nix shader                                                                   |
| -> shader files                                                              |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"
#include "../os/file.h"
#include "../os/msg.h"

namespace nix {

const int TYPE_LAYOUT = -41;
const int TYPE_MODULE = -42;


Shader *Shader::default_2d = nullptr;
Shader *Shader::default_3d = nullptr;
Shader *Shader::_current_ = nullptr;
Shader *Shader::default_load = nullptr;

string vertex_module_default = "vertex-default-nix";

static shared_array<Shader> shaders;

int current_program = 0;

string shader_error;


int create_empty_shader_program() {
	int gl_p = glCreateProgram();
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
	bool has_vertex = false;
	bool has_fragment = false;
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
			has_vertex = true;
		} else if (tag == "FragmentShader") {
			p.type = GL_FRAGMENT_SHADER;
			has_fragment = true;
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
	if (has_fragment and !has_vertex) {
		msg_write(" ...auto import " + vertex_module_default);
		parts.add({GL_VERTEX_SHADER, format("#import %s\n", vertex_module_default)});
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
				//msg_error("FOUND " + imp);
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
	if (gl_shader <= 0)
		throw Exception("could not create gl shader object");
	
	const char *pbuf[2];

	pbuf[0] = source.c_str();
	glShaderSource(gl_shader, 1, pbuf, NULL);

	glCompileShader(gl_shader);

	int status;
	glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &status);
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
		}
	}

	int status;
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		shader_error.resize(16384);
		int size;
		glGetProgramInfoLog(prog, shader_error.num, &size, (char*)shader_error.data);
		shader_error.resize(size);
		throw Exception("while linking the shader program: " + shader_error);
	}

	for (int shader: shaders)
		glDeleteShader(shader);

	program = prog;
	shader_error = "";

	find_locations();
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

	for (Shader *s: weak(shaders))
		if (s->filename == filename)
			return (s->program >= 0) ? s : nullptr;

	msg_write("loading shader: " + filename.str());

	string source = os::fs::read_text(filename);
	Shader *shader = Shader::create(source);
	if (shader)
		shader->filename = filename;

	return shader;
}

void select_default_vertex_module(const string &name) {
	vertex_module_default = name;
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
	program = -1;
}

void delete_all_shaders() {
	return;
	shaders.clear();
	init_shaders();
}

void set_shader(Shader *s) {
	if (s == nullptr) {
		msg_error("setting null shader");
		return;
	}
	//	s = Shader::default_3d;
	Shader::_current_ = s;
	current_program = s->program;
	glUseProgram(current_program);

	//s->set_default_data();
}

int Shader::get_location(const string &name) {
	return glGetUniformLocation(program, name.c_str());
}

bool Shader::link_uniform_block(const string &name, int binding) {
	if (program < 0)
		return false;
	int index = glGetUniformBlockIndex(program, name.c_str());
	if (index < 0)
		return false;
	glUniformBlockBinding(program, index, binding);
	return true;
}

void Shader::set_floats_l(int location, const float *data, int num) {
	if (location < 0)
		return;
	//NixSetShader(this);
	if (num == 1) {
		glProgramUniform1f(program, location, *data);
	} else if (num == 2) {
		glProgramUniform2fv(program, location, 1, data);
	} else if (num == 3) {
		glProgramUniform3fv(program, location, 1, data);
	} else if (num == 4) {
		glProgramUniform4fv(program, location, 1, data);
	} else if (num == 16) {
		glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, (float*)data);
	}
}

void Shader::set_int_l(int location, int i) {
	if (location < 0)
		return;
	glProgramUniform1i(program, location, i);
}

void Shader::set_float_l(int location, float f) {
	if (location < 0)
		return;
	glProgramUniform1f(program, location, f);
}

void Shader::set_color_l(int location, const color &c) {
	if (location < 0)
		return;
	glProgramUniform4fv(program, location, 1, (float*)&c);
}

void Shader::set_matrix_l(int location, const matrix &m) {
	if (location < 0)
		return;
	glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, (float*)&m);
}

void Shader::set_int(const string &name, int i) {
	set_int_l(get_location(name), i);
}

void Shader::set_float(const string &name, float f) {
	set_float_l(get_location(name), f);
}

void Shader::set_color(const string &name, const color &c) {
	set_color_l(get_location(name), c);
}

void Shader::set_matrix(const string &name, const matrix &m) {
	set_matrix_l(get_location(name), m);
}

void Shader::set_floats(const string &name, const float *data, int num) {
	set_floats_l(get_location(name), data, num);
}

void Shader::set_default_data() {
	set_matrix_l(location[LOCATION_MATRIX_MVP], model_view_projection_matrix);
	set_matrix_l(location[LOCATION_MATRIX_M], model_matrix);
	set_matrix_l(location[LOCATION_MATRIX_V], view_matrix);
	set_matrix_l(location[LOCATION_MATRIX_P], projection_matrix);
	for (int i=0; i<NIX_MAX_TEXTURELEVELS; i++)
		set_int_l(location[LOCATION_TEX + i], i);
	if (tex_cube_level >= 0)
		set_int_l(location[LOCATION_TEX_CUBE], tex_cube_level);
	set_color_l(location[LOCATION_MATERIAL_ALBEDO], material.albedo);
	set_float_l(location[LOCATION_MATERIAL_ROUGHNESS], material.roughness);
	set_float_l(location[LOCATION_MATERIAL_METAL], material.metal);
	set_color_l(location[LOCATION_MATERIAL_EMISSION], material.emission);
}

void Shader::dispatch(int nx, int ny, int nz) {
	glUseProgram(program);
	glDispatchCompute(nx, ny, nz);
}


void init_shaders() {
	try {


		//ShaderModule
		shader_modules.add({{"", vertex_module_default},
R"foodelim(
#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos; // camera space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
	out_pos = matrix.view * matrix.model * vec4(in_position, 1);
}
)foodelim"});



	Shader::default_3d = nix::Shader::create(
R"foodelim(
<Layout>
	version = 330 core
</Layout>
<VertexShader>
#import vertex-default-nix
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;
struct Material { vec4 albedo, emission; float roughness, metal; };
/*layout(binding = 2)*/ uniform Material material;
struct Light { mat4 proj; vec4 pos, dir, color; float radius, theta, harshness; };
uniform int num_lights = 0;
/*layout(binding = 1)*/ uniform LightData { Light light[32]; };

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos;
uniform sampler2D tex0;
out vec4 out_color;

vec4 basic_lighting(Light l, vec3 n, vec4 tex_col) {
	float attenuation = 1.0;
	vec3 LD = (matrix.view * vec4(l.dir.xyz, 0)).xyz;
	vec3 LP = (matrix.view * vec4(l.pos.xyz, 1)).xyz;
	if (l.radius > 0) {
		LD = normalize(in_pos.xyz - LP);
		attenuation = min(l.radius / length(in_pos.xyz - LP), 1);
	}
	float d = max(-dot(n, LD), 0) * attenuation;
	vec4 color = material.albedo * material.roughness * l.color * (1 - l.harshness) / 2;
	color += material.albedo * l.color * l.harshness * d;
	color *= tex_col;
	if ((d > 0) && (material.roughness < 0.8)) {
		vec3 e = normalize(in_pos.xyz); // eye dir
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
R"foodelim(<Layout>
	version = 330 core
</Layout>
<VertexShader>
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

