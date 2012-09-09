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




class CScript;

#include "../base/base.h"
#include "../types/types.h"
#include "dasm.h"
#include "script_data.h"
#include "pre_script.h"

extern string ScriptVersion;


extern bool UseConstAsGlobalVar;


enum
{
	WaitingModeNone,
	WaitingModeFirst,
	WaitingModeGT,
	WaitingModeRT
};
#define WaitingModeFinished		WaitingModeNone



struct sSerialCommandParam
{
	int kind;
	char *p;
	sType *type;
	int shift;
	//int c_id, v_id;
	bool operator == (const sSerialCommandParam &param) const
	{	return (kind == param.kind) && (p == param.p) && (type == param.type) && (shift == param.shift);	}
};

struct sSerialCommand
{
	int inst;
	sSerialCommandParam p1, p2;
	int pos;
};

struct SerializerData;

// executable (compiled) data
class CScript
{
public:
	// don't call yourself.... better use LoadScript(...)
	CScript(const string &filename, bool just_analyse = false);
	CScript();
	~CScript();

	// building operational code
	void Compiler();
	void MapConstantsToMemory();
	void MapGlobalVariablesToMemory();
	void AllocateMemory();
	void AllocateStack();
	void AllocateOpcode();
	void CompileFunction(sFunction *f, char *Opcode, int &OpcodeSize);
	void SerializeFunction(SerializerData *d, sFunction *f);
	void SerializeBlock(SerializerData *d, sBlock *block, int level);
	void SerializeParameter(SerializerData *d, sCommand *link, int level, int index, sSerialCommandParam &param);
	sSerialCommandParam SerializeCommand(SerializerData *d, sCommand *com, int level, int index);
	void SerializeOperator(SerializerData *d, sCommand *com, sSerialCommandParam *param, sSerialCommandParam &ret);
	void CompileOsEntryPoint();
	void LinkOsEntryPoint();
	void CompileTaskEntryPoint();

	// error messages
	void DoError(const string &msg, int overwrite_line = -1);
	void DoErrorLink(const string &msg);
	void DoErrorInternal(const string &msg);

	// execution
	void Execute();
	void *MatchFunction(const string &name, const string &return_type, int num_params, ...);//const string &params);
	void SetVariable(const string &name, void *data);

	//debug displaying
	void ShowVars(bool include_consts=false);

// data

	CPreScript *pre_script;

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

	bool Error, ParserError, LinkerError, isCopy, isPrivate, JustAnalyse, ShowCompilerStats;
	string ErrorMsg, ErrorMsgExt[2];
	int ErrorLine, ErrorColumn;
	sFunction *cur_func;
	int WaitingMode;
	float TimeToWait;

	Array<char*> g_var, cnst;

	int MemoryUsed;
};

extern CScript *LoadScript(const string &filename, bool is_public = true, bool just_analyse = false);
extern void RemoveScript(CScript *s);
extern string ScriptDirectory;
extern bool ScriptCompileSilently;
extern bool ScriptShowCompilerStats;
extern void ExecutePublicScripts();
extern void DeleteAllScripts(bool even_immortal = false, bool force = false);
extern void ExecuteSingleScriptCommand(const string &cmd);



#endif
