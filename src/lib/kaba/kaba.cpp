/*----------------------------------------------------------------------------*\
| Kaba                                                                         |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "../os/file.h"
#include "kaba.h"

namespace kaba {

string Version = "0.20.5.1";

//#define ScriptDebug


VirtualTable* get_vtable(const VirtualBase *p) {
	return *(VirtualTable**)p;
}

};
