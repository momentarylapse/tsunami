#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "shared.h"

#ifdef _X_USE_IMAGE_
	#include "../../image/image.h"
	#include "../../image/ImagePainter.h"
#else
	#error "we're screwed"
#endif

namespace kaba {



void SIAddPackageImage(Context *c) {
	add_internal_package(c, "image", "1");

	common_types.image = add_type("Image", sizeof(Image));
	auto TypeImageXfer = add_type_p_xfer(common_types.image);
	common_types.base_painter = add_type("Painter", sizeof(Painter), Flags::None);
	common_types.base_painter_p = add_type_p_raw(common_types.base_painter);
	common_types.base_painter_xfer = add_type_p_xfer(common_types.base_painter);

	lib_create_pointer_xfer(TypeImageXfer);
	lib_create_pointer_xfer(common_types.base_painter_xfer);

	
	add_class(common_types.image);
		class_add_element("width", common_types.i32, &Image::width);
		class_add_element("height", common_types.i32, &Image::height);
		class_add_element("mode", common_types.i32, &Image::mode);
		class_add_element("data", common_types.i32_list, &Image::data);
		class_add_element("error", common_types._bool, &Image::error);
		class_add_element("alpha_used", common_types._bool, &Image::alpha_used);
		class_add_func(Identifier::func::Init, common_types._void, &Image::__init_ext__, Flags::Mutable);
			func_add_param("width", common_types.i32);
			func_add_param("height", common_types.i32);
			func_add_param("c", common_types.color);
		class_add_func(Identifier::func::Init, common_types._void, &Image::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, common_types._void, &Image::__delete__, Flags::Mutable);
		class_add_func("create", common_types._void, &Image::create, Flags::Mutable);
			func_add_param("width", common_types.i32);
			func_add_param("height", common_types.i32);
			func_add_param("c", common_types.color);
		class_add_func("load", TypeImageXfer, &Image::load, Flags::Static);
			func_add_param("filename", common_types.path);
		class_add_func("save", common_types._void, &Image::save);
			func_add_param("filename", common_types.path);
		class_add_func("scale", TypeImageXfer, &Image::scale);
			func_add_param("width", common_types.i32);
			func_add_param("height", common_types.i32);
		class_add_func("set_pixel", common_types._void, &Image::set_pixel, Flags::Mutable);
			func_add_param("x", common_types.i32);
			func_add_param("y", common_types.i32);
			func_add_param("c", common_types.color);
		class_add_func("get_pixel", common_types.color, &Image::get_pixel, Flags::Pure);
			func_add_param("x", common_types.i32);
			func_add_param("y", common_types.i32);
		class_add_func("get_pixel_smooth", common_types.color, &Image::get_pixel_interpolated, Flags::Pure);
			func_add_param("x", common_types.f32);
			func_add_param("y", common_types.f32);
		class_add_func("clear", common_types._void, &Image::clear, Flags::Mutable);
		class_add_func(Identifier::func::Assign, common_types._void, &Image::__assign__, Flags::Mutable);
			func_add_param("other", common_types.image);
		class_add_func("start_draw", common_types.base_painter_xfer, &Image::start_draw, Flags::Mutable);


	add_class(common_types.base_painter);
		class_derive_from(common_types.object, DeriveFlags::COPY_VTABLE);
		class_add_element("width", common_types.i32, &Painter::width);
		class_add_element("height", common_types.i32, &Painter::height);
		class_add_func_virtual(Identifier::func::Delete, common_types._void, &ImagePainter::__delete__, Flags::Mutable | Flags::Override);
	//class_add_func_virtual("end", common_types._void, &HuiPainter::end));
		class_add_func_virtual("area", common_types.rect, &Painter::area);
		class_add_func_virtual("set_color", common_types._void, &Painter::set_color); // Flags::MUTABLE ...nope... let's allow const references for now...
			func_add_param("c", common_types.color);
		class_add_func_virtual("set_line_width", common_types._void, &Painter::set_line_width);
			func_add_param("w", common_types.f32);
		class_add_func_virtual("set_line_dash", common_types._void, &Painter::set_line_dash);
			func_add_param("w", common_types.f32_list);
		class_add_func_virtual("set_roundness", common_types._void, &Painter::set_roundness);
			func_add_param("r", common_types.f32);
		class_add_func_virtual("set_antialiasing", common_types._void, &Painter::set_antialiasing);
			func_add_param("enabled", common_types._bool);
		class_add_func("set_contiguous", common_types._void, &Painter::set_contiguous);
			func_add_param("contiguous", common_types._bool);
		class_add_func_virtual("set_font", common_types._void, &Painter::set_font);
			func_add_param("font", common_types.string);
			func_add_param("size", common_types.f32);
			func_add_param("bold", common_types._bool);
			func_add_param("italic", common_types._bool);
		class_add_func_virtual("set_font_size", common_types._void, &Painter::set_font_size);
			func_add_param("size", common_types.f32);
		class_add_func_virtual("set_fill", common_types._void, &Painter::set_fill);
			func_add_param("fill", common_types._bool);
		class_add_func_virtual("set_clip", common_types._void, &Painter::set_clip);
			func_add_param("r", common_types.rect);
		class_add_func_virtual("draw_point", common_types._void, &Painter::draw_point);
			func_add_param("p", common_types.vec2);
		class_add_func_virtual("draw_line", common_types._void, &Painter::draw_line);
			func_add_param("a", common_types.vec2);
			func_add_param("b", common_types.vec2);
		class_add_func_virtual("draw_lines", common_types._void, &Painter::draw_lines);
			func_add_param("p", common_types.vec2_list);
		class_add_func_virtual("draw_polygon", common_types._void, &Painter::draw_polygon);
			func_add_param("p", common_types.vec2_list);
		class_add_func_virtual("draw_rect", common_types._void, &Painter::draw_rect);
			func_add_param("r", common_types.rect);
		class_add_func_virtual("draw_circle", common_types._void, &Painter::draw_circle);
			func_add_param("p", common_types.vec2);
			func_add_param("r", common_types.f32);
		class_add_func_virtual("draw_str", common_types._void, &Painter::draw_str);
			func_add_param("p", common_types.vec2);
			func_add_param("str", common_types.string);
		class_add_func_virtual("get_str_width", common_types.f32, &Painter::get_str_width);
			func_add_param("str", common_types.string);
		class_add_func_virtual("draw_image", common_types._void, &Painter::draw_image);
			func_add_param("p", common_types.vec2);
			func_add_param("image", common_types.image);
		class_add_func_virtual("set_option", common_types._void, &Painter::set_option);
			func_add_param("key", common_types.string);
			func_add_param("value", common_types.string);
		class_set_vtable(ImagePainter);
}

};
