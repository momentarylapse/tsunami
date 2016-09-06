/*
 * chunked.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: ankele
 */

#include "../base/base.h"
#include "../file/file.h"
#include "chunked.h"

void strip(string &s)
{
	while((s.num > 0) and (s.back() == ' '))
		s.resize(s.num - 1);
}

ChunkedFileParser::ChunkedFileParser(int _header_name_size)
{
	header_name_size = _header_name_size;
	base = NULL;
}

ChunkedFileParser::Context::Context()
{
	f = NULL;
}

void ChunkedFileParser::Context::pop()
{
	layers.pop();
}

void ChunkedFileParser::Context::push(const string &name, int pos, int size)
{
	layers.add(ChunkedFileParser::Context::Layer(name, pos, size));
}

int ChunkedFileParser::Context::end()
{
	if (layers.num > 0)
		return layers.back().pos0 + layers.back().size;
	return 2000000000;
}

string ChunkedFileParser::Context::str()
{
	string s;
	for (Layer &l : layers){
		if (s.num > 0)
			s += "/";
		s += l.name;
	}
	return s;
}

FileChunkBasic::FileChunkBasic(const string &_name)
{
	name = _name;
	root = NULL;
	context = NULL;
}
FileChunkBasic::~FileChunkBasic()
{
	for (FileChunkBasic *c : children)
		delete(c);
}
void FileChunkBasic::create()
{
	throw string("no data creation... " + name);
}
void FileChunkBasic::notify()
{
	if (root)
		root->on_notify();
}
void FileChunkBasic::info(const string &message)
{
	if (root)
		root->on_info(message);
}
void FileChunkBasic::warn(const string &message)
{
	if (root)
		root->on_warn(message);
}
void FileChunkBasic::error(const string &message)
{
	if (root)
		root->on_error(message);
}

void FileChunkBasic::add_child(FileChunkBasic *c)
{
	children.add(c);
}
void FileChunkBasic::set_root(ChunkedFileParser *r)
{
	root = r;
	context = &r->context;
	for (FileChunkBasic *c : children)
		c->set_root(r);
}

FileChunkBasic *FileChunkBasic::get_sub(const string &name)
{
	for (FileChunkBasic *c : children)
		if (c->name == name){
			c->context = context;
			c->set_parent(get());
			return c;
		}
	throw string("no sub... " + name + " in " + context->str());
}

void FileChunkBasic::write_sub(const string &name, void *p)
{
	FileChunkBasic *c = get_sub(name);
	c->set(p);
	c->write_complete();
}

void FileChunkBasic::write_sub_parray(const string &name, DynamicArray &a)
{
	FileChunkBasic *c = get_sub(name);
	Array<void*> *pa = (Array<void*>*)&a;
	for (int i=0; i<pa->num; i++){
		c->set((*pa)[i]);
		c->write_complete();
	}
}
void FileChunkBasic::write_sub_array(const string &name, DynamicArray &a)
{
	FileChunkBasic *c = get_sub(name);
	for (int i=0; i<a.num; i++){
		c->set((char*)a.data + a.element_size * i);
		c->write_complete();
	}
}

void FileChunkBasic::write_begin_chunk(File *f)
{
	string s = name + "        ";
	f->WriteBuffer(s.data, root->header_name_size);
	f->WriteInt(0); // temporary
	context->push(name, context->f->GetPos(), 0);
}

void FileChunkBasic::write_end_chunk(File *f)
{
	int pos = f->GetPos();
	int chunk_pos = context->layers.back().pos0;
	f->SetPos(chunk_pos - 4, true);
	f->WriteInt(pos - chunk_pos);
	f->SetPos(pos, true);
	context->pop();
}
void FileChunkBasic::write_complete()
{
	File *f = context->f;
	write_begin_chunk(f);
	write_contents();
	write_end_chunk(f);
}
void FileChunkBasic::write_contents()
{
	File *f = context->f;
	write(f);
	write_subs();
}

string FileChunkBasic::read_header()
{
	string cname;
	cname.resize(root->header_name_size);
	context->f->ReadBuffer(cname.data, cname.num);
	strip(cname);
	int size = context->f->ReadInt();
	int pos0 = context->f->GetPos();

	if (size < 0)
		throw string("chunk with negative size found");
	if (pos0 + size > context->end())
		throw string("inner chunk is larger than its parent");

	context->push(cname, pos0, size);
	return cname;
}


void FileChunkBasic::read_contents()
{
	File *f = context->f;

	read(f);

	// read nested chunks
	while (f->GetPos() < context->end() - 8){

		string cname = read_header();

		bool ok = false;
		for (FileChunkBasic *c : children)
			if (c->name == cname){
				c->set_parent(get());
				c->create();
				c->context = context;
				c->read_contents();
				ok = true;
				break;
			}
		if (!ok){
			if (root)
				root->on_unhandled();
			//throw string("no sub handler: " + context->str());
		}

		f->SetPos(context->end(), true);
		context->pop();
	}
}


ChunkedFileParser::~ChunkedFileParser()
{
	delete base;
}

bool ChunkedFileParser::read(const string &filename, void *p)
{
	File *f = FileOpen(filename);
	if (!f)
		throw string("can not open file");
	f->SetBinaryMode(true);
	context.f = f;
	//context.push(Context::Layer(name, 0, f->GetSize()));

	base->set_root(this);
	base->set(p);
	string cname = base->read_header();
	if (cname != base->name)
		throw string("wrong base chunk: ") + cname + " (" + base->name + " expected)";
	base->read_contents();

	delete(f);

	return true;
}

bool ChunkedFileParser::write(const string &filename, void *p)
{
	File *f = FileCreate(filename);
	if (!f)
		throw string("can not create file");
	f->SetBinaryMode(true);
	context.f = f;
	//context->push(Context::Layer(name, 0, 0));

	base->set_root(this);
	base->set(p);
	base->write_complete();

	delete(f);

	return true;
}



