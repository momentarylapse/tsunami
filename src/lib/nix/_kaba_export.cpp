#include "../base/base.h"
#include "nix.h"
#include "../kabaexport/KabaExporter.h"


#if HAS_LIB_GL

#define gl_p(p)		p



KABA_LINK_GROUP_BEGIN

xfer<nix::Texture> __LoadTexture(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return nix::Texture::load(filename));
	return nullptr;
}

xfer<nix::Shader> __ContextLoadShader(nix::Context *ctx, const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return ctx->load_shader(filename));
	return nullptr;
}

xfer<nix::Shader> __ContextCreateShader(nix::Context *ctx, const string &source) {
	KABA_EXCEPTION_WRAPPER(return ctx->create_shader(source));
	return nullptr;
}

KABA_LINK_GROUP_END


class KabaTexture : public nix::Texture {
public:
	void _cdecl __init__(int width, int height, const string &format) {
		new(this) nix::Texture(width, height, format);
	}

	void _cdecl __init_multi_sample__(int width, int height, int samples, const string &format) {
		new(this) nix::TextureMultiSample(width, height, samples, format);
	}
	void _cdecl __init_volume__(int nx, int ny, int nz, const string &format) {
		new(this) nix::VolumeTexture(nx, ny, nz, format);
	}
	void _cdecl __init_image__(int width, int height, const string &format) {
		new(this) nix::ImageTexture(width, height, format);
	}
	void _cdecl __init_depth__(int width, int height, const string &format) {
		new(this) nix::DepthBuffer(width, height, format);
	}
	void _cdecl __init_cube__(int size, const string &format) {
		new(this) nix::CubeMap(size, format);
	}
};

class KabaFrameBuffer : public nix::FrameBuffer {
public:
	void __init__(const shared_array<nix::Texture> &attachments) {
		new(this) nix::FrameBuffer(attachments);
	}
};

class KabaUniformBuffer : public nix::UniformBuffer {
public:
	void __init__(int size) {
		new(this) nix::UniformBuffer(size);
	}
};

class KabaShaderStorageBuffer : public nix::ShaderStorageBuffer {
public:
	void __init__(int size) {
		new(this) nix::ShaderStorageBuffer(size);
	}
};

void _dummy_func() {}



void export_package_gl(kaba::Exporter* e) {
	e->declare_class_size("Context", sizeof(nix::Context));
	e->declare_class_element("Context.gl_version", &nix::Context::gl_version);
	e->declare_class_element("Context.gl_renderer", &nix::Context::gl_renderer);
	e->declare_class_element("Context.glsl_version", &nix::Context::glsl_version);
	e->declare_class_element("Context.extensions", &nix::Context::extensions);
	e->declare_class_element("Context.default_framebuffer", &nix::Context::default_framebuffer);
	e->declare_class_element("Context.tex_white", &nix::Context::tex_white);
	e->declare_class_element("Context.default_shader_2d", &nix::Context::default_2d);
	e->declare_class_element("Context.default_shader_3d", &nix::Context::default_3d);
	e->declare_class_element("Context.vb_temp", &nix::Context::vb_temp);
	e->declare_class_element("Context.vb_temp_i", &nix::Context::vb_temp_i);
	e->link_class_func("Context.load_shader", &__ContextLoadShader);
	e->link_class_func("Context.create_shader", &__ContextCreateShader);

	e->declare_class_size("VertexBuffer", sizeof(nix::VertexBuffer));
	e->link_class_func("VertexBuffer.__init__", &nix::VertexBuffer::__init__);
	e->link_class_func("VertexBuffer.__delete__", &nix::VertexBuffer::__delete__);
	e->link_class_func("VertexBuffer.update", &nix::VertexBuffer::update);
	e->link_class_func("VertexBuffer.update_index", &nix::VertexBuffer::update_index);
	e->link_class_func("VertexBuffer.create_quad", &nix::VertexBuffer::create_quad);
	e->link_class_func("VertexBuffer.create_cube", &nix::VertexBuffer::create_cube);
	e->link_class_func("VertexBuffer.count", &nix::VertexBuffer::count);

	e->declare_class_size("Texture", sizeof(nix::Texture));
	e->declare_class_element("Texture.width", &nix::Texture::width);
	e->declare_class_element("Texture.height", &nix::Texture::height);
	// shared counter...
	e->link_class_func("Texture.__init__", &KabaTexture::__init__);
	e->link_class_func("Texture.__delete__", &kaba::generic_delete<nix::Texture>);
	e->link_class_func("Texture.set_options", &nix::Texture::set_options);
	e->link_class_func("Texture.write", &nix::Texture::write);
	e->link_class_func("Texture.read", &nix::Texture::read);
	e->link_class_func("Texture.read_float", &nix::Texture::read_float);
	e->link_class_func("Texture.write_float", &nix::Texture::write_float);
	e->link_func("Texture.load", &__LoadTexture);


	e->link_class_func("VolumeTexture.__init__", &KabaTexture::__init_volume__);
	// delete...

	e->link_class_func("ImageTexture.__init__", &KabaTexture::__init_image__);
	
	e->link_class_func("DepthBuffer.__init__", &KabaTexture::__init_depth__);
	
	e->link_class_func("CubeMap.__init__", &KabaTexture::__init_cube__);


	e->declare_class_size("FrameBuffer", sizeof(nix::FrameBuffer));
	e->link_class_func("FrameBuffer.__init__", &KabaFrameBuffer::__init__);
	e->link_class_func("FrameBuffer.__delete__", &kaba::generic_delete<nix::FrameBuffer>);
	e->link_class_func("FrameBuffer.area", &nix::FrameBuffer::area);
	e->link_class_func("FrameBuffer.clear_color", &nix::FrameBuffer::clear_color);
	e->link_class_func("FrameBuffer.clear_depth", &nix::FrameBuffer::clear_depth);
//	e->link_class_func("FrameBuffer.update", &nix::FrameBuffer::update);
	e->link_class_func("FrameBuffer.update", &nix::FrameBuffer::update_x);
	e->link_class_func("FrameBuffer.read", &nix::FrameBuffer::read);
	e->declare_class_element("FrameBuffer.width", &nix::FrameBuffer::width);
	e->declare_class_element("FrameBuffer.height", &nix::FrameBuffer::height);
	e->declare_class_element("FrameBuffer.color_attachments", &nix::FrameBuffer::color_attachments);
	e->declare_class_element("FrameBuffer.depth_buffer", &nix::FrameBuffer::depth_buffer);
	//class_add_element(Identifier::SharedCount, TypeInt32, gl_p(&nix::FrameBuffer::_pointer_ref_counter));

	e->declare_class_size("Shader", sizeof(nix::Shader));
	e->link_class_func("Shader.__delete__", &kaba::generic_delete<nix::Shader>);
	e->link_class_func("Shader.location", &nix::Shader::get_location);
	e->link_class_func("Shader.link_uniform_block", &nix::Shader::link_uniform_block);
	e->link_class_func("Shader.set_float_l", &nix::Shader::set_float_l);
	e->link_class_func("Shader.set_matrix_l", &nix::Shader::set_matrix_l);
	e->link_class_func("Shader.set_color_l", &nix::Shader::set_color_l);
	e->link_class_func("Shader.set_int_l", &nix::Shader::set_int_l);
	e->link_class_func("Shader.set_floats_l", &nix::Shader::set_floats_l);
	e->link_class_func("Shader.set_float", &nix::Shader::set_float);
	e->link_class_func("Shader.set_matrix", &nix::Shader::set_matrix);
	e->link_class_func("Shader.set_color", &nix::Shader::set_color);
	e->link_class_func("Shader.set_int", &nix::Shader::set_int);
	e->link_class_func("Shader.set_floats", &nix::Shader::set_floats);
	e->link_class_func("Shader.dispatch", &nix::Shader::dispatch);
		
	e->declare_class_size("Buffer", sizeof(nix::Buffer));
	e->link_class_func("Buffer.update:Buffer:void&:i32", &nix::Buffer::update);
	e->link_class_func("Buffer.update:Buffer:@DynamicArray:i32", &nix::Buffer::update_array);
	e->link_class_func("Buffer.read:Buffer:void&:i32", &nix::Buffer::read);
	e->link_class_func("Buffer.read:Buffer:@DynamicArray", &nix::Buffer::read_array);
			
	e->declare_class_size("UniformBuffer", sizeof(nix::UniformBuffer));
	e->link_class_func("UniformBuffer.__init__", &KabaUniformBuffer::__init__);
	e->link_class_func("UniformBuffer.__delete__", &kaba::generic_delete<nix::Buffer>);

	e->declare_class_size("ShaderStorageBuffer", sizeof(nix::ShaderStorageBuffer));
	e->link_class_func("ShaderStorageBuffer.__init__", &KabaShaderStorageBuffer::__init__);
	e->link_class_func("ShaderStorageBuffer.__delete__", &kaba::generic_delete<nix::Buffer>);

		// drawing
	e->link_func("init", &nix::init);
	//add_func("kill", TypeVoid, gl_p(&nix::kill), Flags::STATIC);
	e->link_func("start_frame_hui", &nix::start_frame_hui);
	e->link_func("end_frame_hui", &nix::end_frame_hui);
	e->link_func("bind_frame_buffer", &nix::bind_frame_buffer);
	e->link_func("set_viewport", &nix::set_viewport);
	e->link_func("clear_color", &nix::clear_color);
	e->link_func("clear_z", &nix::clear_z);
	e->link_func("clear", &nix::clear);
	e->link_func("set_model_matrix", &nix::set_model_matrix);
	e->link_func("draw_triangles", &nix::draw_triangles);
	e->link_func("draw_lines", &nix::draw_lines);
	e->link_func("draw_points", &nix::draw_points);
	e->link_func("disable_alpha", &nix::disable_alpha);
	e->link_func("set_alpha", &nix::set_alpha_sd);
	e->link_func("set_stencil", &nix::set_stencil);
	e->link_func("set_projection_perspective", &nix::set_projection_perspective);
	e->link_func("set_projection_perspective:vec2:vec2:f32:f32", &nix::set_projection_perspective_ext);
	e->link_func("set_projection_ortho_relative", &nix::set_projection_ortho_relative);
	e->link_func("set_projection_ortho_pixel", &nix::set_projection_ortho_pixel);
	e->link_func("set_projection_ortho_ext", &nix::set_projection_ortho_ext);
	e->link_func("set_projection_matrix", &nix::set_projection_matrix);
	e->link_func("set_view_matrix", &nix::set_view_matrix);
	e->link_func("set_scissor", &nix::set_scissor);
	e->link_func("set_z", &nix::set_z);
	e->link_func("set_cull", &nix::set_cull);
	e->link_func("set_wire", &nix::set_wire);
	e->link_func("set_material", &nix::set_material);
	e->link_func("bind_textures", &nix::bind_textures);
	e->link_func("bind_texture", &nix::bind_texture);
	e->link_func("set_shader", &nix::set_shader);
	e->link_func("bind_uniform_buffer", &nix::bind_uniform_buffer);
	e->link_func("bind_storage_buffer", &nix::bind_storage_buffer);
	e->link_func("bind_image", &nix::bind_image);
	e->link_func("screen_shot_to_image", &nix::screen_shot_to_image);

	e->link("target", &nix::target_rect);

	// alpha operations
	e->declare_enum("Alpha.ZERO", nix::Alpha::ZERO);
	e->declare_enum("Alpha.ONE", nix::Alpha::ONE);
	e->declare_enum("Alpha.SOURCE_COLOR", nix::Alpha::SOURCE_COLOR);
	e->declare_enum("Alpha.SOURCE_INV_COLOR", nix::Alpha::SOURCE_INV_COLOR);
	e->declare_enum("Alpha.SOURCE_ALPHA", nix::Alpha::SOURCE_ALPHA);
	e->declare_enum("Alpha.SOURCE_INV_ALPHA", nix::Alpha::SOURCE_INV_ALPHA);
	e->declare_enum("Alpha.DEST_COLOR", nix::Alpha::DEST_COLOR);
	e->declare_enum("Alpha.DEST_INV_COLOR", nix::Alpha::DEST_INV_COLOR);
	e->declare_enum("Alpha.DEST_ALPHA", nix::Alpha::DEST_ALPHA);
	e->declare_enum("Alpha.DEST_INV_ALPHA", nix::Alpha::DEST_INV_ALPHA);
	
	// stencil operations
	e->declare_enum("StencilOp.NONE", nix::StencilOp::NONE);
	e->declare_enum("StencilOp.INCREASE", nix::StencilOp::INCREASE);
	e->declare_enum("StencilOp.DECREASE", nix::StencilOp::DECREASE);
	e->declare_enum("StencilOp.SET", nix::StencilOp::SET);
	e->declare_enum("StencilOp.MASK_EQUAL", nix::StencilOp::MASK_EQUAL);
	e->declare_enum("StencilOp.MASK_NOT_EQUAL", nix::StencilOp::MASK_NOT_EQUAL);
	e->declare_enum("StencilOp.MASK_LESS", nix::StencilOp::MASK_LESS);
	e->declare_enum("StencilOp.MASK_LESS_EQUAL", nix::StencilOp::MASK_LESS_EQUAL);
	e->declare_enum("StencilOp.MASK_GREATER", nix::StencilOp::MASK_GREATER);
	e->declare_enum("StencilOp.MASK_GREATER_EQUAL", nix::StencilOp::MASK_GREATER_EQUAL);
	e->declare_enum("StencilOp.RESET", nix::StencilOp::RESET);
	// fog
	e->declare_enum("FogMode.LINEAR", nix::FogMode::LINEAR);
	e->declare_enum("FogMode.EXP", nix::FogMode::EXP);
	e->declare_enum("FogMode.EXP2", nix::FogMode::EXP2);
	// culling
	e->declare_enum("CullMode.NONE", nix::CullMode::NONE);
	e->declare_enum("CullMode.BACK", nix::CullMode::BACK);
	e->declare_enum("CullMode.FRONT", nix::CullMode::FRONT);



//	add_ext_var("vb_temp", TypeVertexBufferRef, gl_p(&nix::vb_temp));
}

#else

void export_package_gl(kaba::Exporter* e) {
}

#endif


