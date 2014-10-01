/*
 * FormatGp4.cpp
 *
 *  Created on: 01.10.2014
 *      Author: michi
 */

#include "FormatGp4.h"
#include "../Tsunami.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"

FormatGp4::FormatGp4() :
	Format("GuitarPro 4", "gp4", FLAG_MIDI | FLAG_READ | FLAG_MULTITRACK)
{
}

FormatGp4::~FormatGp4()
{
}

static string read_str1(CFile *f)
{
	int l = f->ReadByte();
	//msg_write(l);
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;
}

static int read_int_be(CFile *f)
{
	int a = f->ReadByte();
	int b = f->ReadByte();
	int c = f->ReadByte();
	int d = f->ReadByte();
	return d | (c << 8) | (b << 16) | (a << 24);
}

static string read_str4(CFile *f)
{
	int l = f->ReadInt();
	/*msg_write(l);
	string s;
	s.resize(l);
	f->ReadBuffer(s.data, l);
	return s;*/
	return read_str1(f);
}

void FormatGp4::SaveBuffer(AudioFile *a, BufferBox *b, const string &filename){}

void FormatGp4::LoadTrack(Track *t, const string & filename, int offset, int level){}

void FormatGp4::SaveAudio(AudioFile *a, const string & filename){}

void FormatGp4::LoadAudio(AudioFile *a, const string &filename)
{
	CFile *f = FileOpen(filename);
	char *data = new char[1024];

	try{

		if (!f)
			throw string("can't open file");
		f->SetBinaryMode(true);
		string s;
		s = read_str1(f);
		msg_write("version: "+s);
		f->ReadBuffer(data, 30 - s.num);
		msg_write("title: "+read_str4(f));
		read_str4(f);
		msg_write("artist: "+read_str4(f));
		msg_write("album: "+read_str4(f));
		msg_write("author: "+read_str4(f));
		msg_write("copy: "+read_str4(f));
		msg_write("writer: "+read_str4(f));
		read_str4(f);
		int n = f->ReadInt();
		msg_write(n);
		for (int i=0; i<n; i++)
			msg_write("comment: " + read_str4(f));
	}catch(const string &s){
		tsunami->log->Error(s);
	}

	delete(data);
	if (f)
		FileClose(f);
}

