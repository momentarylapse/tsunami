#if !defined(FILE_OP_H__INCLUDED_)
#define FILE_OP_H__INCLUDED_


class Date;


//--------------------------------------------------------------
// file/directory operations

bool _cdecl dir_create(const string &dir);
bool _cdecl dir_delete(const string &dir);
string _cdecl get_current_dir();
bool _cdecl file_rename(const string &source,const string &target);
bool _cdecl file_copy(const string &source,const string &target);
bool _cdecl file_delete(const string &filename);
bool _cdecl file_exists(const string &filename);
bool _cdecl file_is_directory(const string &path);
int64 _cdecl file_size(const string &path);
Date _cdecl file_mtime(const string &path);
string _cdecl file_hash(const string &filename, const string &type);


string _cdecl shell_execute(const string &cmd);


Array<string> _cdecl dir_search(const string &dir,const string &filter,bool show_directories);

#endif
