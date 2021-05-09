#include "../kaba.h"
#include "../../config.h"
#include "lib.h"



#ifdef _X_USE_PDF_
	#include "../../xfile/pdf.h"
	#define pdf_p(p)		(void*)p
#else
	namespace pdf {
		typedef int Parser;
	}
#define pdf_p(p)		nullptr
#endif


namespace kaba {


extern const Class *TypeBasePainterP;
extern const Class *TypePath;


void SIAddPackageDoc() {
	add_package("doc");

	const Class *TypePdf = add_type("pdf", 0);
	const Class *TypePdfParser = add_type("Parser", sizeof(pdf::Parser), Flags::NONE, TypePdf);


	add_class(TypePdfParser);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, pdf_p(mf(&pdf::Parser::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, pdf_p(mf(&pdf::Parser::__delete__)));
		class_add_func("set_page_size", TypeVoid, pdf_p(mf(&pdf::Parser::add_page)));
			func_add_param("width", TypeFloat32);
			func_add_param("height", TypeFloat32);
		class_add_func("add_page", TypeBasePainterP, pdf_p(mf(&pdf::Parser::add_page)));
		class_add_func("save", TypeVoid, pdf_p(mf(&pdf::Parser::save)));
			func_add_param("filename", TypePath);

	add_class(TypePdf);
		class_add_func("add_font_directory", TypeVoid, pdf_p(&pdf::add_font_directory), Flags::STATIC);
			func_add_param("dir", TypePath);
}

};
