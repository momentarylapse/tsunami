/*----------------------------------------------------------------------------*\
| CScript                                                                      |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.10.04 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(SCRIPT_H__INCLUDED_)
#define SCRIPT_H__INCLUDED_

namespace Script{
class Script;
}

#include "../base/base.h"
#include "asm/asm.h"
#include "lib/script_data.h"
#include "syntax/syntax_tree.h"

namespace Script{

extern string Version;

class Exception : public Asm::Exception
{
public:
	Exception(const string &message, const string &expression, int line, int column, Script *s);
	Exception(const Asm::Exception &e, Script *s);
};
/*struct SyntaxException : Exception{};
struct LinkerException : Exception{};
struct LinkerException : Exception{};*/

enum
{
	WaitingModeNone,
	WaitingModeFirst,
	WaitingModeGT,
	WaitingModeRT
};
#define WaitingModeFinished		WaitingModeNone

// executable (compiled) data
class Script
{
public:
	// don't call yourself.... better use LoadScript(...)
	Script();
	~Script();

	void Load(const string &filename, bool just_analyse = false);

	// building operational code
	void Compiler();
	void MapConstantsToMemory();
	void MapGlobalVariablesToMemory();
	void AllocateMemory();
	void AllocateStack();
	void AllocateOpcode();
	void AlignOpcode();
	void CompileFunction(Function *f, char *Opcode, int &OpcodeSize);
	void CompileOsEntryPoint();
	void LinkOsEntryPoint();
	void CompileTaskEntryPoint();

	// error messages
	void DoError(const string &msg, int overwrite_line = -1);
	void DoErrorLink(const string &msg);
	void DoErrorInternal(const string &msg);

	// execution
	void Execute();
	void *MatchFunction(const string &name, const string &return_type, int num_params, ...);
	void *MatchClassFunction(const string &_class, bool allow_derived, const string &name, const string &return_type, int num_params, ...);
	void SetVariable(const string &name, void *data);

	//debug displaying
	void ShowVars(bool include_consts=false);

// data

	string Filename;
	SyntaxTree *syntax;

	int ReferenceCounter;

	char *Opcode; // executable code
	int OpcodeSize;
	char *ThreadOpcode; // executable code
	int ThreadOpcodeSize;
	char *Memory; // memory for global variables, constants etc
	int MemorySize;
	char *Stack; // stack for local variables etc

	Array<t_func*> func;
	t_func *first_execution, *continue_execution;

	bool JustAnalyse, ShowCompilerStats;
	Function *cur_func;
	int WaitingMode;
	float TimeToWait;

	Array<char*> g_var, cnst;

	int MemoryUsed;
};

Script *Load(const string &filename, bool just_analyse = false);
void Remove(Script *s);
void ExecutePublicScripts();
void DeleteAllScripts(bool even_immortal = false, bool force = false);
void ExecuteSingleScriptCommand(const string &cmd);

};

#endif
