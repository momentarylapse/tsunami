/*
 * chunked.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: ankele
 */

#include "../base/base.h"
#include "../file/file.h"
#include "chunked.h"

void strip(string &s) {
	while ((s.num > 0) and (s.back() == ' '))
		s.resize(s.num - 1);
}

ChunkedFileParser::ChunkedFileParser(int _header_name_size) {
	header_name_size = _header_name_size;
	base = nullptr;
}

ChunkedFileParser::Context::Context() {
	f = nullptr;
}

void ChunkedFileParser::Context::pop() {
	layers.pop();
}

void ChunkedFileParser::Context::push(const string &name, int pos, int size) {
	layers.add({name, pos, size});
}

int ChunkedFileParser::Context::end() {
	if (layers.num > 0)
		return layers.back().pos0 + layers.back().size;
	return 2000000000;
}

string ChunkedFileParser::Context::str() {
	string s;
	for (Layer &l: layers) {
		if (s.num > 0)
			s += "/";
		s += l.name;
	}
	return s;
}

FileChunkBasic::FileChunkBasic(const string &_name) {
	name = _name;
	root = nullptr;
	context = nullptr;
}
FileChunkBasic::~FileChunkBasic() {
	for (auto *c: children)
		delete c;
}
void FileChunkBasic::create() {
	throw Exception("no data creation... " + name);
}
void FileChunkBasic::notify() {
	if (root)
		root->on_notify();
}
void FileChunkBasic::info(const string &message) {
	if (root)
		root->on_info(message);
}
void FileChunkBasic::warn(const string &message) {
	if (root)
		root->on_warn(message);
}
void FileChunkBasic::error(const string &message) {
	if (root)
		root->on_error(message);
}

void FileChunkBasic::add_child(FileChunkBasic *c) {
	children.add(c);
	c->define_children();
}
void FileChunkBasic::set_root(ChunkedFileParser *r) {
	root = r;
	context = &r->context;
	for (auto *c: children)
		c->set_root(r);
}

FileChunkBasic *FileChunkBasic::get_sub(const string &name) {
	for (auto *c: children)
		if (c->name == name) {
			c->context = context;
			c->set_parent(get());
			return c;
		}
	throw Exception(format("no sub... '%s' in '%s'", name, context->str()));
}

void FileChunkBasic::write_sub(const string &name, void *p) {
	auto *c = get_sub(name);
	c->set(p);
	c->write_complete();
}

void FileChunkBasic::write_sub_parray(const string &name, DynamicArray &a) {
	auto c = get_sub(name);
	for (int i=0; i<a.num; i++) {
		c->set(*(void**)a.simple_element(i));
		c->write_complete();
	}
}
void FileChunkBasic::write_sub_array(const string &name, DynamicArray &a) {
	auto c = get_sub(name);
	for (int i=0; i<a.num; i++) {
		c->set(a.simple_element(i));
		c->write_complete();
	}
}

void FileChunkBasic::write_begin_chunk(File *f) {
	string s = name + "        ";
	f->write_buffer(s.data, root->header_name_size);
	f->write_int(-1); // temporary
	context->push(name, context->f->get_pos(), 0);
}

void FileChunkBasic::write_end_chunk(File *f) {
	int pos = f->get_pos();
	int chunk_pos = context->layers.back().pos0;
	f->set_pos(chunk_pos - 4);
	f->write_int(pos - chunk_pos);
	f->set_pos(pos);
	context->pop();
}

void FileChunkBasic::write_complete() {
	File *f = context->f;
	write_begin_chunk(f);
	write_contents();
	write_end_chunk(f);
}

void FileChunkBasic::write_contents() {
	File *f = context->f;
	write(f);
	write_subs();
}

string FileChunkBasic::read_header() {
	string cname;
	cname.resize(root->header_name_size);
	context->f->read_buffer(cname.data, cname.num);
	strip(cname);
	int size = context->f->read_int();
	int pos0 = context->f->get_pos();


	if (size == -1) {
		root->on_warn("chunk with undefined size.... trying to recover");
		size = context->f->get_size() - context->f->get_pos();
	}

	if (size < 0)
		throw Exception("chunk with negative size found");
	if (pos0 + size > context->end())
		throw Exception("inner chunk is larger than its parent");


	context->push(cname, pos0, size);
	return cname;
}


void FileChunkBasic::read_contents() {
	File *f = context->f;

	read(f);

	// read nested chunks
	while (f->get_pos() < context->end() - 8) {
		string cname = read_header();

		bool ok = false;
		for (FileChunkBasic *c: children)
			if (c->name == cname) {
				c->set_parent(get());
				c->create();
				c->context = context;
				c->read_contents();
				ok = true;
				break;
			}
		if (!ok) {
			string tt;
			tt.resize(context->end() - f->get_pos());
			f->read_buffer(tt);
			msg_write(tt.substr(0, 100).hex());

			if (root)
				root->on_unhandled();
			//throw Exception("no sub handler: " + context->str());
		}

		f->set_pos(context->end());
		context->pop();
	}
}


ChunkedFileParser::~ChunkedFileParser() {
	delete base;
}

void ChunkedFileParser::set_base(FileChunkBasic *b) {
	base = b;
	base->define_children();
}

bool ChunkedFileParser::read(const Path &filename, void *p) {
	File *f = FileOpen(filename);
	context.f = f;
	//context.push(Context::Layer(name, 0, f->GetSize()));

	base->set_root(this);
	base->set(p);
	string cname = base->read_header();
	if (cname != base->name)
		throw Exception(format("wrong base chunk: %s (%s expected)", cname, base->name));
	base->read_contents();

	delete f;

	return true;
}

bool ChunkedFileParser::write(const Path &filename, void *p) {
	File *f = FileCreate(filename);
	context.f = f;
	//context->push(Context::Layer(name, 0, 0));

	base->set_root(this);
	base->set(p);
	base->write_complete();

	delete f;

	return true;
}



