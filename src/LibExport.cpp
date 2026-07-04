#include "lib/kapi/KabaExporter.h"
#include <plugins/PluginManager.h>

extern "C" {
#ifndef OS_WINDOWS
	__attribute__ ((visibility ("default")))
#endif
	void export_symbols(kaba::Exporter* e) {
		tsunami::PluginManager::export_kaba_package_tsunami(e);
	}
}

