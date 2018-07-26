/*
 * chunked.h
 *
 *  Created on: Dec 11, 2015
 *      Author: ankele
 */

#ifndef SRC_LIB_XFILE_CHUNKED_H_
#define SRC_LIB_XFILE_CHUNKED_H_


class FileChunkBasic;

class ChunkedFileParser
{
public:
	ChunkedFileParser(int _header_name_size);
	virtual ~ChunkedFileParser();

	bool read(const string &filename, void *p);
	bool write(const string &filename, void *p);
	void set_base(FileChunkBasic *base);

	virtual void on_notify(){};
	virtual void on_unhandled(){};
	virtual void on_error(const string &message){}
	virtual void on_warn(const string &message){}
	virtual void on_info(const string &message){}

	FileChunkBasic *base;
	int header_name_size;

	struct Context
	{
		struct Layer
		{
			string name;
			int pos0, size;
			Layer(){}
			Layer(const string &_name, int _pos, int _size)
			{
				name = _name;
				pos0 = _pos;
				size = _size;
			}
		};
		Context();
		void pop();
		void push(const string &name, int pos, int size);
		int end();
		string str();

		File *f;
		Array<Layer> layers;
	};
	Context context;

};

class FileChunkBasic
{
public:
	FileChunkBasic(const string &_name);
	virtual ~FileChunkBasic();
	virtual void write(File *f) = 0;
	virtual void read(File *f) = 0;
	virtual void create();
	string name;

	void notify();
	void info(const string &message);
	void warn(const string &message);
	void error(const string &message);
	void add_child(FileChunkBasic *c);
	void set_root(ChunkedFileParser *r);

	virtual void set(void *t){};
	virtual void set_parent(void *t){};
	virtual void *get(){ return 0; }

	ChunkedFileParser::Context *context;
	Array<FileChunkBasic*> children;
	ChunkedFileParser *root;

	virtual void write_subs(){}

	FileChunkBasic *get_sub(const string &name);

	void write_sub(const string &name, void *p);
	void write_sub_parray(const string &name, DynamicArray &a);
	void write_sub_array(const string &name, DynamicArray &a);
	void write_begin_chunk(File *f);
	void write_end_chunk(File *f);
	void write_complete();
	void write_contents();
	string read_header();
	void read_contents();
};



template<class PT, class T>
class FileChunk : public FileChunkBasic
{
public:
	FileChunk(const string &name) : FileChunkBasic(name){ me = nullptr; parent = nullptr; }
	void *get() override
	{
		return me;
	}
	void set(void *_me) override
	{
		me = (T*)_me;
	}
	void set_parent(void *_parent) override
	{
		parent = (PT*)_parent;
	}
	T *me;
	PT *parent;
};





#endif /* SRC_LIB_XFILE_CHUNKED_H_ */
