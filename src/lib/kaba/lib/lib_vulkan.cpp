#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "../../math/matrix.h"
#include "common.h"
#include "exception.h"

#ifdef _X_USE_VULKAN_
	#include "../../vulkan/vulkan.h"

#if HAS_LIB_VULKAN
#endif
#endif

namespace kaba {



#if defined(_X_USE_VULKAN_) && HAS_LIB_VULKAN
	#define vul_p(p)		p



#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

vulkan::Texture* __vulkan_load_texture(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return vulkan::Texture::load(filename));
	return nullptr;
}

vulkan::Shader* __vulkan_load_shader(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return vulkan::Shader::load(filename));
	return nullptr;
}

void __vulkan_init(GLFWwindow* window, const Array<string> &op) {
	KABA_EXCEPTION_WRAPPER(vulkan::init(window, op));
}

#pragma GCC pop_options

class VulkanVertexList : public Array<vulkan::Vertex1> {
public:
	void __init__() {
		new(this) VulkanVertexList;
	}
};

class VulkanVertex : public vulkan::Vertex1 {
public:
	void __assign__(const VulkanVertex &o) { *this = o; }
};


class ColorList : public Array<color> {
public:
	void __init__() {
		new(this) ColorList;
	}
	void __delete__() {
		this->~ColorList();
	}
	void __assign__(ColorList &o) {
		*this = o;
	}
};


#else
	namespace vulkan{
		typedef int VertexBuffer;
		typedef int Texture;
		typedef int Shader;
		typedef int Pipeline;
		typedef int RayPipeline;
		typedef int Vertex1;
		typedef int RenderPass;
		typedef int UniformBuffer;
		typedef int DescriptorPool;
		typedef int DescriptorSet;
		typedef int CommandBuffer;
		typedef int SwapChain;
		typedef int Fence;
		typedef int Semaphore;
		typedef int DepthBuffer;
		typedef int FrameBuffer;
		typedef int DynamicTexture;
		typedef int StorageImage;
		typedef int AccelerationStructure;
	};
	#define vul_p(p)		nullptr
#endif


extern const Class *TypeIntList;
extern const Class *TypeIntP;
extern const Class *TypePointerList;
extern const Class *TypeStringList;
extern const Class *TypeImage;
extern const Class *TypeColorList;
extern const Class *TypePath;
extern const Class *TypeDynamicArray;



void SIAddPackageVulkan() {
	add_package("vulkan");
	
	auto TypeVertexBuffer	= add_type  ("VertexBuffer", sizeof(vulkan::VertexBuffer));
	//auto TypeVertexBufferP	= add_type_p(TypeVertexBuffer);
	auto TypeTexture		= add_type  ("Texture", sizeof(vulkan::Texture));
	auto TypeTextureP		= add_type_p(TypeTexture);
	auto TypeTexturePList	= add_type_l(TypeTextureP);
	auto TypeDynamicTexture	= add_type  ("DynamicTexture", sizeof(vulkan::DynamicTexture));
	auto TypeDepthBuffer	= add_type  ("DepthBuffer", sizeof(vulkan::DepthBuffer));
	auto TypeDepthBufferP	= add_type_p(TypeDepthBuffer);
	auto TypeFrameBuffer	= add_type  ("FrameBuffer", sizeof(vulkan::FrameBuffer));
	auto TypeFrameBufferP	= add_type_p(TypeFrameBuffer);
	auto TypeFrameBufferPList= add_type_l(TypeFrameBufferP);
	auto TypeShader			= add_type  ("Shader", sizeof(vulkan::Shader));
	auto TypeShaderP		= add_type_p(TypeShader);
	auto TypeShaderPList	= add_type_l(TypeShaderP);
	auto TypeCommandBuffer	= add_type  ("CommandBuffer", sizeof(vulkan::CommandBuffer));
	auto TypeCommandBufferP	= add_type_p(TypeCommandBuffer);
	auto TypeVertex			= add_type  ("Vertex", sizeof(vulkan::Vertex1));
	auto TypeVertexList		= add_type_l(TypeVertex);
	auto TypePipeline		= add_type  ("Pipeline", sizeof(vulkan::Pipeline));
	auto TypeRayPipeline	= add_type  ("RayPipeline", sizeof(vulkan::RayPipeline));
	auto TypeRenderPass		= add_type  ("RenderPass", sizeof(vulkan::RenderPass));
	auto TypeRenderPassP	= add_type_p(TypeRenderPass);
	auto TypeUniformBuffer	= add_type  ("UniformBuffer", sizeof(vulkan::UniformBuffer));
	auto TypeUniformBufferP	= add_type_p(TypeUniformBuffer);
	auto TypeUniformBufferPList= add_type_l(TypeUniformBufferP);
	auto TypeDescriptorPool	= add_type  ("DescriptorPool", sizeof(vulkan::DescriptorPool));
	auto TypeDescriptorSet	= add_type  ("DescriptorSet", sizeof(vulkan::DescriptorSet));
	auto TypeDescriptorSetP	= add_type_p(TypeDescriptorSet);
	auto TypeSwapChain		= add_type  ("SwapChain", sizeof(vulkan::SwapChain));
	auto TypeFence			= add_type  ("Fence", sizeof(vulkan::Fence));
	auto TypeSemaphore		= add_type  ("Semaphore", sizeof(vulkan::Semaphore));
	auto TypeSemaphoreP		= add_type_p(TypeSemaphore);
	auto TypeSemaphorePList	= add_type_l(TypeSemaphoreP);
	auto TypeAccelerationStructure = add_type  ("AccelerationStructure", sizeof(vulkan::AccelerationStructure));
	auto TypeAccelerationStructureP = add_type_p(TypeAccelerationStructure);

	add_class(TypeVertex);
		class_add_elementx("pos", TypeVector, vul_p(&vulkan::Vertex1::pos));
		class_add_elementx("normal", TypeVector, vul_p(&vulkan::Vertex1::normal));
		class_add_elementx("u", TypeFloat32, vul_p(&vulkan::Vertex1::u));
		class_add_elementx("v", TypeFloat32, vul_p(&vulkan::Vertex1::v));
		class_add_funcx("__assign__", TypeVoid, vul_p(&VulkanVertex::__assign__));
			func_add_param("o", TypeVertex);

	add_class(TypeVertexList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&VulkanVertexList::__init__));

	add_class(TypeVertexBuffer);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::VertexBuffer::__init__));
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::VertexBuffer::__delete__));
		class_add_funcx("build", TypeVoid, vul_p(&vulkan::VertexBuffer::build_v3_v3_v2_i));
			func_add_param("vertices", TypeDynamicArray);
			func_add_param("indices", TypeIntList);
		class_add_funcx("build", TypeVoid, vul_p(&vulkan::VertexBuffer::build));
			func_add_param("vertices", TypeDynamicArray);



	add_class(TypeTexture);
		class_add_elementx("width", TypeInt, vul_p(&vulkan::Texture::width));
		class_add_elementx("height", TypeInt, vul_p(&vulkan::Texture::height));
		class_add_elementx("view", TypePointer, vul_p(&vulkan::Texture::view));
		class_add_elementx("format", TypeInt, vul_p(&vulkan::Texture::format));
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::Texture::__init__));
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::Texture::__delete__));
		class_add_funcx("override", TypeVoid, vul_p(&vulkan::Texture::override));
			func_add_param("image", TypeImage);
		class_add_funcx("override", TypeVoid, vul_p(&vulkan::Texture::overridex));
			func_add_param("data", TypePointer);
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
			func_add_param("format", TypeString);
		class_add_funcx("load", TypeTextureP, vul_p(&__vulkan_load_texture), Flags::STATIC);
			func_add_param("filename", TypePath);


	add_class(TypeDynamicTexture);
		class_derive_from(TypeTexture, true, false);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::DynamicTexture::__init__));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
			func_add_param("format", TypeString);


	add_class(TypeDepthBuffer);
		class_derive_from(TypeTexture, true, false);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::DepthBuffer::__init__));
			func_add_param("w", TypeInt);
			func_add_param("h", TypeInt);
			func_add_param("format", TypeString);
			func_add_param("with_sampler", TypeBool);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::DepthBuffer::__delete__), Flags::OVERRIDE);


	add_class(TypeFrameBuffer);
		class_add_elementx("width", TypeInt, vul_p(&vulkan::FrameBuffer::width));
		class_add_elementx("height", TypeInt, vul_p(&vulkan::FrameBuffer::height));
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::FrameBuffer::__init__));
			func_add_param("w", TypeInt);
			func_add_param("h", TypeInt);
			func_add_param("rp", TypeRenderPass);
			func_add_param("attachments", TypePointerList);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::FrameBuffer::__delete__));


	add_class(TypeShader);
		//class_add_elementx("descr_layout", TypePointerList, vul_p(&vulkan::Shader::descr_layouts));
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::Shader::__init__));
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::Shader::__delete__));
		class_add_funcx("load", TypeShaderP, vul_p(&__vulkan_load_shader), Flags::STATIC);
			func_add_param("filename", TypePath);


	add_class(TypeUniformBuffer);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::UniformBuffer::__init__));
			func_add_param("size", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::UniformBuffer::__delete__));
		class_add_funcx("update", TypeVoid, vul_p(&vulkan::UniformBuffer::update));
			func_add_param("source", TypePointer);

	add_class(TypeDescriptorPool);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::DescriptorPool::__init__));
			func_add_param("s", TypeString);
			func_add_param("max_sets", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::DescriptorPool::__delete__));
		class_add_funcx("create_set", TypeDescriptorSetP, vul_p(&vulkan::DescriptorPool::create_set), Flags::CONST);
			func_add_param("bindings", TypeString);

	add_class(TypeDescriptorSet);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::DescriptorSet::__delete__));
		class_add_funcx("update", TypeVoid, vul_p(&vulkan::DescriptorSet::update));
		class_add_funcx("set_texture", TypeVoid, vul_p(&vulkan::DescriptorSet::set_texture));
			func_add_param("binding", TypeInt);
			func_add_param("tex", TypeTexture);
		class_add_funcx("set_storage_image", TypeVoid, vul_p(&vulkan::DescriptorSet::set_storage_image));
			func_add_param("binding", TypeInt);
			func_add_param("tex", TypeTexture);
		class_add_funcx("set_buffer", TypeVoid, vul_p(&vulkan::DescriptorSet::set_buffer));
			func_add_param("binding", TypeInt);
			func_add_param("ubo", TypeUniformBuffer);
		class_add_funcx("set_acceleration_structure", TypeVoid, vul_p(&vulkan::DescriptorSet::set_acceleration_structure));
			func_add_param("binding", TypeInt);
			func_add_param("as", TypeAccelerationStructure);


	add_class(TypePipeline);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::Pipeline::__init__));
			func_add_param("shader", TypeShader);
			func_add_param("pass", TypeRenderPass);
			func_add_param("subpass", TypeInt);
			func_add_param("num_textures", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::Pipeline::__delete__));
		class_add_funcx("set_wireframe", TypeVoid, vul_p(&vulkan::Pipeline::set_wireframe));
			func_add_param("w", TypeBool);
		class_add_funcx("set_line_width", TypeVoid, vul_p(&vulkan::Pipeline::set_line_width));
			func_add_param("w", TypeFloat32);
#if defined(_X_USE_VULKAN_) && HAS_LIB_VULKAN
		void (vulkan::Pipeline::*mpf)(float) = &vulkan::Pipeline::set_blend;
		class_add_funcx("set_blend", TypeVoid, vul_p(mpf));
#else
		class_add_funcx("set_blend", TypeVoid, nullptr);
#endif
			func_add_param("alpha", TypeFloat32);
#if defined(_X_USE_VULKAN_) && HAS_LIB_VULKAN
		void (vulkan::Pipeline::*mpf2)(VkBlendFactor, VkBlendFactor) = &vulkan::Pipeline::set_blend;
		class_add_funcx("set_blend", TypeVoid, vul_p(mpf2));
#else
		class_add_funcx("set_blend", TypeVoid, nullptr);
#endif
			func_add_param("src", TypeInt);
			func_add_param("dst", TypeInt);
		class_add_funcx("set_z", TypeVoid, vul_p(&vulkan::Pipeline::set_z));
			func_add_param("test", TypeBool);
			func_add_param("write", TypeBool);
		class_add_funcx("set_dynamic", TypeVoid, vul_p(&vulkan::Pipeline::set_dynamic));
			func_add_param("mode", TypeIntList);
		class_add_funcx("rebuild", TypeVoid, vul_p(&vulkan::Pipeline::rebuild));

	add_class(TypeRayPipeline);
		class_derive_from(TypePipeline, true, false);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::RayPipeline::__init__));
			func_add_param("layout", TypeString);
			func_add_param("shader", TypeShaderPList);
		class_add_funcx("create_sbt", TypeVoid, vul_p(&vulkan::RayPipeline::create_sbt));

	add_class(TypeRenderPass);
		class_add_elementx("clear_color", TypeColorList, vul_p(&vulkan::RenderPass::clear_color));
		class_add_elementx("clear_z", TypeFloat32, vul_p(&vulkan::RenderPass::clear_z));
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::RenderPass::__init__));
			func_add_param("formats", TypeIntList);
			func_add_param("options", TypeString);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::RenderPass::__delete__));
		class_add_funcx("rebuild", TypeVoid, vul_p(&vulkan::RenderPass::rebuild));
		class_add_funcx("add_subpass", TypeVoid, vul_p(&vulkan::RenderPass::add_subpass));
			func_add_param("color_att", TypeIntList);
			func_add_param("depth_att", TypeInt);
		class_add_funcx("add_dependency", TypeVoid, vul_p(&vulkan::RenderPass::add_dependency));
			func_add_param("src", TypeInt);
			func_add_param("src_opt", TypeString);
			func_add_param("dst", TypeInt);
			func_add_param("dst_opt", TypeString);


	add_class(TypeSwapChain);
		class_add_elementx("width", TypeInt, vul_p(&vulkan::SwapChain::width));
		class_add_elementx("height", TypeInt, vul_p(&vulkan::SwapChain::height));
		class_add_elementx("format", TypeInt, vul_p(&vulkan::SwapChain::image_format));
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::SwapChain::__init__));
			func_add_param("win", TypePointer);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::SwapChain::__delete__));
		class_add_funcx("create_depth_buffer", TypeDepthBufferP, vul_p(&vulkan::SwapChain::create_depth_buffer));
		class_add_funcx("create_render_pass", TypeRenderPassP, vul_p(&vulkan::SwapChain::create_render_pass));
			func_add_param("depth_buffer", TypeDepthBufferP);
		class_add_funcx("create_frame_buffers", TypeFrameBufferPList, vul_p(&vulkan::SwapChain::create_frame_buffers));
			func_add_param("render_pass", TypeRenderPassP);
			func_add_param("depth_buffer", TypeDepthBufferP);
		class_add_funcx("create_textures", TypeTexturePList, vul_p(&vulkan::SwapChain::create_textures));
		class_add_funcx("rebuild", TypeVoid, vul_p(&vulkan::SwapChain::rebuild));
		class_add_funcx("present", TypeBool, vul_p(&vulkan::SwapChain::present));
			func_add_param("image_index", TypeInt);
			func_add_param("wait_sem", TypeSemaphorePList);
		class_add_funcx("aquire_image", TypeBool, vul_p(&vulkan::SwapChain::aquire_image));
			func_add_param("image_index", TypeIntP);
			func_add_param("signal_sem", TypeSemaphore);


	add_class(TypeFence);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::Fence::__init__));
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::Fence::__delete__));
		class_add_funcx("wait", TypeVoid, vul_p(&vulkan::Fence::wait));
		class_add_funcx("reset", TypeVoid, vul_p(&vulkan::Fence::reset));


	add_class(TypeSemaphore);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::Semaphore::__init__));
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::Semaphore::__delete__));


	add_class(TypeCommandBuffer);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, vul_p(&vulkan::CommandBuffer::__init__));
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, vul_p(&vulkan::CommandBuffer::__delete__));
		class_add_funcx("begin", TypeVoid, vul_p(&vulkan::CommandBuffer::begin));
		class_add_funcx("end", TypeVoid, vul_p(&vulkan::CommandBuffer::end));
		class_add_funcx("set_bind_point", TypeVoid, vul_p(&vulkan::CommandBuffer::set_bind_point));
			func_add_param("p", TypeString);
		class_add_funcx("bind_pipeline", TypeVoid, vul_p(&vulkan::CommandBuffer::bind_pipeline));
			func_add_param("p", TypePipeline);
		class_add_funcx("draw", TypeVoid, vul_p(&vulkan::CommandBuffer::draw));
			func_add_param("vb", TypeVertexBuffer);
		class_add_funcx("begin_render_pass", TypeVoid, vul_p(&vulkan::CommandBuffer::begin_render_pass));
			func_add_param("rp", TypeRenderPass);
			func_add_param("fb", TypeFrameBuffer);
		class_add_funcx("next_subpass", TypeVoid, vul_p(&vulkan::CommandBuffer::next_subpass));
		class_add_funcx("end_render_pass", TypeVoid, vul_p(&vulkan::CommandBuffer::end_render_pass));
		class_add_funcx("push_constant", TypeVoid, vul_p(&vulkan::CommandBuffer::push_constant));
			func_add_param("offset", TypeInt);
			func_add_param("size", TypeInt);
			func_add_param("data", TypePointer);
		class_add_funcx("bind_descriptor_set", TypeVoid, vul_p(&vulkan::CommandBuffer::bind_descriptor_set));
			func_add_param("index", TypeInt);
			func_add_param("set", TypeDescriptorSet);
		class_add_funcx("set_scissor", TypeVoid, vul_p(&vulkan::CommandBuffer::set_scissor));
			func_add_param("r", TypeRect);
		class_add_funcx("set_viewport", TypeVoid, vul_p(&vulkan::CommandBuffer::set_viewport));
			func_add_param("r", TypeRect);
		class_add_funcx("dispatch", TypeVoid, vul_p(&vulkan::CommandBuffer::dispatch));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);
		class_add_funcx("barrier", TypeVoid, vul_p(&vulkan::CommandBuffer::barrier));
			func_add_param("t", TypeTexturePList);
			func_add_param("mode", TypeInt);
		class_add_funcx("image_barrier", TypeVoid, vul_p(&vulkan::CommandBuffer::image_barrier));
			func_add_param("t", TypeTexture);
			func_add_param("flags", TypeIntList);
		class_add_funcx("copy_image", TypeVoid, vul_p(&vulkan::CommandBuffer::copy_image));
			func_add_param("src", TypeTexture);
			func_add_param("dst", TypeTexture);
			func_add_param("extend", TypeIntList);
		class_add_funcx("trace_rays", TypeVoid, vul_p(&vulkan::CommandBuffer::trace_rays));
			func_add_param("nx", TypeInt);
			func_add_param("ny", TypeInt);
			func_add_param("nz", TypeInt);

	add_class(TypeAccelerationStructure);
		//class_add_funcx("create_top", TypeAccelerationStructureP, vul_p(&vulkan::AccelerationStructure::create_top), Flags::STATIC);
		class_add_funcx("create_top", TypeAccelerationStructureP, vul_p(&vulkan::AccelerationStructure::create_top_simple), Flags::STATIC);
			func_add_param("instances", TypeDynamicArray);
		class_add_funcx("create_bottom", TypeAccelerationStructureP, vul_p(&vulkan::AccelerationStructure::create_bottom), Flags::STATIC);
			func_add_param("vb", TypeVertexBuffer);

	add_funcx("create_window", TypePointer, vul_p(&vulkan::create_window), Flags::STATIC);
		func_add_param("title", TypeString);
		func_add_param("w", TypeInt);
		func_add_param("h", TypeInt);
	add_funcx("window_handle", TypeBool, vul_p(&vulkan::window_handle), Flags::STATIC);
		func_add_param("w", TypePointer);
	add_funcx("window_close", TypeVoid, vul_p(&vulkan::window_close), Flags::STATIC);
		func_add_param("w", TypePointer);

	add_funcx("init", TypeVoid, vul_p(&__vulkan_init), Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("win", TypePointer);
		func_add_param("op", TypeStringList);
	add_funcx("destroy", TypeVoid, vul_p(&vulkan::destroy), Flags::STATIC);
	add_funcx("queue_submit_command_buffer", TypeVoid, vul_p(&vulkan::queue_submit_command_buffer), Flags::STATIC);
		func_add_param("cb", TypeCommandBuffer);
		func_add_param("wait_sem", TypeSemaphorePList);
		func_add_param("signal_sem", TypeSemaphorePList);
		func_add_param("fence", TypeFence);
	add_funcx("wait_device_idle", TypeVoid, vul_p(&vulkan::wait_device_idle), Flags::STATIC);

}

};
