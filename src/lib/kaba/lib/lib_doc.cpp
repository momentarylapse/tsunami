#include "../kaba.h"
#include "../../config.h"
#include "../../any/any.h"
#include "lib.h"



#if __has_include("../../doc/pdf.h")
	#include "../../doc/pdf.h"
	#define pdf_p(p) p
#else
	namespace pdf {
		typedef int Parser;
	}
	#define pdf_p(p) nullptr
#endif
#if __has_include("../../doc/ttf.h")
	#include "../../doc/ttf.h"
	#define ttf_p(p) p
#else
	#define ttf_p(p) nullptr
#endif


namespace kaba {


extern const Class *TypeBasePainterP;
extern const Class *TypePath;

void SIAddPackageDoc(Context *c) {
	add_package(c, "doc");

	const Class *TypePdf = add_type("pdf", 0);
	const Class *TypePdfParser = add_type("Parser", sizeof(pdf::Parser), Flags::NONE, TypePdf);


	add_class(TypePdfParser);
		class_add_func(Identifier::Func::INIT, TypeVoid, pdf_p(&pdf::Parser::__init__));
		class_add_func(Identifier::Func::DELETE, TypeVoid, pdf_p(&pdf::Parser::__delete__));
		class_add_func("set_page_size", TypeVoid, pdf_p(&pdf::Parser::add_page));
			func_add_param("width", TypeFloat32);
			func_add_param("height", TypeFloat32);
		class_add_func("add_page", TypeBasePainterP, pdf_p(&pdf::Parser::add_page));
		class_add_func("save", TypeVoid, pdf_p(&pdf::Parser::save));
			func_add_param("filename", TypePath);

	add_class(TypePdf);
		class_add_func("add_font_directory", TypeVoid, ttf_p(&ttf::add_font_directory), Flags::STATIC);
			func_add_param("dir", TypePath);

}

};
