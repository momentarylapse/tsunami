/*----------------------------------------------------------------------------*\
| Hui utility                                                                  |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _UTIL_EXISTS_
#define _UTIL_EXISTS_

class Path;

namespace hui
{

void _cdecl SetDirectory(const Path &dir);
int _cdecl GetCpuCount();

void _cdecl OpenDocument(const Path &filename);

};

#endif

