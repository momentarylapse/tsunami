#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "lib.h"

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
extern const Class *TypeComplexList;
extern const Class *TypeObject;
extern const Class *TypePath;

const Class *TypeBasePainter;
const Class *TypeBasePainterP;



void SIAddPackageImage() {
	add_package("image");

	TypeImage = add_type("Image", sizeof(Image));
	auto TypeImageP = add_type_p(TypeImage);
	TypeBasePainter = add_type("Painter", sizeof(Painter), Flags::NONE, TypeImage);
	TypeBasePainterP = add_type_p(TypeBasePainter);

	
	add_class(TypeImage);
		class_add_elementx("width", TypeInt, &Image::width);
		class_add_elementx("height", TypeInt, &Image::height);
		class_add_elementx("mode", TypeInt, &Image::mode);
		class_add_elementx("data", TypeIntList, &Image::data);
		class_add_elementx("error", TypeBool, &Image::error);
		class_add_elementx("alpha_used", TypeBool, &Image::alpha_used);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Image::__init_ext__);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("c", TypeColor);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Image::__init__);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &Image::__delete__);
		class_add_funcx("create", TypeVoid, &Image::create);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("c", TypeColor);
		class_add_funcx("load", TypeImageP, &Image::load, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_funcx("save", TypeVoid, &Image::save);
			func_add_param("filename", TypePath);
		class_add_funcx("scale", TypeImageP, &Image::scale, Flags::CONST);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
		class_add_funcx("set_pixel", TypeVoid, &Image::set_pixel);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("c", TypeColor);
		class_add_funcx("get_pixel", TypeColor, &Image::get_pixel, Flags::PURE);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
		class_add_funcx("get_pixel_smooth", TypeColor, &Image::get_pixel_interpolated, Flags::PURE);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_funcx("clear", TypeVoid, &Image::clear);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &Image::__assign__);
			func_add_param("other", TypeImage);
		class_add_funcx("start_draw", TypeBasePainterP, &Image::start_draw);


		add_class(TypeBasePainter);
			class_derive_from(TypeObject, false, true);
			class_add_elementx("width", TypeInt, &Painter::width);
			class_add_elementx("height", TypeInt, &Painter::height);
			class_add_func_virtualx(IDENTIFIER_FUNC_DELETE, TypeVoid, &ImagePainter::__delete__);
			//class_add_func_virtualx("end", TypeVoid, &HuiPainter::end));
			class_add_func_virtualx("set_color", TypeVoid, &Painter::set_color, Flags::CONST);
				func_add_param("c", TypeColor);
			class_add_func_virtualx("set_line_width", TypeVoid, &Painter::set_line_width, Flags::CONST);
				func_add_param("w", TypeFloat32);
			class_add_func_virtualx("set_line_dash", TypeVoid, &Painter::set_line_dash, Flags::CONST);
				func_add_param("w", TypeFloatList);
			class_add_func_virtualx("set_roundness", TypeVoid, &Painter::set_roundness, Flags::CONST);
				func_add_param("r", TypeFloat32);
			class_add_func_virtualx("set_antialiasing", TypeVoid, &Painter::set_antialiasing, Flags::CONST);
				func_add_param("enabled", TypeBool);
			class_add_func_virtualx("set_font", TypeVoid, &Painter::set_font, Flags::CONST);
				func_add_param("font", TypeString);
				func_add_param("size", TypeFloat32);
				func_add_param("bold", TypeBool);
				func_add_param("italic", TypeBool);
			class_add_func_virtualx("set_font_size", TypeVoid, &Painter::set_font_size, Flags::CONST);
				func_add_param("size", TypeFloat32);
			class_add_func_virtualx("set_fill", TypeVoid, &Painter::set_fill, Flags::CONST);
				func_add_param("fill", TypeBool);
			class_add_func_virtualx("clip", TypeVoid, &Painter::set_clip, Flags::CONST);
				func_add_param("r", TypeRect);
			class_add_func_virtualx("draw_point", TypeVoid, &Painter::draw_point, Flags::CONST);
				func_add_param("x", TypeFloat32);
				func_add_param("y", TypeFloat32);
			class_add_func_virtualx("draw_line", TypeVoid, &Painter::draw_line, Flags::CONST);
				func_add_param("x1", TypeFloat32);
				func_add_param("y1", TypeFloat32);
				func_add_param("x2", TypeFloat32);
				func_add_param("y2", TypeFloat32);
			class_add_func_virtualx("draw_lines", TypeVoid, &Painter::draw_lines, Flags::CONST);
				func_add_param("p", TypeComplexList);
			class_add_func_virtualx("draw_polygon", TypeVoid, &Painter::draw_polygon, Flags::CONST);
				func_add_param("p", TypeComplexList);
			class_add_func_virtualx("draw_rect", TypeVoid, mf((void (Painter::*) (float,float,float,float))&Painter::draw_rect), Flags::CONST);
				func_add_param("x", TypeFloat32);
				func_add_param("y", TypeFloat32);
				func_add_param("w", TypeFloat32);
				func_add_param("h", TypeFloat32);
			class_add_func_virtualx("draw_rect", TypeVoid, mf((void (Painter::*) (const rect&))&Painter::draw_rect), Flags::CONST);
				func_add_param("r", TypeRect);
			class_add_func_virtualx("draw_circle", TypeVoid, &Painter::draw_circle, Flags::CONST);
				func_add_param("x", TypeFloat32);
				func_add_param("y", TypeFloat32);
				func_add_param("r", TypeFloat32);
			class_add_func_virtualx("draw_str", TypeVoid, &Painter::draw_str, Flags::CONST);
				func_add_param("x", TypeFloat32);
				func_add_param("y", TypeFloat32);
				func_add_param("str", TypeString);
			class_add_func_virtualx("get_str_width", TypeFloat32, &Painter::get_str_width, Flags::CONST);
				func_add_param("str", TypeString);
			class_add_func_virtualx("draw_image", TypeVoid, &Painter::draw_image, Flags::CONST);
				func_add_param("x", TypeFloat32);
				func_add_param("y", TypeFloat32);
				func_add_param("image", TypeImage);
			class_add_func_virtualx("set_option", TypeVoid, &Painter::set_option, Flags::CONST);
				func_add_param("key", TypeString);
				func_add_param("value", TypeString);
			class_set_vtable(ImagePainter);
}

};
