#include "../kaba.h"
#include "../../config.h"
#include "../../doc/config.h"
#include "../../any/any.h"
#include "lib.h"



#if __has_include("../../doc/pdf.h")
	#include "../../doc/pdf.h"
	#define pdf_p(p)		p
#else
	namespace pdf {
		typedef int Parser;
	}
#define pdf_p(p)		nullptr
#endif


namespace kaba {


extern const Class *TypeBasePainterP;
extern const Class *TypePath;
extern const Class *TypeStringList;
extern const Class *TypeAny;
const Class *TypeDocConfiguration;


Any _doc_config_get(Configuration &c, const string &key) {
	return c.get(key, Any());
}

void SIAddPackageDoc() {
	add_package("doc");

	TypeDocConfiguration = add_type("Configuration", sizeof(Configuration));
	const Class *TypePdf = add_type("pdf", 0);
	const Class *TypePdfParser = add_type("Parser", sizeof(pdf::Parser), Flags::NONE, TypePdf);


	add_class(TypePdfParser);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, pdf_p(&pdf::Parser::__init__));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, pdf_p(&pdf::Parser::__delete__));
		class_add_func("set_page_size", TypeVoid, pdf_p(&pdf::Parser::add_page));
			func_add_param("width", TypeFloat32);
			func_add_param("height", TypeFloat32);
		class_add_func("add_page", TypeBasePainterP, pdf_p(&pdf::Parser::add_page));
		class_add_func("save", TypeVoid, pdf_p(&pdf::Parser::save));
			func_add_param("filename", TypePath);

	add_class(TypePdf);
		class_add_func("add_font_directory", TypeVoid, pdf_p(&pdf::add_font_directory), Flags::STATIC);
			func_add_param("dir", TypePath);


	add_class(TypeDocConfiguration);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Configuration::__init__);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &Configuration::__del__);
		class_add_func("load", TypeBool, &Configuration::load);
			func_add_param("path", TypePath);
		class_add_func("save", TypeVoid, &Configuration::save, Flags::CONST);
			func_add_param("path", TypePath);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &Configuration::set_int);
			func_add_param("name", TypeString);
			func_add_param("value", TypeInt);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &Configuration::set_float); // FIXME: operator preference...
			func_add_param("name", TypeString);
			func_add_param("value", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &Configuration::set_bool);
			func_add_param("name", TypeString);
			func_add_param("value", TypeBool);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &Configuration::set_str);
			func_add_param("name", TypeString);
			func_add_param("value", TypeString);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &Configuration::set);
			func_add_param("name", TypeString);
			func_add_param("value", TypeAny);
		class_add_func("get_int", TypeInt, &Configuration::get_int, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeInt);
		class_add_func("get_float", TypeFloat32, &Configuration::get_float, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeFloat32);
		class_add_func("get_bool", TypeBool, &Configuration::get_bool, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeBool);
		class_add_func("get_str", TypeString, &Configuration::get_str, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeString);
		class_add_func(IDENTIFIER_FUNC_GET, TypeAny, &_doc_config_get, Flags::CONST);
			func_add_param("name", TypeString);
		class_add_func("keys", TypeStringList, &Configuration::keys, Flags::CONST);
}

};
