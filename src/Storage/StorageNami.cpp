/*
 * StorageNami.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "StorageNami.h"
#include "../Tsunami.h"


StorageNami::StorageNami() :
	StorageAny("nami")
{
}

StorageNami::~StorageNami()
{
}


Array<int> ChunkPos;
struct ChunkData
{
	AudioFile *a;
	Track *t, *s;
	Effect *fx;
}chunk_data;

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
	ChunkPos.resize(ChunkPos.num -1);

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
	f->WriteStr(e->filename);
	f->WriteBool(e->only_on_selection);
	f->WriteInt(e->start);
	f->WriteInt(e->end);
	foreach(e->param, p)
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
	b->get_16bit_buffer(data);
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

	foreach(s->buffer, b)
		WriteBufferBox(f, &b);

	EndChunk(f);
}

void WriteTrack(CFile *f, Track *t)
{
	BeginChunk(f, "track");
	f->WriteStr(t->name);
	f->WriteFloat(t->volume);
	f->WriteBool(t->muted);
	f->WriteInt(0); // reserved
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);

	foreach(t->buffer, buf)
		WriteBufferBox(f, &buf);

	foreach(t->sub, sub)
		WriteSubTrack(f, &sub);

	foreach(t->fx, effect)
		WriteEffect(f, &effect);

	EndChunk(f);
}

void StorageNami::SaveAudio(AudioFile *a, const string & filename)
{
	tsunami->progress->Start(_("speichere nami"), 0);
	a->filename = filename;

//	int length = a->GetLength();
//	int min = a->GetMin();
	CFile *f = CreateFile(filename);
	f->SetBinaryMode(true);

	BeginChunk(f, "nami");

	f->WriteInt(a->sample_rate);

	foreach(a->tag, tag)
		WriteTag(f, &tag);

	foreachi(a->track, track, i){
		WriteTrack(f, &track);
		tsunami->progress->Set(_("speichere nami"), ((float)i + 0.5f) / (float)a->track.num);
	}

	foreach(a->fx, effect)
		WriteEffect(f, &effect);

	EndChunk(f);

	FileClose(f);
	tsunami->progress->End();
}



void StorageNami::SaveTrack(Track *t, const string & filename)
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
		e.filename = f->ReadStr();
		e.only_on_selection = false;
		e.start = 0;
		e.end = -1;
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
		e.filename = f->ReadStr();
		e.only_on_selection = f->ReadBool();
		e.start = f->ReadInt();
		e.end = f->ReadInt();
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
		Track *t = a->AddEmptyTrack(-1);
		BufferBox buf = t->GetBuffers(0, length);
		for (int i=0;i<length;i++){
			buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
			buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
		}
		tsunami->progress->Set((float)f->GetPos() / (float)file_size);
		t->UpdatePeaks();
		msg_db_m("b",1);
		int NumSubs = f->ReadInt();
		for (int n=0;n<NumSubs;n++){
			string name = f->ReadStr();
			int pos = (float)f->ReadInt();
			int slength = f->ReadInt();
			Track *s = t->AddEmptySubTrack(pos, slength, name);
			//msg_write(s->Length);
			s->volume = f->ReadFloat();
			f->ReadBool(); // s->echo_enabled
			f->ReadFloat(); // s->echo_vol
			f->ReadFloat(); // s->echo_delay
			f->ReadFloat(); // s->echo_damp
			tdata.resize(length * 2);
			f->ReadBuffer((char*)&tdata[0], length * 4);
			buf = s->GetBuffers(0, slength);
			for (int i=0;i<slength;i++){
				buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
				buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
			}
			tsunami->progress->Set((float)f->GetPos() / (float)file_size);
			s->UpdatePeaks();
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
			BufferBox buf = t->GetBuffers(0, length);
			for (int i=0;i<length;i++){
				buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
				buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
			}
			tsunami->progress->Set((float)f->GetPos() / (float)file_size);
			t->UpdatePeaks();
			msg_db_m("b",1);
			int NumSubs = f->ReadInt();
			for (int n=0;n<NumSubs;n++){
				string name = f->ReadStr();
				int pos = (float)f->ReadInt();
				int slength = f->ReadInt();
				Track *s = t->AddEmptySubTrack(pos, slength, name);
				//msg_write(s->Length);
				s->volume = f->ReadFloat();
				f->ReadBool(); // s->echo_enabled
				f->ReadFloat(); // s->echo_vol
				f->ReadFloat(); // s->echo_delay
				f->ReadFloat(); // s->echo_damp
				tdata.resize(slength * 2);
				f->ReadBuffer((char*)&tdata[0], slength * 4);
				buf = s->GetBuffers(0, slength);
				for (int i=0;i<slength;i++){
					buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
					buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
				}
				tsunami->progress->Set((float)f->GetPos() / (float)file_size);
				s->UpdatePeaks();
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
			BufferBox buf = t->GetBuffers(0, length);
			for (int i=0;i<length;i++){
				buf.r[i] = (float)tdata[i*2  ] / 32768.0f;
				buf.l[i] = (float)tdata[i*2+1] / 32768.0f;
			}
			tsunami->progress->Set((float)f->GetPos() / (float)file_size);
			t->UpdatePeaks();
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
				Track *s = t->AddEmptySubTrack(pos, slength, name);
				//msg_write(s->Length);
				s->volume = f->ReadFloat();
				s->muted = f->ReadBool();
				tdata.resize(slength * 2);
				f->ReadBuffer((char*)&tdata[0], slength * 4);
				BufferBox sbuf = s->GetBuffers(0, slength);
				for (int i=0;i<slength;i++){
					sbuf.r[i] = (float)tdata[i*2  ] / 32768.0f;
					sbuf.l[i] = (float)tdata[i*2+1] / 32768.0f;
				}
				tsunami->progress->Set((float)f->GetPos() / (float)file_size);
				s->UpdatePeaks();
				if (ffv == 3)
					ReadFXListOld(f, s->fx);
			}
		}
		if (ffv == 3)
			ReadFXListOld(f, a->fx);
		else
			ReadFXList(f, a->fx);
	}else
		tsunami->Log(Tsunami::LOG_ERROR, format(_("Falsche Version des Dateiformats: %d  (%d erwartet)"), ffv, 5));
	tdata.clear();




	// compress...
	foreach(a->track, t){
		if (t.buffer.num != 1)
			continue;
		bool empty = true;
		for (int i=0;i<t.buffer[0].num;i++)
			if ((t.buffer[0].r[i] != 0) || (t.buffer[0].l[i] != 0)){
				empty = false;
				break;
			}

		if (empty)
			t.buffer.clear();

	}
}

void ReadChunk(CFile *f);
void ReadChunkList(CFile *f)
{
	while (f->GetPos() < ChunkPos.back())
		ReadChunk(f);
}

void ReadChunkNami(CFile *f, AudioFile *a)
{
	a->sample_rate = f->ReadInt();
	ReadChunkList(f);
}

void ReadChunkTag(CFile *f, Array<Tag> &tag)
{
	Tag t;
	t.key = f->ReadStr();
	t.value = f->ReadStr();
	tag.add(t);
}

void ReadChunkTrack(CFile *f, AudioFile *a)
{
	Track *t = a->AddEmptyTrack();
	chunk_data.t = t;
	t->name = f->ReadStr();
	t->volume = f->ReadFloat();
	t->muted = f->ReadBool();
	f->ReadInt(); // reserved
	f->ReadInt();
	f->ReadInt();
	f->ReadInt();
	ReadChunkList(f);
	tsunami->progress->Set((float)f->GetPos() / (float)f->GetSize());
}

void ReadChunkSub(CFile *f, Track *t)
{
	string name = f->ReadStr();
	int pos = f->ReadInt();
	int length = f->ReadInt();
	Track *s = t->AddEmptySubTrack(pos, length, name);
	chunk_data.s = s;
	s->volume = f->ReadFloat();
	s->muted = f->ReadBool();
	s->rep_num = f->ReadInt();
	s->rep_delay = f->ReadInt();
	f->ReadInt(); // reserved
	f->ReadInt();
	ReadChunkList(f);
}

void ReadChunkEffectParam(CFile *f, Effect *e)
{
	EffectParam p;
	p.name = f->ReadStr();
	p.type = f->ReadStr();
	p.value = f->ReadStr();
	e->param.add(p);
}

void ReadChunkEffect(CFile *f, Array<Effect> &fx)
{
	Effect e;
	e.filename = f->ReadStr();
	e.only_on_selection = f->ReadBool();
	e.start = f->ReadInt();
	e.end = f->ReadInt();
	fx.add(e);
	chunk_data.fx = &fx.back();
	ReadChunkList(f);
}

void ReadChunkBufferBox(CFile *f, Track *t)
{
	BufferBox dummy;
	t->buffer.add(dummy);
	BufferBox *b = &t->buffer.back();
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
	ReadChunkList(f);
}

void ReadChunk(CFile *f)
{
	string cname;
	cname.resize(8);
	f->ReadBuffer(cname.data, 8);
	int size = f->ReadInt();
	ChunkPos.add(f->GetPos() + size);

	if (cname == "nami    "){
		ReadChunkNami(f, chunk_data.a);
	}else if (cname == "tag     "){
		ReadChunkTag(f, chunk_data.a->tag);
	}else if (cname == "fx      "){
		if (chunk_data.t)
			ReadChunkEffect(f, chunk_data.t->fx);
		else
			ReadChunkEffect(f, chunk_data.a->fx);
		chunk_data.fx = NULL;
	}else if (cname == "fxparam "){
		if (chunk_data.fx)
			ReadChunkEffectParam(f, chunk_data.fx);
	}else if (cname == "track   "){
		ReadChunkTrack(f, chunk_data.a);
		chunk_data.t = NULL;
	}else if (cname == "sub     "){
		if (chunk_data.t)
			ReadChunkSub(f, chunk_data.t);
		chunk_data.s = NULL;
	}else if (cname == "bufbox  "){
		if (chunk_data.s)
			ReadChunkBufferBox(f, chunk_data.s);
		else if (chunk_data.t)
			ReadChunkBufferBox(f, chunk_data.t);
	}else
		msg_error("unknown nami chunk: " + cname);

	f->SetPos(ChunkPos.back(), true);
	ChunkPos.resize(ChunkPos.num - 1);
}

void load_nami_file_new(CFile *f, AudioFile *a)
{
	chunk_data.a = a;
	chunk_data.t = NULL;
	chunk_data.s = NULL;
	chunk_data.fx = NULL;
	ReadChunk(f);
}



void StorageNami::LoadAudio(AudioFile *a, const string & filename)
{
	msg_db_r("load_nami_file", 1);
	tsunami->progress->Set(_("lade nami"), 0);
	msg_db_m("a",1);

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

	msg_db_m("c",1);
	FileClose(f);
	msg_db_m("d",1);


	msg_db_l(1);
}



void StorageNami::LoadTrack(Track *t, const string & filename)
{
}

