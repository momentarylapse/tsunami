#include "../kaba.h"
#include "../../math/mat4.h"
#include "../../base/optional.h"
#include "lib.h"
#include "shared.h"
#include "list.h"
#include "../dynamic/exception.h"

#if __has_include("../../vulkan/vulkan.h") && HAS_LIB_VULKAN
	#include "../../vulkan/vulkan.h"
	#define KABA_EXPORT_VULKAN
#endif

namespace kaba {

extern const Class* TypeIntOptional;
extern const Class* TypeFloatOptional;


#ifdef KABA_EXPORT_VULKAN
	#define vul_p(p)		p




KABA_LINK_GROUP_BEGIN

xfer<vulkan::Texture> __vulkan_load_texture(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return vulkan::Texture::load(filename));
	return nullptr;
}

xfer<vulkan::Shader> __vulkan_load_shader(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return vulkan::Shader::load(filename));
	return nullptr;
}

xfer<void> __vulkan_init(const Array<string> &op) {
	KABA_EXCEPTION_WRAPPER(return vulkan::init(op));
	return nullptr;
}

xfer<vulkan::Device> __vulkan_device_create_simple(vulkan::Instance *instance, void* surface, const Array<string> &op) {
#if HAS_LIB_GLFW
	KABA_EXCEPTION_WRAPPER(return vulkan::Device::create_simple(instance, (vulkan::Surface)surface, op));
#endif
	return nullptr;
}

KABA_LINK_GROUP_END

class VulkanTexture : vulkan::Texture {
	public:
	void __init__() {
		new(this) vulkan::Texture();
	}
	void __init_ext__(int w, int h, const string &format) {
		new(this) vulkan::Texture(w, h, format);
	}
	void __delete__() {
		this->~VulkanTexture();
	}
};

class VulkanVolumeTexture : public vulkan::VolumeTexture {
public:
	void __init__(int nx, int ny, int nz, const string &format) {
		new(this) vulkan::VolumeTexture(nx, ny, nz, format);
	}
};

class VulkanStorageTexture : public vulkan::StorageTexture {
public:
	void __init__(int nx, int ny, int nz, const string &format) {
		new(this) vulkan::StorageTexture(nx, ny, nz, format);
	}
};

class VulkanCubeMap : public vulkan::CubeMap {
public:
	void __init__(int size, const string &format) {
		new(this) vulkan::CubeMap(size, format);
	}
};

class VulkanVertexList : public Array<vulkan::Vertex1> {
public:
	void __init__() {
		new(this) VulkanVertexList;
	}
};

class VulkanDepthBuffer : public vulkan::DepthBuffer {
public:
	void __init__(int w, int h, const string &format, bool with_sampler) {
		new(this) vulkan::DepthBuffer(w, h, format, with_sampler);
	}
};

class VulkanFrameBuffer : public vulkan::FrameBuffer {
public:
	void __init__(vulkan::RenderPass *rp, const shared_array<vulkan::Texture> &attachments) {
		new(this) vulkan::FrameBuffer(rp, attachments);
	}
	void __delete__() {
		this->~VulkanFrameBuffer();
	}
};

class VulkanCommandBuffer : public vulkan::CommandBuffer {
public:
	void __delete__() {
		this->~VulkanCommandBuffer();
	}
};

class VulkanUniformBuffer : public vulkan::UniformBuffer {
public:
	void __init__(int size) {
		new(this) vulkan::UniformBuffer(size);
	}
	void __init_multi__(int size, int count) {
		new(this) vulkan::UniformBuffer(size, count);
	}
	void __delete__() {
		this->~VulkanUniformBuffer();
	}
};

class VulkanDescriptorPool : public vulkan::DescriptorPool {
public:
	void __init__(const string &s, int max_sets) {
		new(this) DescriptorPool(s, max_sets);
	}
	void __delete__() {
		this->~VulkanDescriptorPool();
	}
};

class VulkanDescriptorSet : public vulkan::DescriptorSet {
public:
	void __delete__() {
		this->~VulkanDescriptorSet();
	}
};

class VulkanInstance : public vulkan::Instance {
public:
	void __delete__() {
		this->~VulkanInstance();
	}
	vulkan::Surface _create_glfw_surface(void* window) {
#ifdef HAS_LIB_GLFW
		KABA_EXCEPTION_WRAPPER(return create_glfw_surface((GLFWwindow*)window));
#endif
		return nullptr;
	}
	vulkan::Surface _create_headless_surface() {
		KABA_EXCEPTION_WRAPPER(return create_headless_surface());
		return nullptr;
	}
};

class VulkanVertexBuffer : public vulkan::VertexBuffer {
public:
	void __init__(const string &format) {
		new(this) vulkan::VertexBuffer(format);
	}
	void __delete__() {
		this->~VulkanVertexBuffer();
	}
};

class VulkanGraphicsPipeline : public vulkan::GraphicsPipeline {
public:
	void __init__(vulkan::Shader *shader, vulkan::RenderPass *render_pass, int subpass, const string &topology, const string &format) {
		new(this) vulkan::GraphicsPipeline(shader, render_pass, subpass, topology, format);
	}
	void __delete__() {
		this->~VulkanGraphicsPipeline();
	}
};

class VulkanComputePipeline : public vulkan::ComputePipeline {
public:
	void __init__(vulkan::Shader *shaders) {
		new(this) vulkan::ComputePipeline(shaders);
	}
};

class VulkanRayPipeline : public vulkan::RayPipeline {
public:
	void __init__(const string &dset_layouts, const Array<vulkan::Shader*> &shaders, int recursion_depth) {
		new (this) vulkan::RayPipeline(dset_layouts, shaders, recursion_depth);
	}
};

class VulkanRenderPass : public vulkan::RenderPass {
public:
	void __init__(const Array<string> &formats, const Array<string> &options) {
		new(this) vulkan::RenderPass(formats, options);
	}
	void __delete__() {
		this->~VulkanRenderPass();
	}
};

class VulkanSwapChain : public vulkan::SwapChain {
public:
	void __delete__() {
		this->~VulkanSwapChain();
	}
	base::optional<int> acquire_image_x(vulkan::Semaphore *sem) {
		int index;
		if (vulkan::SwapChain::acquire_image(&index, sem))
			return index;
		return base::None;
	}
	static xfer<vulkan::SwapChain> _create_for_glfw(vulkan::Device* device, void* window) {
#ifdef HAS_LIB_GLFW
		KABA_EXCEPTION_WRAPPER(return create_for_glfw(device, (GLFWwindow*)window));
#endif
		return nullptr;
	}
	static xfer<vulkan::SwapChain> _create(vulkan::Device* device, int width, int height) {
		KABA_EXCEPTION_WRAPPER(return create(device, width, height));
		return nullptr;
	}
};

class VulkanFence : public vulkan::Fence {
public:
	void __init__(vulkan::Device *device) {
		new(this) vulkan::Fence(device);
	}
	void __delete__() {
		this->~VulkanFence();
	}
};

class VulkanSemaphore : public vulkan::Semaphore {
public:
	void __init__(vulkan::Device *device) {
		new(this) vulkan::Semaphore(device);
	}
	void __delete__() {
		this->~VulkanSemaphore();
	}
};


class VulkanShader : public vulkan::Shader {
public:
	void __init__() {
		new(this) vulkan::Shader();
	}
	void __delete__() {
		this->~VulkanShader();
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
	namespace vulkan {
		typedef int Instance;
		typedef int Device;
		typedef int Queue;
		typedef int VertexBuffer;
		typedef int Shader;
		typedef int BasePipeline;
		typedef int GraphicsPipeline;
		typedef int ComputePipeline;
		typedef int RayPipeline;
		typedef int Vertex1;
		typedef int RenderPass;
		typedef int Buffer;
		typedef int UniformBuffer;
		typedef int DescriptorPool;
		typedef int DescriptorSet;
		typedef int CommandBuffer;
		typedef int CommandPool;
		typedef int SwapChain;
		typedef int Fence;
		typedef int Semaphore;
		typedef int DepthBuffer;
		typedef int FrameBuffer;
		typedef int VolumeTexture;
		typedef int CubeMap;
		typedef int StorageTexture;
		typedef int AccelerationStructure;
		struct Texture : public Sharable<base::Empty> {};
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



void SIAddPackageVulkan(Context *c) {
	add_package(c, "vulkan");

	auto TypeInstance = add_type("Instance", sizeof(vulkan::Instance));
	auto TypeInstanceP = add_type_p_raw(TypeInstance);
	auto TypeInstanceXfer = add_type_p_xfer(TypeInstance);
	auto TypeDevice = add_type("Device", sizeof(vulkan::Device));
	auto TypeDeviceP = add_type_p_raw(TypeDevice);
	auto TypeDeviceXfer = add_type_p_xfer(TypeDevice);
	auto TypeQueue = add_type("Queue", sizeof(vulkan::Queue));
	auto TypeVertexBuffer = add_type("VertexBuffer", sizeof(vulkan::VertexBuffer));
	auto TypeVertexBufferP = add_type_p_raw(TypeVertexBuffer);
	auto TypeTexture = add_type("Texture", sizeof(vulkan::Texture));
	auto TypeTextureXfer = add_type_p_xfer(TypeTexture);
	auto TypeTextureP = add_type_p_raw(TypeTexture);
	auto TypeTexturePList = add_type_list(TypeTextureP);
	auto TypeTextureSharedNN = add_type_p_shared_not_null(TypeTexture);
	auto TypeTextureSharedNNList = add_type_list(TypeTextureSharedNN);
	auto TypeTextureXferList = add_type_list(TypeTextureXfer);
	auto TypeVolumeTexture = add_type("VolumeTexture", sizeof(vulkan::VolumeTexture));
	auto TypeCubeMap = add_type("CubeMap", sizeof(vulkan::CubeMap));
	auto TypeStorageTexture = add_type("StorageTexture", sizeof(vulkan::StorageTexture));
	auto TypeDepthBuffer = add_type("DepthBuffer", sizeof(vulkan::DepthBuffer));
	auto TypeDepthBufferP = add_type_p_raw(TypeDepthBuffer);
	auto TypeDepthBufferXfer = add_type_p_xfer(TypeDepthBuffer);
	auto TypeFrameBuffer = add_type("FrameBuffer", sizeof(vulkan::FrameBuffer));
	auto TypeFrameBufferP = add_type_p_raw(TypeFrameBuffer);
	auto TypeFrameBufferXfer = add_type_p_xfer(TypeFrameBuffer);
	auto TypeFrameBufferXferList = add_type_list(TypeFrameBufferXfer);
	auto TypeShader = add_type("Shader", sizeof(vulkan::Shader));
	auto TypeShaderXfer = add_type_p_xfer(TypeShader);
	auto TypeShaderP = add_type_p_raw(TypeShader);
	auto TypeShaderPList = add_type_list(TypeShaderP);
	auto TypeCommandBuffer = add_type("CommandBuffer", sizeof(vulkan::CommandBuffer));
	auto TypeCommandBufferXfer = add_type_p_xfer(TypeCommandBuffer);
	auto TypeCommandBufferP = add_type_p_raw(TypeCommandBuffer);
	auto TypeCommandPool = add_type("CommandPool", sizeof(vulkan::CommandPool));
	auto TypeCommandPoolP = add_type_p_raw(TypeCommandPool);
	auto TypePipeline = add_type("Pipeline", sizeof(vulkan::BasePipeline));
	auto TypePipelineP = add_type_p_raw(TypePipeline);
	auto TypeGraphicsPipeline = add_type("GraphicsPipeline", sizeof(vulkan::GraphicsPipeline));
	auto TypeComputePipeline = add_type("ComputePipeline", sizeof(vulkan::ComputePipeline));
	auto TypeRayPipeline = add_type("RayPipeline", sizeof(vulkan::RayPipeline));
	auto TypeRenderPass = add_type("RenderPass", sizeof(vulkan::RenderPass));
	auto TypeRenderPassP = add_type_p_raw(TypeRenderPass);
	auto TypeRenderPassXfer = add_type_p_xfer(TypeRenderPass);
	auto TypeBuffer = add_type("Buffer", sizeof(vulkan::Buffer));
	auto TypeBufferP = add_type_p_raw(TypeBuffer);
	auto TypeUniformBuffer = add_type("UniformBuffer", sizeof(vulkan::UniformBuffer));
	auto TypeDescriptorPool = add_type("DescriptorPool", sizeof(vulkan::DescriptorPool));
	auto TypeDescriptorSet = add_type("DescriptorSet", sizeof(vulkan::DescriptorSet));
	auto TypeDescriptorSetXfer = add_type_p_xfer(TypeDescriptorSet);
	auto TypeDescriptorSetP = add_type_p_raw(TypeDescriptorSet);
	auto TypeSwapChain = add_type("SwapChain", sizeof(vulkan::SwapChain));
	auto TypeSwapChainXfer = add_type_p_xfer(TypeSwapChain);
	auto TypeFence = add_type("Fence", sizeof(vulkan::Fence));
	auto TypeFenceP = add_type_p_raw(TypeFence);
	auto TypeSemaphore = add_type("Semaphore", sizeof(vulkan::Semaphore));
	auto TypeSemaphoreP = add_type_p_raw(TypeSemaphore);
	auto TypeSemaphorePList = add_type_list(TypeSemaphoreP);
	auto TypeAccelerationStructure = add_type("AccelerationStructure", sizeof(vulkan::AccelerationStructure));
	auto TypeAccelerationStructureXfer = add_type_p_xfer(TypeAccelerationStructure);
	auto TypeAccelerationStructureP = add_type_p_raw(TypeAccelerationStructure);
	auto TypeImageLayout = add_type_enum("ImageLayout");
	auto TypeAccessFlags = add_type_enum("AccessFlags");
	auto TypePipelineBindPoint = add_type_enum("BindPoint", TypePipeline);

	lib_create_list<vulkan::Texture*>(TypeTexturePList);
	lib_create_list<shared<vulkan::Texture>>(TypeTextureSharedNNList);
	lib_create_list<vulkan::Texture*>(TypeTextureXferList);
	lib_create_list<vulkan::FrameBuffer*>(TypeFrameBufferXferList);
	lib_create_list<vulkan::Shader*>(TypeShaderPList);
	lib_create_list<vulkan::Semaphore*>(TypeSemaphorePList);

	lib_create_pointer_xfer(TypeDeviceXfer);
	lib_create_pointer_xfer(TypeInstanceXfer);
	lib_create_pointer_xfer(TypeTextureXfer);
	lib_create_pointer_xfer(TypeShaderXfer);
	lib_create_pointer_xfer(TypeDepthBufferXfer);
	lib_create_pointer_xfer(TypeFrameBufferXfer);
	lib_create_pointer_xfer(TypeDescriptorSetXfer);
	lib_create_pointer_xfer(TypeCommandBufferXfer);
	lib_create_pointer_xfer(TypeRenderPassXfer);
	lib_create_pointer_xfer(TypeAccelerationStructureXfer);
	lib_create_pointer_shared<vulkan::Texture>(TypeTextureSharedNN, TypeTextureXfer);


	add_class(TypeInstance);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanInstance::__delete__), Flags::Mutable);
		class_add_func("create_glfw_surface", TypePointer, vul_p(&VulkanInstance::_create_glfw_surface), Flags::Mutable);
			func_add_param("window", TypePointer);
		class_add_func("create_headless_surface", TypePointer, vul_p(&VulkanInstance::_create_headless_surface), Flags::Mutable);


	add_class(TypeDevice);
		class_add_element("graphics_queue", TypeQueue, vul_p(&vulkan::Device::graphics_queue));
		class_add_element("present_queue", TypeQueue, vul_p(&vulkan::Device::present_queue));
		class_add_element("compute_queue", TypeQueue, vul_p(&vulkan::Device::compute_queue));
		class_add_element("command_pool", TypeCommandPoolP, vul_p(&vulkan::Device::command_pool));
		class_add_func("wait_idle", TypeVoid, vul_p(&vulkan::Device::wait_idle), Flags::Mutable);
		class_add_func("create_simple", TypeDeviceXfer, vul_p(&__vulkan_device_create_simple), Flags::Static | Flags::RaisesExceptions);
			func_add_param("instance", TypeInstanceP);
			func_add_param("surface", TypePointer);
			func_add_param("op", TypeStringList);


	add_class(TypeVertexBuffer);
		class_add_element("vertex", TypeBuffer, vul_p(&vulkan::VertexBuffer::vertex_buffer));
		class_add_element("index", TypeBuffer, vul_p(&vulkan::VertexBuffer::index_buffer));
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanVertexBuffer::__init__), Flags::Mutable);
			func_add_param("format", TypeString);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanVertexBuffer::__delete__), Flags::Mutable);
		class_add_func("update", TypeVoid, vul_p(&vulkan::VertexBuffer::update), Flags::Mutable);
			func_add_param("vertices", TypeDynamicArray);
		class_add_func("update_index", TypeVoid, vul_p(&vulkan::VertexBuffer::update_index), Flags::Mutable);
			func_add_param("indices", TypeIntList);
		class_add_func("create_quad", TypeVoid, vul_p(&vulkan::VertexBuffer::create_quad), Flags::Mutable);
			func_add_param("dest", TypeRect);
			func_add_param("source", TypeRect);



	add_class(TypeTexture);
		class_add_element(Identifier::SharedCount, TypeInt32, vul_p(&vulkan::Texture::_pointer_ref_counter));
		class_add_element("width", TypeInt32, vul_p(&vulkan::Texture::width));
		class_add_element("height", TypeInt32, vul_p(&vulkan::Texture::height));
		class_add_element("view", TypePointer, vul_p(&vulkan::Texture::view));
		//class_add_element("format", TypeInt32, vul_p(&vulkan::Texture::format));
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanTexture::__init__), Flags::Mutable);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanTexture::__init_ext__), Flags::Mutable);
			func_add_param("w", TypeInt32);
			func_add_param("h", TypeInt32);
			func_add_param("format", TypeString);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanTexture::__delete__), Flags::Mutable);
		class_add_func("write", TypeVoid, vul_p(&vulkan::Texture::write), Flags::Mutable);
			func_add_param("image", TypeImage);
		class_add_func("write", TypeVoid, vul_p(&vulkan::Texture::writex), Flags::Mutable);
			func_add_param("data", TypeReference);
			func_add_param("nx", TypeInt32);
			func_add_param("ny", TypeInt32);
			func_add_param("nz", TypeInt32);
			func_add_param("format", TypeString);
		class_add_func("load", TypeTextureXfer, vul_p(&__vulkan_load_texture), Flags::Static);
			func_add_param("filename", TypePath);


	add_class(TypeVolumeTexture);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanVolumeTexture::__init__), Flags::Mutable);
			func_add_param("nx", TypeInt32);
			func_add_param("ny", TypeInt32);
			func_add_param("nz", TypeInt32);
			func_add_param("format", TypeString);


	add_class(TypeCubeMap);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanCubeMap::__init__), Flags::Mutable);
			func_add_param("size", TypeInt32);
			func_add_param("format", TypeString);


	add_class(TypeStorageTexture);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanStorageTexture::__init__), Flags::Mutable);
			func_add_param("nx", TypeInt32);
			func_add_param("ny", TypeInt32);
			func_add_param("nz", TypeInt32);
			func_add_param("format", TypeString);


	add_class(TypeDepthBuffer);
		class_derive_from(TypeTexture);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanDepthBuffer::__init__), Flags::Mutable);
			func_add_param("w", TypeInt32);
			func_add_param("h", TypeInt32);
			func_add_param("format", TypeString);
			func_add_param("with_sampler", TypeBool);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanTexture::__delete__), Flags::Override | Flags::Mutable);


	add_class(TypeFrameBuffer);
		class_add_element(Identifier::SharedCount, TypeInt32, vul_p(&vulkan::FrameBuffer::_pointer_ref_counter));
		class_add_element("width", TypeInt32, vul_p(&vulkan::FrameBuffer::width));
		class_add_element("height", TypeInt32, vul_p(&vulkan::FrameBuffer::height));
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanFrameBuffer::__init__), Flags::Mutable);
			func_add_param("rp", TypeRenderPassP);
			func_add_param("attachments", TypeTextureSharedNNList);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanFrameBuffer::__delete__), Flags::Mutable);
		class_add_func("area", TypeRect, vul_p(&vulkan::FrameBuffer::area));


	add_class(TypeShader);
		class_add_element(Identifier::SharedCount, TypeInt32, vul_p(&vulkan::Shader::_pointer_ref_counter));
		//class_add_element("descr_layout", TypePointerList, vul_p(&vulkan::Shader::descr_layouts));
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanShader::__init__), Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanShader::__delete__), Flags::Mutable);
		class_add_func("load", TypeShaderXfer, vul_p(&__vulkan_load_shader), Flags::Static);
			func_add_param("filename", TypePath);


	add_class(TypeUniformBuffer);
		class_derive_from(TypeBuffer);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanUniformBuffer::__init__), Flags::Mutable);
			func_add_param("size", TypeInt32);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanUniformBuffer::__delete__), Flags::Mutable);
		class_add_func("update", TypeVoid, vul_p(&vulkan::UniformBuffer::update), Flags::Mutable);
			func_add_param("source", TypeReference);


	add_class(TypeDescriptorPool);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanDescriptorPool::__init__), Flags::Mutable);
			func_add_param("s", TypeString);
			func_add_param("max_sets", TypeInt32);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanDescriptorPool::__delete__), Flags::Mutable);
		class_add_func("create_set", TypeDescriptorSetXfer, vul_p(&vulkan::DescriptorPool::_create_set_str));
			func_add_param("bindings", TypeString);


	add_class(TypeDescriptorSet);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanDescriptorSet::__delete__), Flags::Mutable);
		class_add_func("update", TypeVoid, vul_p(&vulkan::DescriptorSet::update), Flags::Mutable);
		class_add_func("set_texture", TypeVoid, vul_p(&vulkan::DescriptorSet::set_texture), Flags::Mutable);
			func_add_param("binding", TypeInt32);
			func_add_param("tex", TypeTextureP);
		class_add_func("set_storage_image", TypeVoid, vul_p(&vulkan::DescriptorSet::set_storage_image), Flags::Mutable);
			func_add_param("binding", TypeInt32);
			func_add_param("tex", TypeTextureP);
		class_add_func("set_buffer", TypeVoid, vul_p(&vulkan::DescriptorSet::set_buffer), Flags::Mutable);
			func_add_param("binding", TypeInt32);
			func_add_param("buf", TypeBufferP);
		class_add_func("set_acceleration_structure", TypeVoid, vul_p(&vulkan::DescriptorSet::set_acceleration_structure), Flags::Mutable);
			func_add_param("binding", TypeInt32);
			func_add_param("as", TypeAccelerationStructureP);


	add_class(TypeGraphicsPipeline);
		class_derive_from(TypePipeline);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanGraphicsPipeline::__init__), Flags::Mutable);
			func_add_param("shader", TypeShaderP);
			func_add_param("pass", TypeRenderPassP);
			func_add_param("subpass", TypeInt32);
			func_add_param("topology", TypeString);
			func_add_param("format", TypeString);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanGraphicsPipeline::__delete__), Flags::Mutable);
		class_add_func("set_wireframe", TypeVoid, vul_p(&vulkan::GraphicsPipeline::set_wireframe), Flags::Mutable);
			func_add_param("w", TypeBool);
		class_add_func("set_line_width", TypeVoid, vul_p(&vulkan::GraphicsPipeline::set_line_width), Flags::Mutable);
			func_add_param("w", TypeFloat32);
#ifdef KABA_EXPORT_VULKAN
		void (vulkan::GraphicsPipeline::*mpf)(float) = &vulkan::GraphicsPipeline::set_blend;
		class_add_func("set_blend", TypeVoid, vul_p(mpf), Flags::Mutable);
#else
		class_add_func("set_blend", TypeVoid, nullptr, Flags::Mutable);
#endif
			func_add_param("alpha", TypeFloat32);
#ifdef KABA_EXPORT_VULKAN
		void (vulkan::GraphicsPipeline::*mpf2)(vulkan::Alpha, vulkan::Alpha) = &vulkan::GraphicsPipeline::set_blend;
		class_add_func("set_blend", TypeVoid, vul_p(mpf2), Flags::Mutable);
#else
		class_add_func("set_blend", TypeVoid, nullptr, Flags::Mutable);
#endif
			func_add_param("src", TypeInt32);
			func_add_param("dst", TypeInt32);
		class_add_func("set_z", TypeVoid, vul_p(&vulkan::GraphicsPipeline::set_z), Flags::Mutable);
			func_add_param("test", TypeBool);
			func_add_param("write", TypeBool);
		class_add_func("set_dynamic", TypeVoid, vul_p(&vulkan::GraphicsPipeline::set_dynamic), Flags::Mutable);
			func_add_param("mode", TypeIntList);
		class_add_func("rebuild", TypeVoid, vul_p(&vulkan::GraphicsPipeline::rebuild), Flags::Mutable);


	add_class(TypeComputePipeline);
		class_derive_from(TypePipeline);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanComputePipeline::__init__), Flags::Mutable);
			func_add_param("shader", TypeShaderP);


	add_class(TypeRayPipeline);
		class_derive_from(TypePipeline);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanRayPipeline::__init__), Flags::Mutable);
			func_add_param("layout", TypeString);
			func_add_param("shader", TypeShaderPList);
			func_add_param("recursion_depth", TypeInt32);
		class_add_func("create_sbt", TypeVoid, vul_p(&vulkan::RayPipeline::create_sbt), Flags::Mutable);


	add_class(TypeRenderPass);
		class_add_element("clear_color", TypeColorList, vul_p(&vulkan::RenderPass::clear_color), Flags::Mutable);
		class_add_element("clear_z", TypeFloat32, vul_p(&vulkan::RenderPass::clear_z), Flags::Mutable);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanRenderPass::__init__), Flags::Mutable);
			func_add_param("formats", TypeStringList);
			func_add_param("options", TypeStringList);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanRenderPass::__delete__), Flags::Mutable);
		class_add_func("rebuild", TypeVoid, vul_p(&vulkan::RenderPass::rebuild), Flags::Mutable);
		class_add_func("add_subpass", TypeVoid, vul_p(&vulkan::RenderPass::add_subpass), Flags::Mutable);
			func_add_param("color_att", TypeIntList);
			func_add_param("depth_att", TypeInt32);
		class_add_func("add_dependency", TypeVoid, vul_p(&vulkan::RenderPass::add_dependency), Flags::Mutable);
			func_add_param("src", TypeInt32);
			func_add_param("src_opt", TypeString);
			func_add_param("dst", TypeInt32);
			func_add_param("dst_opt", TypeString);


	add_class(TypeSwapChain);
		class_add_element("width", TypeInt32, vul_p(&vulkan::SwapChain::width));
		class_add_element("height", TypeInt32, vul_p(&vulkan::SwapChain::height));
		class_add_element("format", TypeInt32, vul_p(&vulkan::SwapChain::image_format));
		class_add_func("create_for_glfw", TypeSwapChainXfer, vul_p(&VulkanSwapChain::_create_for_glfw), Flags::Static | Flags::RaisesExceptions);
			func_add_param("device", TypeDeviceP);
			func_add_param("win", TypePointer);
		class_add_func("create", TypeSwapChainXfer, vul_p(&VulkanSwapChain::_create), Flags::Static | Flags::RaisesExceptions);
			func_add_param("device", TypeDeviceP);
			func_add_param("width", TypeInt32);
			func_add_param("height", TypeInt32);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanSwapChain::__delete__), Flags::Mutable);
		class_add_func("create_depth_buffer", TypeDepthBufferXfer, vul_p(&vulkan::SwapChain::create_depth_buffer), Flags::Mutable);
		class_add_func("create_render_pass", TypeRenderPassXfer, vul_p(&vulkan::SwapChain::create_render_pass), Flags::Mutable);
			func_add_param("depth_buffer", TypeDepthBufferP);
			func_add_param("options", TypeStringList);
		class_add_func("create_frame_buffers", TypeFrameBufferXferList, vul_p(&vulkan::SwapChain::create_frame_buffers), Flags::Mutable);
			func_add_param("render_pass", TypeRenderPassP);
			func_add_param("depth_buffer", TypeDepthBufferP);
		class_add_func("create_textures", TypeTextureXferList, vul_p(&vulkan::SwapChain::create_textures), Flags::Mutable);
		class_add_func("present", TypeBool, vul_p(&vulkan::SwapChain::present), Flags::Mutable);
			func_add_param("image_index", TypeInt32);
			func_add_param("wait_sem", TypeSemaphorePList);
		class_add_func("acquire_image", TypeIntOptional, vul_p(&VulkanSwapChain::acquire_image_x), Flags::Mutable);
			func_add_param("signal_sem", TypeSemaphoreP);


	add_class(TypeFence);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanFence::__init__), Flags::Mutable);
			func_add_param("device", TypeDeviceP);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanFence::__delete__), Flags::Mutable);
		class_add_func("wait", TypeVoid, vul_p(&vulkan::Fence::wait), Flags::Mutable);
		class_add_func("reset", TypeVoid, vul_p(&vulkan::Fence::reset), Flags::Mutable);


	add_class(TypeSemaphore);
		class_add_func(Identifier::func::Init, TypeVoid, vul_p(&VulkanSemaphore::__init__), Flags::Mutable);
			func_add_param("device", TypeDeviceP);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanSemaphore::__delete__), Flags::Mutable);


	add_class(TypeCommandPool);
		//class_add_func(Identifier::Func::DELETE, TypeVoid, vul_p(&VulkanCommandPool::__delete__));
		class_add_func("create_command_buffer", TypeCommandBufferXfer, vul_p(&vulkan::CommandPool::create_command_buffer), Flags::Mutable);


	add_class(TypeCommandBuffer);
		class_add_func(Identifier::func::Delete, TypeVoid, vul_p(&VulkanCommandBuffer::__delete__), Flags::Mutable);
		class_add_func("begin", TypeVoid, vul_p(&vulkan::CommandBuffer::begin), Flags::Mutable);
		class_add_func("end", TypeVoid, vul_p(&vulkan::CommandBuffer::end), Flags::Mutable);
		class_add_func("set_bind_point", TypeVoid, vul_p(&vulkan::CommandBuffer::set_bind_point), Flags::Mutable);
			func_add_param("bp", TypePipelineBindPoint);
		class_add_func("clear", TypeVoid, vul_p(&vulkan::CommandBuffer::clear), Flags::Mutable);
			func_add_param("area", TypeRect);
			func_add_param("colors", TypeColorList);
			func_add_param("z", TypeFloatOptional);
		class_add_func("bind_pipeline", TypeVoid, vul_p(&vulkan::CommandBuffer::bind_pipeline), Flags::Mutable);
			func_add_param("p", TypePipelineP);
		class_add_func("draw", TypeVoid, vul_p(&vulkan::CommandBuffer::draw), Flags::Mutable);
			func_add_param("vb", TypeVertexBufferP);
		class_add_func("begin_render_pass", TypeVoid, vul_p(&vulkan::CommandBuffer::begin_render_pass), Flags::Mutable);
			func_add_param("rp", TypeRenderPassP);
			func_add_param("fb", TypeFrameBufferP);
		class_add_func("next_subpass", TypeVoid, vul_p(&vulkan::CommandBuffer::next_subpass), Flags::Mutable);
		class_add_func("end_render_pass", TypeVoid, vul_p(&vulkan::CommandBuffer::end_render_pass), Flags::Mutable);
		class_add_func("push_constant", TypeVoid, vul_p(&vulkan::CommandBuffer::push_constant), Flags::Mutable);
			func_add_param("offset", TypeInt32);
			func_add_param("size", TypeInt32);
			func_add_param("data", TypeReference);
		class_add_func("bind_descriptor_set", TypeVoid, vul_p(&vulkan::CommandBuffer::bind_descriptor_set), Flags::Mutable);
			func_add_param("index", TypeInt32);
			func_add_param("set", TypeDescriptorSetP);
		class_add_func("set_scissor", TypeVoid, vul_p(&vulkan::CommandBuffer::set_scissor), Flags::Mutable);
			func_add_param("r", TypeRect);
		class_add_func("set_viewport", TypeVoid, vul_p(&vulkan::CommandBuffer::set_viewport), Flags::Mutable);
			func_add_param("r", TypeRect);
		class_add_func("dispatch", TypeVoid, vul_p(&vulkan::CommandBuffer::dispatch), Flags::Mutable);
			func_add_param("nx", TypeInt32);
			func_add_param("ny", TypeInt32);
			func_add_param("nz", TypeInt32);
		class_add_func("barrier", TypeVoid, vul_p(&vulkan::CommandBuffer::barrier), Flags::Mutable);
			func_add_param("t", TypeTexturePList);
			func_add_param("mode", TypeInt32);
		class_add_func("image_barrier", TypeVoid, vul_p(&vulkan::CommandBuffer::image_barrier), Flags::Mutable);
			func_add_param("t", TypeTextureP);
			func_add_param("src_access", TypeAccessFlags);
			func_add_param("dst_access", TypeAccessFlags);
			func_add_param("old_layout", TypeImageLayout);
			func_add_param("new_layout", TypeImageLayout);
		class_add_func("copy_image", TypeVoid, vul_p(&vulkan::CommandBuffer::copy_image), Flags::Mutable);
			func_add_param("src", TypeTextureP);
			func_add_param("dst", TypeTextureP);
			func_add_param("extend", TypeIntList);
		class_add_func("trace_rays", TypeVoid, vul_p(&vulkan::CommandBuffer::trace_rays), Flags::Mutable);
			func_add_param("nx", TypeInt32);
			func_add_param("ny", TypeInt32);
			func_add_param("nz", TypeInt32);


	add_class(TypeAccelerationStructure);
		class_add_func("create_top", TypeAccelerationStructureXfer, vul_p(&vulkan::AccelerationStructure::create_top), Flags::Static);
			func_add_param("device", TypeDeviceP);
			func_add_param("instances", TypeDynamicArray);
			func_add_param("matrices", TypeDynamicArray);
		class_add_func("create_bottom", TypeAccelerationStructureXfer, vul_p(&vulkan::AccelerationStructure::create_bottom), Flags::Static);
			func_add_param("device", TypeDevice);
			func_add_param("vb", TypeVertexBuffer);


	add_class(TypeQueue);
		class_add_func("submit", TypeVoid, vul_p(&vulkan::Queue::submit), Flags::Mutable);
			func_add_param("cb", TypeCommandBufferP);
			func_add_param("wait_sem", TypeSemaphorePList);
			func_add_param("signal_sem", TypeSemaphorePList);
			func_add_param("fence", TypeFenceP);


	add_class(TypeAccessFlags);
		class_add_enum("NONE", TypeAccessFlags, vul_p(vulkan::AccessFlags::NONE));
		class_add_enum("SHADER_WRITE_BIT", TypeAccessFlags, vul_p(vulkan::AccessFlags::SHADER_WRITE_BIT));
		class_add_enum("TRANSFER_READ_BIT", TypeAccessFlags, vul_p(vulkan::AccessFlags::TRANSFER_READ_BIT));
		class_add_enum("TRANSFER_WRITE_BIT", TypeAccessFlags, vul_p(vulkan::AccessFlags::TRANSFER_WRITE_BIT));

	add_class(TypeImageLayout);
		class_add_enum("UNDEFINED", TypeImageLayout, vul_p(vulkan::ImageLayout::UNDEFINED));
		class_add_enum("GENERAL", TypeImageLayout, vul_p(vulkan::ImageLayout::GENERAL));
		class_add_enum("TRANSFER_SRC_OPTIMAL", TypeImageLayout, vul_p(vulkan::ImageLayout::TRANSFER_SRC_OPTIMAL));
		class_add_enum("TRANSFER_DST_OPTIMAL", TypeImageLayout, vul_p(vulkan::ImageLayout::TRANSFER_DST_OPTIMAL));
		class_add_enum("PRESENT_SRC", TypeImageLayout, vul_p(vulkan::ImageLayout::PRESENT_SRC));

	add_class(TypePipelineBindPoint);
		class_add_enum("GRAPHICS", TypePipelineBindPoint, vul_p(vulkan::PipelineBindPoint::GRAPHICS));
		class_add_enum("RAY_TRACING", TypePipelineBindPoint, vul_p(vulkan::PipelineBindPoint::RAY_TRACING));
		class_add_enum("COMPUTE", TypePipelineBindPoint, vul_p(vulkan::PipelineBindPoint::COMPUTE));



	lib_create_list<shared<vulkan::Texture>>(TypeTextureSharedNNList);

#ifdef HAS_LIB_GLFW
	add_func("create_window", TypeReference, vul_p(&vulkan::create_window), Flags::Static);
		func_add_param("title", TypeString);
		func_add_param("w", TypeInt32);
		func_add_param("h", TypeInt32);
	add_func("window_handle", TypeBool, vul_p(&vulkan::window_handle), Flags::Static);
		func_add_param("w", TypePointer);
	add_func("window_close", TypeVoid, vul_p(&vulkan::window_close), Flags::Static);
		func_add_param("w", TypePointer);
#else
#endif

	add_func("init", TypeInstanceXfer, vul_p(&__vulkan_init), Flags::Static | Flags::RaisesExceptions);
		func_add_param("op", TypeStringList);


	add_ext_var("default_device", TypeDeviceP, vul_p(&vulkan::default_device));
}

};
