#if !defined(FILE_OP_H__INCLUDED_)
#define FILE_OP_H__INCLUDED_



//--------------------------------------------------------------
// file/directory operations

bool _cdecl dir_create(const string &dir);
bool _cdecl dir_delete(const string &dir);
string _cdecl get_current_dir();
bool _cdecl file_rename(const string &source,const string &target);
bool _cdecl file_copy(const string &source,const string &target);
bool _cdecl file_delete(const string &filename);
bool _cdecl file_test_existence(const string &filename);
bool _cdecl file_is_directory(const string &path);
string _cdecl file_hash(const string &filename, const string &type);


string _cdecl shell_execute(const string &cmd);

//--------------------------------------------------------------
// searching directories

struct DirEntry
{
	string name;
	int size;
	bool is_dir;
	
	void _cdecl __init__()
	{	new(this) DirEntry;	}
	void _cdecl __delete__()
	{	this->~DirEntry();	}
	void _cdecl __assign__(const DirEntry &o)
	{	*this = o;	}
	string _cdecl str()
	{	return "(\"" + name + "\", " + i2s(size) + "\", " + b2s(is_dir) + ")";	}
};

Array<DirEntry> _cdecl dir_search(const string &dir,const string &filter,bool show_directories);

#endif
