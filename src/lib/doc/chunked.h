/*
 * chunked.h
 *
 *  Created on: Dec 11, 2015
 *      Author: ankele
 */

#ifndef SRC_LIB_XFILE_CHUNKED_H_
#define SRC_LIB_XFILE_CHUNKED_H_

#include "../os/formatter.h"

class Path;

class FileChunkBasic;

class ChunkedFileParser {
public:
	ChunkedFileParser(int _header_name_size);
	virtual ~ChunkedFileParser();

	bool read(const Path &filename, void *p);
	bool write(const Path &filename, void *p);
	void set_base(FileChunkBasic *base);

	virtual void on_notify(){};
	virtual void on_unhandled(){};
	virtual void on_error(const string &message){}
	virtual void on_warn(const string &message){}
	virtual void on_info(const string &message){}

	FileChunkBasic *base;
	int header_name_size;

	struct Context {
		struct Layer {
			string name;
			int pos0, size;
			Layer(){}
			Layer(const string &_name, int _pos, int _size) {
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

		BinaryFormatter *f;
		Array<Layer> layers;
	};
	Context context;

};

class FileChunkBasic {
public:
	FileChunkBasic(const string &_name);
	virtual ~FileChunkBasic();
	virtual void define_children() {};
	virtual void read(BinaryFormatter *f) = 0;
	virtual void write(BinaryFormatter *f) = 0;
	virtual void write_subs() {}
	virtual void create();
	string name;
	void _clamp_name_rec(int length);

	void notify();
	void info(const string &message);
	void warn(const string &message);
	void error(const string &message);
	void add_child(FileChunkBasic *c);
	void set_root(ChunkedFileParser *r);

	virtual void set(void *t) {};
	virtual void set_parent(void *t) {};
	virtual void *get() { return 0; }

	ChunkedFileParser::Context *context;
	Array<FileChunkBasic*> children;
	ChunkedFileParser *root;

	FileChunkBasic *get_sub(const string &name);

	void write_sub(const string &name, void *p);
	void write_sub_parray(const string &name, const DynamicArray &a);
	void write_sub_array(const string &name, const DynamicArray &a);
	void write_begin_chunk(BinaryFormatter *f);
	void write_end_chunk(BinaryFormatter *f);
	void write_complete();
	void write_contents();
	string read_header();
	void read_contents();
};



template<class PT, class T>
class FileChunk : public FileChunkBasic {
public:
	FileChunk(const string &name) : FileChunkBasic(name){ me = nullptr; parent = nullptr; }
	void *get() override {
		return me;
	}
	void set(void *_me) override {
		me = (T*)_me;
	}
	void set_parent(void *_parent) override {
		parent = (PT*)_parent;
	}
	T *me;
	PT *parent;
};





#endif /* SRC_LIB_XFILE_CHUNKED_H_ */
