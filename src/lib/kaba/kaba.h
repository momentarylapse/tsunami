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
#include "syntax/SyntaxTree.h"

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
	void compile();
	void update_constant_locations();
	void map_constants_to_opcode();
	void map_global_variables_to_memory();
	void allocate_opcode();
	void align_opcode();
	void assemble_function(int index, Function *f, Asm::InstructionWithParamsList *list);
	void compile_functions(char *oc, int &ocs);
	void CompileOsEntryPoint();
	void LinkOsEntryPoint();
	void link_functions();

	// error messages
	void do_error(const string &msg, int override_line = -1);
	void do_error_link(const string &msg);
	void do_error_internal(const string &msg);

	// execution
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

	Array<Asm::WantedLabel> functions_to_link;
	Array<int> function_vars_to_link;

	bool just_analyse, show_compiler_stats;
	Function *cur_func;
};

Script *Load(const string &filename, bool just_analyse = false);
Script *CreateForSource(const string &source, bool just_analyse = false);
void Remove(Script *s);
void ExecutePublicScripts();
void DeleteAllScripts(bool even_immortal = false, bool force = false);
void ExecuteSingleScriptCommand(const string &cmd);

const Class *GetDynamicType(const void *p);

};

#endif
