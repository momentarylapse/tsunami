#if !defined(FILE_OP_H__INCLUDED_)
#define FILE_OP_H__INCLUDED_



//--------------------------------------------------------------
// file/directory operations

bool dir_create(const string &dir);
bool dir_delete(const string &dir);
string get_current_dir();
bool file_rename(const string &source,const string &target);
bool file_copy(const string &source,const string &target);
bool file_delete(const string &filename);
bool file_test_existence(const string &filename);


string shell_execute(const string &cmd);

//--------------------------------------------------------------
// searching directories

struct DirEntry
{
	string name;
	int size;
	bool is_dir;

	void __init__()
	{	name.__init__();	}
	void __assign__(const DirEntry &o)
	{	*this = o;	}
	string str()
	{	return "(\"" + name + "\", " + i2s(size) + "\", " + b2s(is_dir) + ")";	}
};

Array<DirEntry> _cdecl dir_search(const string &dir,const string &filter,bool show_directories);

#endif
