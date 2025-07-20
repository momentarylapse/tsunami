#include "lib/kabaexport/KabaExporter.h"
#include <plugins/PluginManager.h>

extern "C" {
	__attribute__ ((visibility ("default")))
	void export_symbols(kaba::Exporter* e) {
		tsunami::PluginManager::export_kaba_package_tsunami(e);
	}
}

