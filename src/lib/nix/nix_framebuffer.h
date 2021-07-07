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

class FrameBuffer : public Sharable<Empty> {
public:
	FrameBuffer();
	FrameBuffer(const Array<Texture*> &attachments);
	~FrameBuffer();
	void __init__(const Array<Texture*> &attachments);
	void __delete__();
	void update(const Array<Texture*> &attachments);
	void update_x(const Array<Texture*> &attachments, int cube_face);
	Array<Texture*> color_attachments;
	DepthBuffer *depth_buffer;
	unsigned int frame_buffer;
	int width, height;
	int multi_samples;
	rect area() const;

	void clear_color(int index, const color &c);
	void clear_depth(float depth);

	void _check();

	static FrameBuffer *DEFAULT;
};

void bind_frame_buffer(FrameBuffer *fb);


};

#endif
