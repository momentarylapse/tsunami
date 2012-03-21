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

extern Array<string> dir_search_name;
extern Array<bool> dir_search_is_dir;

int _cdecl dir_search(const string &dir,const string &filter,bool show_directories);

#endif
