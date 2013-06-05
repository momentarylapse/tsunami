#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

#ifdef _X_USE_IMAGE_
	#include "../../image/image.h"
#endif

namespace Script{

extern Type *TypeImage;

#ifdef _X_USE_IMAGE_
	static Image *_image;
	#define	GetDAImage(x)		long(&_image->x)-long(_image)
#else
	typedef int Image;
	#define	GetDAImage(x)		0
#endif

#ifdef _X_USE_IMAGE_
	#define image_p(p)		(void*)(p)
#else
	#define image_p(p)		NULL
#endif

extern Type *TypeIntList;


#ifdef _X_USE_IMAGE_
void amd64_image_get_pixel(color &r, Image &i, int x, int y)
{	r = i.GetPixel(x, y);	}
#define amd64_wrap(orig, wrap)	((config.instruction_set == Asm::InstructionSetAMD64) ? ((void*)(wrap)) : ((void*)(orig)))
#else
#define amd64_wrap(orig, wrap)	NULL
#endif


void SIAddPackageImage()
{
	add_package("image", false);

	TypeImage			= add_type  ("Image",		sizeof(Image));

	
	add_class(TypeImage);
		class_add_element("width",			TypeInt,		GetDAImage(width));
		class_add_element("height",			TypeInt,		GetDAImage(height));
		class_add_element("mode",			TypeInt,		GetDAImage(mode));
		class_add_element("data",			TypeIntList,	GetDAImage(data));
		class_add_element("error",			TypeBool,		GetDAImage(error));
		class_add_element("alpha_used",		TypeBool,		GetDAImage(alpha_used));
		class_add_func("__init__",			TypeVoid,	image_p(mf((tmf)&Image::__init__)));
		class_add_func("__delete__",		TypeVoid,	image_p(mf((tmf)&Image::__delete__)));
		class_add_func("Create",			TypeVoid,	image_p(mf((tmf)&Image::Create)));
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("c",			TypeColor);
		class_add_func("Load",			TypeVoid,	image_p(mf((tmf)&Image::Load)));
			func_add_param("filename",	TypeString);
		class_add_func("Save",			TypeVoid,	image_p(mf((tmf)&Image::Save)));
			func_add_param("filename",	TypeString);
		class_add_func("Scale",			TypeVoid,	image_p(mf((tmf)&Image::Scale)));
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
		class_add_func("SetPixel",		TypeVoid,	image_p(mf((tmf)&Image::SetPixel)));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("c",			TypeColor);
		class_add_func("GetPixel",		TypeColor,	amd64_wrap(mf((tmf)&Image::GetPixel), &amd64_image_get_pixel));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("Delete",			TypeVoid,	image_p(mf((tmf)&Image::Delete)));
		class_add_func("__assign__",			TypeVoid,	image_p(mf((tmf)&Image::operator=)));
			func_add_param("other",		TypeImage);
}

};
