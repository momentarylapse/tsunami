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
	Exception(const Asm::Exception &e, Script *s, Function *f);
};
/*struct SyntaxException : Exception{};
struct LinkerException : Exception{};
struct LinkerException : Exception{};*/


// executable (compiled) data
class Script
{
public:
	// don't call yourself.... better use LoadScript(...)
	Script();
	~Script();

	void load(const string &filename, bool just_analyse = false);

	// building operational code
	void compile();
	void update_constant_locations();
	void map_constants_to_opcode();
	void _map_global_variables_to_memory(char *mem, int &offset, char *address, const Class *name_space);
	void map_global_variables_to_memory();
	void map_constants_to_memory(char *mem, int &offset, char *address);
	void allocate_opcode();
	void align_opcode();
	void allocate_memory();
	void assemble_function(int index, Function *f, Asm::InstructionWithParamsList *list);
	void compile_functions(char *oc, int &ocs);
	void CompileOsEntryPoint();
	void LinkOsEntryPoint();
	void link_functions();
	void link_virtual_functions_into_vtable(const Class *c);

	// error messages
	void do_error(const string &msg, int override_line = -1);
	void do_error_link(const string &msg);
	void do_error_internal(const string &msg);

	// execution
	void *match_function(const string &name, const string &return_type, const Array<string> &param_types);
	void *match_class_function(const string &_class, bool allow_derived, const string &name, const string &return_type, const Array<string> &param_types);
	void set_variable(const string &name, void *data);

	//debug displaying
	void show_vars(bool include_consts=false);

// data

	string filename;
	SyntaxTree *syntax;

	int reference_counter;

	char *opcode; // executable code
	int opcode_size;

	char *memory;
	int memory_size;

	Array<Asm::WantedLabel> functions_to_link;
	Array<int> function_vars_to_link;

	bool just_analyse, show_compiler_stats;
	Function *cur_func;


	// package
	bool used_by_default;
	Array<const Class*> classes();
	Array<Variable*> variables();
	Array<Constant*> constants();
	Array<Function*> functions();
	const Class* base_class();
};

Script *Load(const string &filename, bool just_analyse = false);
Script *CreateForSource(const string &source, bool just_analyse = false);
void Remove(Script *s);
void ExecutePublicScripts();
void DeleteAllScripts(bool even_immortal = false, bool force = false);
void ExecuteSingleScriptCommand(const string &cmd);

const Class *GetDynamicType(const VirtualBase *p);
string var2str(const void *p, const Class *type);

};

#endif
