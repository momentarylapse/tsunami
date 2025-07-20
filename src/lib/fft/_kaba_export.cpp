#include "../base/base.h"
#include "fft.h"
#include "../kabaexport/KabaExporter.h"


void export_package_fft(kaba::Exporter* e) {
	e->link_func("c2c", &fft::c2c);
	e->link_func("r2c", &fft::r2c);
	e->link_func("c2r_inv", &fft::c2r_inv);
	e->link_func("c2c_2d", &fft::c2c_2d);
}


