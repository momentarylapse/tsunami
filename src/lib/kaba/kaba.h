/*----------------------------------------------------------------------------*\
| Kaba                                                                         |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.10.04 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(KABA_H__INCLUDED_)
#define KABA_H__INCLUDED_

namespace Kaba{
class Script;
}

#include "../base/base.h"
#include "asm/asm.h"
#include "lib/lib.h"
#include "syntax/syntax_tree.h"

namespace Kaba{

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
	WAITING_MODE_NONE,
	WAITING_MODE_FIRST,
	WAITING_MODE_GT,
	WAITING_MODE_RT
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
	void MapConstantsToOpcode();
	void MapGlobalVariablesToMemory();
	void AllocateMemory();
	void AllocateStack();
	void AllocateOpcode();
	void AlignOpcode();
	void AssembleFunction(int index, Function *f, Asm::InstructionWithParamsList *list);
	void CompileFunctions(char *oc, int &ocs);
	void CompileOsEntryPoint();
	void LinkOsEntryPoint();
	void CompileTaskEntryPoint();
	void LinkFunctions();

	// error messages
	void DoError(const string &msg, int override_line = -1);
	void DoErrorLink(const string &msg);
	void DoErrorInternal(const string &msg);

	// execution
	void __Execute();
	void *MatchFunction(const string &name, const string &return_type, int num_params, ...);
	void *MatchClassFunction(const string &_class, bool allow_derived, const string &name, const string &return_type, int num_params, ...);
	void SetVariable(const string &name, void *data);

	//debug displaying
	void ShowVars(bool include_consts=false);

// data

	string filename;
	SyntaxTree *syntax;

	int reference_counter;

	char *opcode; // executable code
	int opcode_size;
	char *__thread_opcode; // executable code DEPRECATED!!!
	int __thread_opcode_size;
	char *memory; // memory for global variables, constants etc
	int memory_size;
	char *__stack; // stack for local variables etc

	Array<t_func*> func;
	t_func *__first_execution, *__continue_execution;
	Array<Asm::WantedLabel> functions_to_link;
	Array<int> function_vars_to_link;

	bool just_analyse, show_compiler_stats;
	Function *cur_func;
	int __waiting_mode;
	float __time_to_wait;

	Array<char*> g_var, cnst;

	int memory_used;
};

Script *Load(const string &filename, bool just_analyse = false);
Script *CreateForSource(const string &source, bool just_analyse = false);
void Remove(Script *s);
void ExecutePublicScripts();
void DeleteAllScripts(bool even_immortal = false, bool force = false);
void ExecuteSingleScriptCommand(const string &cmd);

Class *GetDynamicType(const void *p);

};

#endif
