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

extern const Class *TypeImage;
extern const Class *TypeIntList;
extern const Class *TypeFloatList;
extern const Class *TypeVec2List;
extern const Class *TypeVec2;
extern const Class *TypeObject;
extern const Class *TypePath;

const Class *TypeBasePainter;
const Class *TypeBasePainterP;
const Class *TypeBasePainterXfer;



void SIAddPackageImage(Context *c) {
	add_package(c, "image");

	TypeImage = add_type("Image", sizeof(Image));
	auto TypeImageXfer = add_type_p_xfer(TypeImage);
	TypeBasePainter = add_type("Painter", sizeof(Painter), Flags::None, TypeImage);
	TypeBasePainterP = add_type_p_raw(TypeBasePainter);
	TypeBasePainterXfer = add_type_p_xfer(TypeBasePainter);

	lib_create_pointer_xfer(TypeImageXfer);
	lib_create_pointer_xfer(TypeBasePainterXfer);

	
	add_class(TypeImage);
		class_add_element("width", TypeInt32, &Image::width);
		class_add_element("height", TypeInt32, &Image::height);
		class_add_element("mode", TypeInt32, &Image::mode);
		class_add_element("data", TypeIntList, &Image::data);
		class_add_element("error", TypeBool, &Image::error);
		class_add_element("alpha_used", TypeBool, &Image::alpha_used);
		class_add_func(Identifier::func::Init, TypeVoid, &Image::__init_ext__, Flags::Mutable);
			func_add_param("width", TypeInt32);
			func_add_param("height", TypeInt32);
			func_add_param("c", TypeColor);
		class_add_func(Identifier::func::Init, TypeVoid, &Image::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, &Image::__delete__, Flags::Mutable);
		class_add_func("create", TypeVoid, &Image::create, Flags::Mutable);
			func_add_param("width", TypeInt32);
			func_add_param("height", TypeInt32);
			func_add_param("c", TypeColor);
		class_add_func("load", TypeImageXfer, &Image::load, Flags::Static);
			func_add_param("filename", TypePath);
		class_add_func("save", TypeVoid, &Image::save);
			func_add_param("filename", TypePath);
		class_add_func("scale", TypeImageXfer, &Image::scale);
			func_add_param("width", TypeInt32);
			func_add_param("height", TypeInt32);
		class_add_func("set_pixel", TypeVoid, &Image::set_pixel, Flags::Mutable);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("c", TypeColor);
		class_add_func("get_pixel", TypeColor, &Image::get_pixel, Flags::Pure);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
		class_add_func("get_pixel_smooth", TypeColor, &Image::get_pixel_interpolated, Flags::Pure);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_func("clear", TypeVoid, &Image::clear, Flags::Mutable);
		class_add_func(Identifier::func::Assign, TypeVoid, &Image::__assign__, Flags::Mutable);
			func_add_param("other", TypeImage);
		class_add_func("start_draw", TypeBasePainterXfer, &Image::start_draw, Flags::Mutable);


	add_class(TypeBasePainter);
		class_derive_from(TypeObject, DeriveFlags::COPY_VTABLE);
		class_add_element("width", TypeInt32, &Painter::width);
		class_add_element("height", TypeInt32, &Painter::height);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, &ImagePainter::__delete__, Flags::Mutable);
		//class_add_func_virtual("end", TypeVoid, &HuiPainter::end));
		class_add_func_virtual("set_color", TypeVoid, &Painter::set_color); // Flags::MUTABLE ...nope... let's allow const references for now...
			func_add_param("c", TypeColor);
		class_add_func_virtual("set_line_width", TypeVoid, &Painter::set_line_width);
			func_add_param("w", TypeFloat32);
		class_add_func_virtual("set_line_dash", TypeVoid, &Painter::set_line_dash);
			func_add_param("w", TypeFloatList);
		class_add_func_virtual("set_roundness", TypeVoid, &Painter::set_roundness);
			func_add_param("r", TypeFloat32);
		class_add_func_virtual("set_antialiasing", TypeVoid, &Painter::set_antialiasing);
			func_add_param("enabled", TypeBool);
		class_add_func_virtual("set_font", TypeVoid, &Painter::set_font);
			func_add_param("font", TypeString);
			func_add_param("size", TypeFloat32);
			func_add_param("bold", TypeBool);
			func_add_param("italic", TypeBool);
		class_add_func_virtual("set_font_size", TypeVoid, &Painter::set_font_size);
			func_add_param("size", TypeFloat32);
		class_add_func_virtual("set_fill", TypeVoid, &Painter::set_fill);
			func_add_param("fill", TypeBool);
		class_add_func_virtual("clip", TypeVoid, &Painter::set_clip);
			func_add_param("r", TypeRect);
		class_add_func_virtual("draw_point", TypeVoid, &Painter::draw_point);
			func_add_param("p", TypeVec2);
		class_add_func_virtual("draw_line", TypeVoid, &Painter::draw_line);
			func_add_param("a", TypeVec2);
			func_add_param("b", TypeVec2);
		class_add_func_virtual("draw_lines", TypeVoid, &Painter::draw_lines);
			func_add_param("p", TypeVec2List);
		class_add_func_virtual("draw_polygon", TypeVoid, &Painter::draw_polygon);
			func_add_param("p", TypeVec2List);
		class_add_func_virtual("draw_rect", TypeVoid, &Painter::draw_rect);
			func_add_param("r", TypeRect);
		class_add_func_virtual("draw_circle", TypeVoid, &Painter::draw_circle);
			func_add_param("p", TypeVec2);
			func_add_param("r", TypeFloat32);
		class_add_func_virtual("draw_str", TypeVoid, &Painter::draw_str);
			func_add_param("p", TypeVec2);
			func_add_param("str", TypeString);
		class_add_func_virtual("get_str_width", TypeFloat32, &Painter::get_str_width);
			func_add_param("str", TypeString);
		class_add_func_virtual("draw_image", TypeVoid, &Painter::draw_image);
			func_add_param("p", TypeVec2);
			func_add_param("image", TypeImage);
		class_add_func_virtual("set_option", TypeVoid, &Painter::set_option);
			func_add_param("key", TypeString);
			func_add_param("value", TypeString);
		class_set_vtable(ImagePainter);
}

};
