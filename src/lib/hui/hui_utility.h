/*----------------------------------------------------------------------------*\
| Hui utility                                                                  |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_UTIL_EXISTS_
#define _HUI_UTIL_EXISTS_


void _cdecl HuiSetDirectory(const string &dir);
void _cdecl HuiSleep(int duration_ms);
int _cdecl HuiGetCpuCount();



// error handling
void HuiSetErrorFunction(hui_callback *error_function);
void HuiSetDefaultErrorHandler(const string &program, const string &version, hui_callback *error_cleanup_function);
void HuiRaiseError(const string &message);
void HuiSendBugReport();

// clipboard
void _cdecl HuiCopyToClipBoard(const string &buffer);
string _cdecl HuiPasteFromClipBoard();
void _cdecl HuiOpenDocument(const string &filename);

// timers
int _cdecl HuiCreateTimer();
float _cdecl HuiGetTime(int index);


#endif

