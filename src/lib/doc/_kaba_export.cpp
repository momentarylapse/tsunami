#include "../base/base.h"
#include "pdf.h"
#include "ttf.h"
#include "../kabaexport/KabaExporter.h"


void export_package_pdf(kaba::Exporter* e) {
	e->declare_class_size("Parser", sizeof(pdf::Parser));
	e->link_func("Parser.__init__", &kaba::generic_init<pdf::Parser>);
	e->link_func("Parser.__delete__", &kaba::generic_delete<pdf::Parser>);
	e->link_class_func("Parser.set_page_size", &pdf::Parser::set_page_size);
	e->link_class_func("Parser.add_page", &pdf::Parser::add_page);
	e->link_class_func("Parser.save", &pdf::Parser::save);
	
	e->link_func("add_font_directory", &ttf::add_font_directory);
}


