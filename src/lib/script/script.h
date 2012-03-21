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

#include "../types/types.h"
#include "dasm.h"
#include "script_data.h"
#include "pre_script.h"

extern string ScriptVersion;


static bool UseConstAsGlobalVar=false;


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
};

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
	void SerializeFunction(sFunction *f);
	void SerializeBlock(sBlock *block, sFunction *f, int level);
	void SerializeParameter(sCommand *link, sFunction *f, int level, int index, sSerialCommandParam &param);
	sSerialCommandParam SerializeCommand(sCommand *com, sFunction *f, int level, int index);
	void SerializeOperator(sCommand *com, sFunction *p_func, sSerialCommandParam *param, sSerialCommandParam &ret);

	// error messages
	void DoError(const string &msg, int overwrite_line = -1);
	void DoErrorLink(const string &msg);
	void DoErrorInternal(const string &msg);

	// execution
	void Execute();
	bool ExecuteScriptFunction(const string &name, ...);
	void *MatchFunction(const string &name, const string &return_type, int num_params, ...);//const string &params);
	void SetVariable(const string &name, void *data);

	//debug displaying
	void ShowVars(bool include_consts=false);

// data

	CPreScript *pre_script;

	int ReferenceCounter;
	void *user_data; // to associate additional data with the script

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
