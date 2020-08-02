/*----------------------------------------------------------------------------*\
| msg                                                                          |
| -> debug logfile system                                                      |
| -> "message.txt" used for output                                             |
|                                                                              |
| vital properties:                                                            |
|  - stores the latest messages in memory for direct access                    |
|                                                                              |
| last updated: 2010.03.06 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(MSG_H)
#define MSG_H

class Path;


// administration
void msg_init(bool verbose = true);
void msg_init(const Path &force_filename, bool verbose = true);
void msg_set_verbose(bool verbose);
void msg_end(bool del_file = true);

// pure output
void msg_write(int i);
void msg_write(const string &str);
void msg_error(const string &str);
void msg_ok();
void msg_right();
void msg_left();
void msg_reset_shift();

// reading log
string msg_get_str(int index);
int msg_get_buffer_size();
string msg_get_buffer(int max_size);
string msg_get_trace();

// output only once
void msg_todo(const string &str);

extern bool msg_inited;


#endif

