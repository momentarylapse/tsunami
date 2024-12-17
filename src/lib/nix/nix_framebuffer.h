/*
 * nix_framebuffer.h
 *
 *  Created on: Jun 25, 2021
 *      Author: michi
 */


#if HAS_LIB_GL

#pragma once

namespace nix{

class Texture;
class DepthBuffer;

class FrameBuffer : public Sharable<base::Empty> {
public:
	FrameBuffer();
	// wrapper for pre-defined frame buffers:
	FrameBuffer(int index, int width, int height);
	explicit FrameBuffer(const shared_array<Texture> &attachments);
	~FrameBuffer();

	void update(const shared_array<Texture> &attachments);
	void update_x(const shared_array<Texture> &attachments, int cube_face);

	shared_array<Texture> color_attachments;
	shared<DepthBuffer> depth_buffer;
	unsigned int frame_buffer;
	int width, height;
	int multi_samples;
	rect area() const;
	bool is_srgb() const;

	void clear_color(int index, const color &c);
	void clear_depth(float depth);

	void _check();

	void read(Image &image) const;
};

void bind_frame_buffer(FrameBuffer *fb);

bool get_srgb();
void set_srgb(bool enabled);


};

#endif
