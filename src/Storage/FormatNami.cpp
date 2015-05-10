/*
 * FormatNami.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatNami.h"
#include "../Tsunami.h"
#include "../Plugins/Effect.h"
#include "../Plugins/MidiEffect.h"
#include "../Stuff/Log.h"
#include "../View/Helper/Progress.h"
#include "../Audio/Synth/Synthesizer.h"


const int CHUNK_SIZE = 1 << 16;


FormatNami::FormatNami() :
	Format("Tsunami", "nami", FLAG_AUDIO | FLAG_MIDI | FLAG_FX | FLAG_MULTITRACK | FLAG_TAGS | FLAG_SUBS | FLAG_READ | FLAG_WRITE)
{
}

FormatNami::~FormatNami()
{
}

void WriteMidi(CFile *f, MidiData &m);

Array<int> ChunkPos;



void strip(string &s)
{
	while((s.num > 0) && (s.back() == ' '))
		s.resize(s.num - 1);
}

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

void WriteEffect(CFile *f, Effect *e)
{
	BeginChunk(f, "effect");
	f->WriteStr(e->name);
	f->WriteBool(e->only_on_selection);
	f->WriteInt(e->range.offset);
	f->WriteInt(e->range.num);
	f->WriteStr(e->configToString());
	f->WriteStr(e->enabled ? "" : "disabled");
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
		tsunami->log->error(_("Amplitude zu gro&s, Signal &ubersteuert."));
	f->WriteBuffer(data.data, data.num * sizeof(short));
	EndChunk(f);
}

void WriteSample(CFile *f, Sample *s)
{
	BeginChunk(f, "sample");
	f->WriteStr(s->name);
	f->WriteFloat(s->volume);
	f->WriteInt(s->offset);
	f->WriteInt(s->type); // reserved
	f->WriteInt(0);
	if (s->type == Track::TYPE_AUDIO)
		WriteBufferBox(f, &s->buf);
	else if (s->type == Track::TYPE_MIDI)
		WriteMidi(f, s->midi);
	EndChunk(f);
}

void WriteSampleRef(CFile *f, SampleRef *s)
{
	BeginChunk(f, "samref");

	f->WriteStr(s->origin->name);
	f->WriteInt(s->pos);
	f->WriteInt(s->origin->get_index());
	f->WriteFloat(s->volume);
	f->WriteBool(s->muted);
	f->WriteInt(s->rep_num);
	f->WriteInt(s->rep_delay);
	f->WriteInt(0); // reserved
	f->WriteInt(0);

	EndChunk(f);
}

void WriteBar(CFile *f, BarPattern &b)
{
	BeginChunk(f, "bar");

	f->WriteInt(b.type);
	f->WriteInt(b.length);
	f->WriteInt(b.num_beats);
	f->WriteInt(b.count);
	f->WriteInt(0); // reserved

	EndChunk(f);
}

void WriteMidiEvent(CFile *f, MidiEvent &e)
{
	BeginChunk(f, "event");

	f->WriteInt(e.pos);
	f->WriteInt(e.pitch);
	f->WriteFloat(e.volume);
	f->WriteInt(0); // reserved

	EndChunk(f);
}

void WriteMidiEffect(CFile *f, MidiEffect *e)
{
	BeginChunk(f, "effect");
	f->WriteStr(e->name);
	f->WriteBool(e->only_on_selection);
	f->WriteInt(e->range.offset);
	f->WriteInt(e->range.num);
	f->WriteStr(e->configToString());
	f->WriteStr(e->enabled ? "" : "disabled");
	EndChunk(f);
}

void WriteMidi(CFile *f, MidiData &m)
{
	BeginChunk(f, "midi");

	f->WriteStr("");
	f->WriteStr("");
	f->WriteStr("");
	f->WriteInt(0); // reserved

	foreach(MidiEvent &e, m)
		WriteMidiEvent(f, e);

	foreach(MidiEffect *e, m.fx)
		WriteMidiEffect(f, e);

	EndChunk(f);
}

void WriteSynth(CFile *f, Synthesizer *s)
{
	BeginChunk(f, "synth");

	f->WriteStr(s->name);
	f->WriteStr(s->configToString());
	f->WriteStr("");
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
	f->WriteFloat(t->panning);
	f->WriteInt(0); // reserved
	f->WriteInt(0);

	foreach(BarPattern &b, t->bar)
		WriteBar(f, b);

	foreachi(TrackLevel &l, t->level, i)
		WriteTrackLevel(f, &l, i);

	foreach(SampleRef *s, t->sample)
		WriteSampleRef(f, s);

	foreach(Effect *effect, t->fx)
		WriteEffect(f, effect);

	if ((t->type == t->TYPE_TIME) || (t->type == t->TYPE_MIDI))
		WriteSynth(f, t->synth);

	if (t->midi.num > 0)
		WriteMidi(f, t->midi);

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

void FormatNami::saveAudio(AudioFile *a, const string & filename)
{
	tsunami->progress->start(_("speichere nami"), 0);
	a->filename = filename;

//	int length = a->GetLength();
//	int min = a->GetMin();
	CFile *f = FileCreate(filename);
	f->SetBinaryMode(true);

	BeginChunk(f, "nami");

	f->WriteInt(a->sample_rate);

	foreach(Tag &tag, a->tag)
		WriteTag(f, &tag);

	WriteLevelName(f, a->level_name);

	foreach(Sample *sample, a->sample)
		WriteSample(f, sample);

	foreachi(Track *track, a->track, i){
		WriteTrack(f, track);
		tsunami->progress->set(_("speichere nami"), ((float)i + 0.5f) / (float)a->track.num);
	}

	foreach(Effect *effect, a->fx)
		WriteEffect(f, effect);

	EndChunk(f);

	FileClose(f);
	tsunami->progress->end();
}



void FormatNami::saveBuffer(AudioFile *a, BufferBox *b, const string &filename)
{
}


#if 0
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
#endif

SampleRef *__AddEmptySubTrack(Track *t, const Range &r, const string &name)
{
	BufferBox buf;
	buf.resize(r.length());
	t->root->addSample(name, buf);
	return t->addSample(r.start(), t->root->sample.num - 1);
}

struct ChunkStack;

typedef void chunk_reader(ChunkStack*, void*);


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

struct ChunkStack
{
	Array<ChunkLevelData> chunk_data;
	CFile *f;


	void AddChunkHandler(const string &tag, chunk_reader *reader, void *data)
	{
		ChunkHandler h;
		h.tag = tag;
		h.reader = reader;
		h.data = data;
		chunk_data.back().handler.add(h);
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
				h.reader(this, h.data);
				handled = true;
				break;
			}

		if (handled){

			// read nested chunks
			while (f->GetPos() < chunk_data.back().pos)
				ReadChunk(f);

		}else
			tsunami->log->error("unknown nami chunk: " + cname + " (within " + chunk_data[chunk_data.num - 2].tag + ")");


		f->SetPos(chunk_data.back().pos, true);
		chunk_data.pop();
	}
};

void ReadChunkTag(ChunkStack *s, Array<Tag> *tag)
{
	Tag t;
	t.key = s->f->ReadStr();
	t.value = s->f->ReadStr();
	tag->add(t);
}

void ReadChunkLevelName(ChunkStack *s, AudioFile *a)
{
	int num = s->f->ReadInt();
	a->level_name.clear();
	for (int i=0;i<num;i++)
		a->level_name.add(s->f->ReadStr());
}

void ReadChunkEffect(ChunkStack *s, Array<Effect*> *fx)
{
	Effect *e = CreateEffect(s->f->ReadStr());
	e->only_on_selection = s->f->ReadBool();
	e->range.offset = s->f->ReadInt();
	e->range.num = s->f->ReadInt();
	string params = s->f->ReadStr();
	e->configFromString(params);
	string temp = s->f->ReadStr();
	if (temp.find("disabled") >= 0)
		e->enabled = false;
	fx->add(e);
}

void ReadChunkBufferBox(ChunkStack *s, TrackLevel *l)
{
	BufferBox dummy;
	l->buffer.add(dummy);
	BufferBox *b = &l->buffer.back();
	b->offset = s->f->ReadInt();
	int num = s->f->ReadInt();
	b->resize(num);
	s->f->ReadInt(); // channels (2)
	s->f->ReadInt(); // bit (16)

	Array<short> data;
	data.resize(num * 2);

	// read chunk'ed
	int offset = 0;
	for (int n=0;n<(num * 4) / CHUNK_SIZE;n++){
		s->f->ReadBuffer(&data[offset], CHUNK_SIZE);
		tsunami->progress->set((float)s->f->GetPos() / (float)s->f->GetSize());
		offset += CHUNK_SIZE / 2;
	}
	s->f->ReadBuffer(&data[offset], (num * 4) % CHUNK_SIZE);

	// insert
	for (int i=0;i<num;i++){
		b->r[i] =  (float)data[i * 2    ] / 32768.0f;
		b->l[i] =  (float)data[i * 2 + 1] / 32768.0f;
	}
}


void ReadChunkSampleBufferBox(ChunkStack *s, BufferBox *b)
{
	b->offset = s->f->ReadInt();
	int num = s->f->ReadInt();
	b->resize(num);
	s->f->ReadInt(); // channels (2)
	s->f->ReadInt(); // bit (16)

	Array<short> data;
	data.resize(num * 2);
	s->f->ReadBuffer(data.data, num * 4);
	for (int i=0;i<num;i++){
		b->r[i] =  (float)data[i * 2    ] / 32768.0f;
		b->l[i] =  (float)data[i * 2 + 1] / 32768.0f;
	}
}

void ReadChunkSampleRef(ChunkStack *s, Track *t)
{
	string name = s->f->ReadStr();
	int pos = s->f->ReadInt();
	int index = s->f->ReadInt();
	SampleRef *r = t->addSample(pos, index);
	r->volume = s->f->ReadFloat();
	r->muted = s->f->ReadBool();
	r->rep_num = s->f->ReadInt();
	r->rep_delay = s->f->ReadInt();
	s->f->ReadInt(); // reserved
	s->f->ReadInt();
}

void ReadChunkSub(ChunkStack *s, Track *t)
{
	string name = s->f->ReadStr();
	int pos = s->f->ReadInt();
	int length = s->f->ReadInt();
	SampleRef *r = __AddEmptySubTrack(t, Range(pos, length), name);
	r->volume = s->f->ReadFloat();
	r->muted = s->f->ReadBool();
	r->rep_num = s->f->ReadInt();
	r->rep_delay = s->f->ReadInt();
	s->f->ReadInt(); // reserved
	s->f->ReadInt();

	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkSampleBufferBox, &r->buf);
	tsunami->log->error("\"sub\" chunk is deprecated!");
}

void ReadChunkBar(ChunkStack *s, Array<BarPattern> *bar)
{
	BarPattern b;
	b.type = s->f->ReadInt();
	b.length = s->f->ReadInt();
	b.num_beats = s->f->ReadInt();
	b.count = s->f->ReadInt();
	s->f->ReadInt(); // reserved
	bar->add(b);
}

void ReadChunkMidiNote(ChunkStack *s, MidiData *m)
{
	MidiNote n;
	n.range.offset = s->f->ReadInt();
	n.range.num = s->f->ReadInt();
	n.pitch = s->f->ReadInt();
	n.volume = s->f->ReadFloat();
	s->f->ReadInt(); // reserved
	m->add(MidiEvent(n.range.offset, n.pitch, n.volume));
	m->add(MidiEvent(n.range.end(), n.pitch, 0));
}

void ReadChunkMidiEvent(ChunkStack *s, MidiData *m)
{
	MidiEvent e;
	e.pos = s->f->ReadInt();
	e.pitch = s->f->ReadInt();
	e.volume = s->f->ReadFloat();
	s->f->ReadInt(); // reserved
	m->add(e);
}

void ReadChunkMidiEffect(ChunkStack *s, MidiData *m)
{
	MidiEffect *e = CreateMidiEffect(s->f->ReadStr());
	e->only_on_selection = s->f->ReadBool();
	e->range.offset = s->f->ReadInt();
	e->range.num = s->f->ReadInt();
	string params = s->f->ReadStr();
	e->configFromString(params);
	string temp = s->f->ReadStr();
	if (temp.find("disabled") >= 0)
		e->enabled = false;
	m->fx.add(e);
}

void ReadChunkMidiData(ChunkStack *s, MidiData *midi)
{
	s->f->ReadStr();
	s->f->ReadStr();
	s->f->ReadStr();
	s->f->ReadInt(); // reserved

	s->AddChunkHandler("midinote", (chunk_reader*)&ReadChunkMidiNote, midi);
	s->AddChunkHandler("event", (chunk_reader*)&ReadChunkMidiEvent, midi);
	s->AddChunkHandler("note", (chunk_reader*)&ReadChunkMidiNote, midi);
	s->AddChunkHandler("effect", (chunk_reader*)&ReadChunkMidiEffect, midi);
}

void ReadChunkSynth(ChunkStack *s, Track *t)
{
	delete(t->synth);
	t->synth = CreateSynthesizer(s->f->ReadStr());
	t->synth->configFromString(s->f->ReadStr());
	s->f->ReadStr();
	s->f->ReadInt();
}

void ReadChunkSample(ChunkStack *s, AudioFile *a)
{
	Sample *sam = new Sample(Track::TYPE_AUDIO);
	a->sample.add(sam);
	sam->owner = a;
	sam->name = s->f->ReadStr();
	sam->volume = s->f->ReadFloat();
	sam->offset = s->f->ReadInt();
	sam->type = s->f->ReadInt();
	s->f->ReadInt(); // reserved

	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkSampleBufferBox, &sam->buf);
	s->AddChunkHandler("midi", (chunk_reader*)&ReadChunkMidiData, &sam->midi);
}

void ReadChunkTrackLevel(ChunkStack *s, Track *t)
{
	int l = s->f->ReadInt();
	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkBufferBox, &t->level[l]);
}

void ReadChunkTrack(ChunkStack *s, AudioFile *a)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO);
	t->name = s->f->ReadStr();
	t->volume = s->f->ReadFloat();
	t->muted = s->f->ReadBool();
	t->type = s->f->ReadInt();
	t->panning = s->f->ReadFloat();
	s->f->ReadInt(); // reserved
	s->f->ReadInt();
	tsunami->progress->set((float)s->f->GetPos() / (float)s->f->GetSize());

	s->AddChunkHandler("level", (chunk_reader*)&ReadChunkTrackLevel, t);
	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkBufferBox, &t->level[0]);
	s->AddChunkHandler("samref", (chunk_reader*)&ReadChunkSampleRef, t);
	s->AddChunkHandler("sub", (chunk_reader*)&ReadChunkSub, t);
	s->AddChunkHandler("effect", (chunk_reader*)&ReadChunkEffect, &t->fx);
	s->AddChunkHandler("bar", (chunk_reader*)&ReadChunkBar, &t->bar);
	s->AddChunkHandler("midi", (chunk_reader*)&ReadChunkMidiData, &t->midi);
	s->AddChunkHandler("synth", (chunk_reader*)&ReadChunkSynth, t);
}

void ReadChunkNami(ChunkStack *s, AudioFile *a)
{
	a->sample_rate = s->f->ReadInt();

	s->AddChunkHandler("tag", (chunk_reader*)&ReadChunkTag, &a->tag);
	s->AddChunkHandler("effect", (chunk_reader*)&ReadChunkEffect, &a->fx);
	s->AddChunkHandler("lvlname", (chunk_reader*)&ReadChunkLevelName, a);
	s->AddChunkHandler("sample", (chunk_reader*)&ReadChunkSample, a);
	s->AddChunkHandler("track", (chunk_reader*)&ReadChunkTrack, a);
}


void load_nami_file_new(CFile *f, AudioFile *a)
{
	AudioFile *old = tsunami->audio;
	tsunami->audio = a;

	ChunkStack s;
	s.f = f;
	s.chunk_data.add(ChunkLevelData("-top level-", 0));
	s.AddChunkHandler("nami", (chunk_reader*)&ReadChunkNami, a);

	s.ReadChunk(f);

	tsunami->audio = old;
}


void check_empty_subs(AudioFile *a)
{
	/*foreach(Track *t, a->track)
		foreachib(Track *s, t->sub, i)
			if (s->length <= 0){
				tsunami->log->Error("empty sub: " + s->name);
				t->sub.erase(i);
			}*/
}

void FormatNami::loadAudio(AudioFile *a, const string & filename)
{
	msg_db_f("load_nami_file", 1);
	tsunami->progress->set(_("lade nami"), 0);

	// TODO?
	a->tag.clear();

	CFile *f = FileOpen(a->filename);
	f->SetBinaryMode(true);

	load_nami_file_new(f, a);

	FileClose(f);

	// some post processing
	check_empty_subs(a);

	a->updateSelection(Range(0, 0));
}



void FormatNami::loadTrack(Track *t, const string &filename, int offset, int level)
{
}

