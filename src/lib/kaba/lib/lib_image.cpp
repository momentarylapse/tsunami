#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"

#ifdef _X_USE_IMAGE_
	#include "../../image/image.h"
#endif

namespace Kaba{

extern const Class *TypeImage;
extern const Class *TypeIntList;


#ifdef _X_USE_IMAGE_
	static Image *_image;
	#define	GetDAImage(x)		int_p(&_image->x)-int_p(_image)
#else
	typedef int Image;
	#define	GetDAImage(x)		0
#endif

#ifdef _X_USE_IMAGE_
	#define image_p(p)		(void*)(p)
#else
	#define image_p(p)		NULL
#endif



void SIAddPackageImage() {
	add_package("image", false);

	TypeImage = add_type("Image", sizeof(Image));
	const Class *TypeImageP = add_type_p("Image*", TypeImage);

	
	add_class(TypeImage);
		class_add_element("width", TypeInt, GetDAImage(width));
		class_add_element("height", TypeInt, GetDAImage(height));
		class_add_element("mode", TypeInt, GetDAImage(mode));
		class_add_element("data", TypeIntList, GetDAImage(data));
		class_add_element("error", TypeBool, GetDAImage(error));
		class_add_element("alpha_used", TypeBool, GetDAImage(alpha_used));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, image_p(mf(&Image::__init_ext__)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("c", TypeColor);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, image_p(mf(&Image::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, image_p(mf(&Image::__delete__)));
		class_add_func("create", TypeVoid, image_p(mf(&Image::create)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("c", TypeColor);
		class_add_func("load", TypeVoid, image_p(mf(&Image::load)));
			func_add_param("filename", TypeString);
		class_add_func("save", TypeVoid, image_p(mf(&Image::save)));
			func_add_param("filename", TypeString);
		class_add_func("scale", TypeImageP, image_p(mf(&Image::scale)));
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
		class_add_func("set_pixel", TypeVoid, image_p(mf(&Image::set_pixel)));
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("c", TypeColor);
		class_add_func("get_pixel", TypeColor, mf(&Image::get_pixel));
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
		class_add_func("clear", TypeVoid, image_p(mf(&Image::clear)));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, image_p(mf(&Image::__assign__)));
			func_add_param("other", TypeImage);

	add_func("LoadImage", TypeImageP, image_p(&LoadImage), FLAG_STATIC);
		func_add_param("filename", TypeString);
}

};
