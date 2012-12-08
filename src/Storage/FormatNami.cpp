/*
 * FormatNami.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatNami.h"
#include "../Tsunami.h"


const int CHUNK_SIZE = 1 << 16;


FormatNami::FormatNami() :
	Format("nami", FLAG_FX | FLAG_MULTITRACK | FLAG_TAGS | FLAG_SUBS)
{
}

FormatNami::~FormatNami()
{
}


Array<int> ChunkPos;

void BeginChunk(CFile *f, const string &name)
{
	string s = name + "        ";
	f->WriteBuffer(s.data, 8);
	f->WriteInt(0); // temporary
	ChunkPos.add(f->GetPos());
}

void EndChunk(CFile *f)
{
	int pos = ChunkPos.back();
	ChunkPos.pop();

	int pos0 = f->GetPos();
	f->SetPos(pos - 4, true);
	f->WriteInt(pos0 - pos);
	f->SetPos(pos0, true);
}

void WriteTag(CFile *f, Tag *t)
{
	BeginChunk(f, "tag");
	f->WriteStr(t->key);
	f->WriteStr(t->value);
	EndChunk(f);
}

void WriteEffectParam(CFile *f, EffectParam *p)
{
	BeginChunk(f, "fxparam");
	f->WriteStr(p->name);
	f->WriteStr(p->type);
	f->WriteStr(p->value);
	EndChunk(f);
}

void WriteEffect(CFile *f, Effect *e)
{
	BeginChunk(f, "fx");
	f->WriteStr(e->name);
	f->WriteBool(e->only_on_selection);
	f->WriteInt(e->range.offset);
	f->WriteInt(e->range.num);
	foreach(EffectParam &p, e->param)
		WriteEffectParam(f, &p);
	EndChunk(f);
}

void WriteBufferBox(CFile *f, BufferBox *b)
{
	BeginChunk(f, "bufbox");
	f->WriteInt(b->offset);
	f->WriteInt(b->num);
	f->WriteInt(2);
	f->WriteInt(16);

	Array<short> data;
	if (!b->get_16bit_buffer(data))
		tsunami->log->Error(_("Amplitude zu gro&s, Signal &ubersteuert."));
	f->WriteBuffer(data.data, data.num * sizeof(short));
	EndChunk(f);
}

void WriteSubTrack(CFile *f, Track *s)
{
	BeginChunk(f, "sub");

	f->WriteStr(s->name);
	f->WriteInt(s->pos);
	f->WriteInt(s->length);
	f->WriteFloat(s->volume);
	f->WriteBool(s->muted);
	f->WriteInt(s->rep_num);
	f->WriteInt(s->rep_delay);
	f->WriteInt(0); // reserved
	f->WriteInt(0);

	foreach(BufferBox &b, s->level[0].buffer)
		WriteBufferBox(f, &b);

	EndChunk(f);
}

void WriteBar(CFile *f, Bar &b)
{
	BeginChunk(f, "bar");

	f->WriteInt(b.type);
	f->WriteInt(b.length);
	f->WriteInt(b.num_beats);
	f->WriteInt(b.count);
	f->WriteInt(0); // reserved

	EndChunk(f);
}

void WriteTrackLevel(CFile *f, TrackLevel *l, int level_no)
{
	BeginChunk(f, "level");
	f->WriteInt(level_no);

	foreach(BufferBox &b, l->buffer)
		WriteBufferBox(f, &b);

	EndChunk(f);
}

void WriteTrack(CFile *f, Track *t)
{
	BeginChunk(f, "track");

	f->WriteStr(t->name);
	f->WriteFloat(t->volume);
	f->WriteBool(t->muted);
	f->WriteInt(t->type);
	f->WriteInt(0); // reserved
	f->WriteInt(0);
	f->WriteInt(0);

	foreach(Bar &b, t->bar)
		WriteBar(f, b);

	foreachi(TrackLevel &l, t->level, i)
		WriteTrackLevel(f, &l, i);

	foreach(Track &sub, t->sub)
		WriteSubTrack(f, &sub);

	foreach(Effect &effect, t->fx)
		WriteEffect(f, &effect);

	EndChunk(f);
}

void WriteLevelName(CFile *f, Array<string> level_name)
{
	BeginChunk(f, "lvlname");

	f->WriteInt(level_name.num);
	foreach(string &l, level_name)
		f->WriteStr(l);

	EndChunk(f);
}

void FormatNami::SaveAudio(AudioFile *a, const string & filename)
{
	tsunami->progress->Start(_("speichere nami"), 0);
	a->filename = filename;

//	int length = a->GetLength();
//	int min = a->GetMin();
	CFile *f = CreateFile(filename);
	f->SetBinaryMode(true);

	BeginChunk(f, "nami");

	f->WriteInt(a->sample_rate);

	foreach(Tag &tag, a->tag)
		WriteTag(f, &tag);

	WriteLevelName(f, a->level_name);

	foreachi(Track &track, a->track, i){
		WriteTrack(f, &track);
		tsunami->progress->Set(_("speichere nami"), ((float)i + 0.5f) / (float)a->track.num);
	}

	foreach(Effect &effect, a->fx)
		WriteEffect(f, &effect);

	EndChunk(f);

	FileClose(f);
	tsunami->progress->End();
}



void FormatNami::SaveBuffer(AudioFile *a, BufferBox *b, const string &filename)
{
}



void ReadCompressed(CFile *f, char *data, int size)
{
	memset(data, 0, size);
	int done = 0;
	while(done < size){
		// how many non-zeros?
		int nonzero = f->ReadInt();
		f->ReadBuffer(&data[done], nonzero);
		done += nonzero;

		// how many zeros?
		int zero = f->ReadInt();
		done += zero;
		//printf("%d  %d  %d\n", nonzero, zero, done);
	}
}

void ReadFXListOld(CFile *f, Array<Effect> &fx)
{
	// reset old params....???
	fx.clear();
	int n = f->ReadInt();
	if (f->Eof){
		return;
	}
	for (int i=0;i<n;i++){
		Effect e;
		e.name = f->ReadStr();
		e.only_on_selection = false;
		e.range.offset = 0;
		e.range.num = -1;
		int num_params = f->ReadInt();
		for (int j=0;j<num_params;j++){
			EffectParam p;
			p.name = f->ReadStr();
			f->ReadChar(); // 'f'
			p.type = "float";
			float val = f->ReadFloat();
			p.value = f2s(val, 6);
			e.param.add(p);
		}
		fx.add(e);
	}
}

void ReadFXList(CFile *f, Array<Effect> &fx)
{
	// reset old params....???
	fx.clear();
	int n = f->ReadInt();
	if (f->Eof){
		return;
	}
	for (int i=0;i<n;i++){
		Effect e;
		e.name = f->ReadStr();
		e.only_on_selection = f->ReadBool();
		e.range.offset = f->ReadInt();
		e.range.num = f->ReadInt();
		int num_params = f->ReadInt();
		for (int j=0;j<num_params;j++){
			EffectParam p;
			p.name = f->ReadStr();
			p.type = f->ReadStr();
			p.value = f->ReadStr();
			e.param.add(p);
		}
		fx.add(e);
	}
}

void load_nami_file_old(CFile *f, AudioFile *a)
{
	int file_size = f->GetSize();
	int ffv = f->ReadFileFormatVersion();
	msg_write("old format: " + i2s(ffv));
	Array<short> tdata;
	if (ffv == 1){
		int length = f->ReadInt();
		//msg_write(length);
		f->ReadInt(); // channels
		a->sample_rate = f->ReadInt();
		//msg_write(a->SampleRate);
		f->ReadInt(); // bits per sample
		tdata.resize(length * 2);
		f->ReadBuffer((char*)&tdata[0], length * 4);
		Track *t = a->AddEmptyTrack();
		BufferBox buf = t->GetBuffers(0, Range(0, length));
		for (int i=0;i<length;i++){
			buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
			buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
		}
		tsunami->progress->Set((float)f->GetPos() / (float)file_size);
		msg_db_m("b",1);
		int NumSubs = f->ReadInt();
		for (int n=0;n<NumSubs;n++){
			string name = f->ReadStr();
			int pos = (float)f->ReadInt();
			int slength = f->ReadInt();
			Track *s = t->AddEmptySubTrack(Range(pos, slength), name);
			//msg_write(s->Length);
			s->volume = f->ReadFloat();
			f->ReadBool(); // s->echo_enabled
			f->ReadFloat(); // s->echo_vol
			f->ReadFloat(); // s->echo_delay
			f->ReadFloat(); // s->echo_damp
			tdata.resize(length * 2);
			f->ReadBuffer((char*)&tdata[0], length * 4);
			buf = s->GetBuffers(0, Range(0, slength));
			for (int i=0;i<slength;i++){
				buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
				buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
			}
			tsunami->progress->Set((float)f->GetPos() / (float)file_size);
		}
	}else if (ffv == 2){
		int length = f->ReadInt();
		f->ReadInt(); // channels
		a->sample_rate = f->ReadInt();
		f->ReadInt(); // bits per sample
		int num_tracks = f->ReadInt();
		for (int k=0;k<num_tracks;k++){
			Track *t = a->AddEmptyTrack();
			t->name = f->ReadStr();
			t->volume = f->ReadFloat();
			tdata.resize(length * 2);
			ReadCompressed(f, (char*)&tdata[0], length * 4);
			//f->ReadBuffer((char*)&tdata[0], length * 4);
			BufferBox buf = t->GetBuffers(0, Range(0, length));
			for (int i=0;i<length;i++){
				buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
				buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
			}
			tsunami->progress->Set((float)f->GetPos() / (float)file_size);
			msg_db_m("b",1);
			int NumSubs = f->ReadInt();
			for (int n=0;n<NumSubs;n++){
				string name = f->ReadStr();
				int pos = (float)f->ReadInt();
				int slength = f->ReadInt();
				Track *s = t->AddEmptySubTrack(Range(pos, slength), name);
				//msg_write(s->Length);
				s->volume = f->ReadFloat();
				f->ReadBool(); // s->echo_enabled
				f->ReadFloat(); // s->echo_vol
				f->ReadFloat(); // s->echo_delay
				f->ReadFloat(); // s->echo_damp
				tdata.resize(slength * 2);
				f->ReadBuffer((char*)&tdata[0], slength * 4);
				buf = s->GetBuffers(0, Range(0, slength));
				for (int i=0;i<slength;i++){
					buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
					buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
				}
				tsunami->progress->Set((float)f->GetPos() / (float)file_size);
			}
		}
	}else if ((ffv == 3) || (ffv == 5)){
		int length = f->ReadInt();
		f->ReadInt(); // channels
		a->sample_rate = f->ReadInt();
		f->ReadInt(); // bits per sample
		int num_meta = f->ReadInt();
		for (int i=0;i<num_meta;i++){
			Tag t;
			t.key = f->ReadStr();
			t.value = f->ReadStr();
			a->tag.add(t);
		}
		int num_tracks = f->ReadInt();
		for (int k=0;k<num_tracks;k++){
			Track *t = a->AddEmptyTrack();
			if (ffv == 5)
				f->ReadInt();
			t->name = f->ReadStr();
			t->volume = f->ReadFloat();
			t->muted = f->ReadBool();
			tdata.resize(length * 2);
			ReadCompressed(f, (char*)&tdata[0], length * 4);
			//f->ReadBuffer((char*)&tdata[0], length * 4);
			BufferBox buf = t->GetBuffers(0, Range(0, length));
			for (int i=0;i<length;i++){
				buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
				buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
			}
			tsunami->progress->Set((float)f->GetPos() / (float)file_size);
			if (ffv == 3)
				ReadFXListOld(f, t->fx);
			else
				ReadFXList(f, t->fx);
			msg_db_m("b",1);
			int NumSubs = f->ReadInt();
			for (int n=0;n<NumSubs;n++){
				if (ffv == 5)
					f->ReadInt();
				string name = f->ReadStr();
				int pos = (float)f->ReadInt();
				int slength = f->ReadInt();
				Track *s = t->AddEmptySubTrack(Range(pos, slength), name);
				//msg_write(s->Length);
				s->volume = f->ReadFloat();
				s->muted = f->ReadBool();
				tdata.resize(slength * 2);
				f->ReadBuffer((char*)&tdata[0], slength * 4);
				BufferBox sbuf = s->GetBuffers(0, Range(0, slength));
				for (int i=0;i<slength;i++){
					sbuf.r[i] = (float)tdata[i*2  ] / 32768.0f;
					sbuf.l[i] = (float)tdata[i*2+1] / 32768.0f;
				}
				tsunami->progress->Set((float)f->GetPos() / (float)file_size);
				if (ffv == 3)
					ReadFXListOld(f, s->fx);
			}
		}
		if (ffv == 3)
			ReadFXListOld(f, a->fx);
		else
			ReadFXList(f, a->fx);
	}else
		tsunami->log->Error(format(_("Falsche Version des Dateiformats: %d  (%d erwartet)"), ffv, 5));
	tdata.clear();




	// compress...
	foreach(Track &t, a->track)
		foreach(TrackLevel &l, t.level){
			if (l.buffer.num != 1)
				continue;
			bool empty = true;
			for (int i=0;i<l.buffer[0].num;i++)
				if ((l.buffer[0].r[i] != 0) || (l.buffer[0].l[i] != 0)){
					empty = false;
					break;
				}

			if (empty)
				l.buffer.clear();
		}
}

typedef void chunk_reader(CFile*, void*);

struct ChunkHandler
{
	string tag;
	chunk_reader *reader;
	void *data;
};

struct ChunkLevelData
{
	ChunkLevelData(){}
	ChunkLevelData(const string &_tag, int _pos)
	{	tag = _tag;	pos = _pos;	}
	int pos;
	string tag;
	Array<ChunkHandler> handler;
};
Array<ChunkLevelData> chunk_data;

void AddChunkHandler(const string &tag, chunk_reader *reader, void *data)
{
	ChunkHandler h;
	h.tag = tag;
	h.reader = reader;
	h.data = data;
	chunk_data.back().handler.add(h);
}

void ReadChunkTag(CFile *f, Array<Tag> *tag)
{
	Tag t;
	t.key = f->ReadStr();
	t.value = f->ReadStr();
	tag->add(t);
}

void ReadChunkLevelName(CFile *f, AudioFile *a)
{
	int num = f->ReadInt();
	a->level_name.clear();
	for (int i=0;i<num;i++)
		a->level_name.add(f->ReadStr());
}

void ReadChunkEffectParam(CFile *f, Effect *e)
{
	EffectParam p;
	p.name = f->ReadStr();
	p.type = f->ReadStr();
	p.value = f->ReadStr();
	e->param.add(p);
}

void ReadChunkEffect(CFile *f, Array<Effect> *fx)
{
	Effect e;
	e.name = f->ReadStr();
	e.only_on_selection = f->ReadBool();
	e.range.offset = f->ReadInt();
	e.range.num = f->ReadInt();
	fx->add(e);

	AddChunkHandler("fxparam", (chunk_reader*)&ReadChunkEffectParam, &fx->back());
}

void ReadChunkBufferBox(CFile *f, TrackLevel *l)
{
	BufferBox dummy;
	l->buffer.add(dummy);
	BufferBox *b = &l->buffer.back();
	b->offset = f->ReadInt();
	int num = f->ReadInt();
	b->resize(num);
	f->ReadInt(); // channels (2)
	f->ReadInt(); // bit (16)

	Array<short> data;
	data.resize(num * 2);

	// read chunk'ed
	int offset = 0;
	for (int n=0;n<(num * 4) / CHUNK_SIZE;n++){
		f->ReadBuffer(&data[offset], CHUNK_SIZE);
		tsunami->progress->Set((float)f->GetPos() / (float)f->GetSize());
		offset += CHUNK_SIZE / 2;
	}
	f->ReadBuffer(&data[offset], (num * 4) % CHUNK_SIZE);

	// insert
	for (int i=0;i<num;i++){
		b->r[i] =  (float)data[i * 2    ] / 32768.0f;
		b->l[i] =  (float)data[i * 2 + 1] / 32768.0f;
	}
}

void ReadChunkSubBufferBox(CFile *f, BufferBox *b)
{
	b->offset = f->ReadInt();
	int num = f->ReadInt();
	b->resize(num);
	f->ReadInt(); // channels (2)
	f->ReadInt(); // bit (16)

	Array<short> data;
	data.resize(num * 2);
	f->ReadBuffer(data.data, num * 4);
	for (int i=0;i<num;i++){
		b->r[i] =  (float)data[i * 2    ] / 32768.0f;
		b->l[i] =  (float)data[i * 2 + 1] / 32768.0f;
	}
}

void ReadChunkSub(CFile *f, Track *t)
{
	string name = f->ReadStr();
	int pos = f->ReadInt();
	int length = f->ReadInt();
	Track *s = t->AddEmptySubTrack(Range(pos, length), name);
	s->volume = f->ReadFloat();
	s->muted = f->ReadBool();
	s->rep_num = f->ReadInt();
	s->rep_delay = f->ReadInt();
	f->ReadInt(); // reserved
	f->ReadInt();

	AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkSubBufferBox, &s->level[0].buffer[0]);
}

void ReadChunkBar(CFile *f, Array<Bar> *bar)
{
	Bar b;
	b.type = f->ReadInt();
	b.length = f->ReadInt();
	b.num_beats = f->ReadInt();
	b.count = f->ReadInt();
	f->ReadInt(); // reserved
	bar->add(b);
}

void ReadChunkTrackLevel(CFile *f, Track *t)
{
	int l = f->ReadInt();
	AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkBufferBox, &t->level[l]);
}

void ReadChunkTrack(CFile *f, AudioFile *a)
{
	Track *t = a->AddEmptyTrack();
	t->name = f->ReadStr();
	t->volume = f->ReadFloat();
	t->muted = f->ReadBool();
	t->type = f->ReadInt();
	f->ReadInt(); // reserved
	f->ReadInt();
	f->ReadInt();
	tsunami->progress->Set((float)f->GetPos() / (float)f->GetSize());

	AddChunkHandler("level", (chunk_reader*)&ReadChunkTrackLevel, t);
	AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkBufferBox, &t->level[0]);
	AddChunkHandler("sub", (chunk_reader*)&ReadChunkSub, t);
	AddChunkHandler("fx", (chunk_reader*)&ReadChunkEffect, &t->fx);
	AddChunkHandler("bar", (chunk_reader*)&ReadChunkBar, &t->bar);
}

void ReadChunkNami(CFile *f, AudioFile *a)
{
	a->sample_rate = f->ReadInt();

	AddChunkHandler("tag", (chunk_reader*)&ReadChunkTag, &a->tag);
	AddChunkHandler("fx", (chunk_reader*)&ReadChunkEffect, &a->fx);
	AddChunkHandler("lvlname", (chunk_reader*)&ReadChunkLevelName, a);
	AddChunkHandler("track", (chunk_reader*)&ReadChunkTrack, a);
}

void strip(string &s)
{
	while((s.num > 0) && (s.back() == ' '))
		s.resize(s.num - 1);
}

void ReadChunk(CFile *f)
{
	string cname;
	cname.resize(8);
	f->ReadBuffer(cname.data, 8);
	strip(cname);
	int size = f->ReadInt();
	chunk_data.add(ChunkLevelData(cname, f->GetPos() + size));


	bool handled = false;
	foreach(ChunkHandler &h, chunk_data[chunk_data.num - 2].handler)
		if (cname == h.tag){
			h.reader(f, h.data);
			handled = true;
			break;
		}

	if (handled){

		// read nested chunks
		while (f->GetPos() < chunk_data.back().pos)
			ReadChunk(f);

	}else
		tsunami->log->Error("unknown nami chunk: " + cname + " (within " + chunk_data[chunk_data.num - 2].tag + ")");


	msg_left();
	f->SetPos(chunk_data.back().pos, true);
	chunk_data.pop();
}

void load_nami_file_new(CFile *f, AudioFile *a)
{
	chunk_data.clear();
	chunk_data.add(ChunkLevelData("-top level-", 0));
	AddChunkHandler("nami", (chunk_reader*)&ReadChunkNami, a);

	ReadChunk(f);
	chunk_data.clear();
}



void FormatNami::LoadAudio(AudioFile *a, const string & filename)
{
	msg_db_r("load_nami_file", 1);
	tsunami->progress->Set(_("lade nami"), 0);

	// TODO?
	a->tag.clear();

	CFile *f = OpenFile(a->filename);
	f->SetBinaryMode(true);
	bool is_old = (f->ReadChar() == 'b');
	f->SetPos(0, true);

	if (is_old)
		load_nami_file_old(f, a);
	else
		load_nami_file_new(f, a);

	FileClose(f);

	a->UpdateSelection();


	msg_db_l(1);
}



void FormatNami::LoadTrack(Track *t, const string & filename)
{
}

