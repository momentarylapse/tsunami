/*
 * nix_buffer.h
 *
 *  Created on: Jun 25, 2021
 *      Author: michi
 */

#if HAS_LIB_GL

#pragma once

namespace nix{


class Buffer {
public:
	enum class Type {
		NONE,
		UNIFORM,
		SSBO
	} type;
	unsigned int buffer;

	Buffer();
	~Buffer();

	void __delete__();
	void update(void *data, int size);
	void update_array(const DynamicArray &a);

	void read(void *data, int size);
	void read_array(DynamicArray &a);
};

class UniformBuffer : public Buffer {
public:
	UniformBuffer();
	void __init__();
};

class ShaderStorageBuffer : public Buffer {
public:
	ShaderStorageBuffer();
	void __init__();
};

void bind_buffer(Buffer *buf, int binding);

};

#endif
