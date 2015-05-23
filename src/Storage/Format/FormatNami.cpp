/*
 * FormatNami.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatNami.h"
#include "../../Tsunami.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Stuff/Log.h"
#include "../../View/Helper/Progress.h"
#include "../../Audio/Synth/Synthesizer.h"

#include <FLAC/all.h>
#include <math.h>

const int CHUNK_SIZE = 1 << 16;


FormatNami::FormatNami() :
	Format("Tsunami", "nami", FLAG_AUDIO | FLAG_MIDI | FLAG_FX | FLAG_MULTITRACK | FLAG_TAGS | FLAG_SUBS | FLAG_READ | FLAG_WRITE)
{
	audio = NULL;
	f = NULL;
}

FormatNami::~FormatNami()
{
}

static FLAC__int32 flac_pcm[CHUNK_SIZE/*samples*/ * 2/*channels*/];

FLAC__StreamEncoderWriteStatus FlacCompressWriteCallback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
{
	string *data = (string*)client_data;
	for (unsigned int i=0; i<bytes; i++)
		data->add(buffer[i]);

	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

string FormatNami::compress_buffer(BufferBox &b)
{
	string data;


	bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;

	int channels = 2;
	int bits = min(format_get_bits(audio->default_format), 24);
	float scale = pow(2.0f, bits);

	// allocate the encoder
	FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
	if (!encoder){
		tsunami->log->error("flac: allocating encoder");
		return "";
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bits);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, audio->sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, b.num);

	// initialize encoder
	if (ok){
		init_status = FLAC__stream_encoder_init_stream(encoder, &FlacCompressWriteCallback, NULL, NULL, NULL, (void*)&data);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			tsunami->log->error(string("flac: initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok){
		int p0 = 0;
		size_t left = (size_t)b.num;
		while (ok && left){
			size_t need = (left > CHUNK_SIZE ? (size_t)CHUNK_SIZE : (size_t)left);
			{
				/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
				for (unsigned int i=0;i<need;i++){
					flac_pcm[i * 2 + 0] = (int)(b.r[p0 + i] * scale);
					flac_pcm[i * 2 + 1] = (int)(b.l[p0 + i] * scale);
				}
				/* feed samples to encoder */
				ok = FLAC__stream_encoder_process_interleaved(encoder, flac_pcm, need);
			}
			left -= need;
			p0 += CHUNK_SIZE;
		}
	}

	ok &= FLAC__stream_encoder_finish(encoder);

	if (!ok){
		tsunami->log->error("flac: encoding: FAILED");
		tsunami->log->error(string("   state: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
	}

	FLAC__stream_encoder_delete(encoder);

	return data;
}

struct UncompressData
{
	BufferBox *buf;
	string *data;
	int sample_offset;
	int byte_offset;
	int bits;
	int channels;
};

void FlacUncompressMetaCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	UncompressData *d = (UncompressData*)client_data;
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO){
		d->bits = metadata->data.stream_info.bits_per_sample;
		d->channels = metadata->data.stream_info.channels;
	}
}

FLAC__StreamDecoderWriteStatus FlacUncompressWriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	UncompressData *d = (UncompressData*)client_data;

	// read decoded PCM samples
	BufferBox *buf = d->buf;
	float scale = pow(2.0f, d->bits);
	int offset = d->sample_offset;
	int n = min(frame->header.blocksize, buf->num - offset);
	for (int i=0; i<n; i++)
		for (int j=0;j<d->channels;j++)
			if (j == 0)
				buf->r[offset + i] = buffer[j][i] / scale;
			else
				buf->l[offset + i] = buffer[j][i] / scale;
	d->sample_offset += frame->header.blocksize;

	//flac_read_samples += frame->header.blocksize;
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderReadStatus FlacUncompressReadCallback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	UncompressData *d = (UncompressData*)client_data;
	*bytes = min(*bytes, d->data->num - d->byte_offset);
	if (*bytes > 0){
		memcpy(buffer, (char*)d->data->data + d->byte_offset, *bytes);
		d->byte_offset += *bytes;
	}
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//msg_write("error");
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

void uncompress_buffer(BufferBox &b, string &data)
{
	bool ok = true;

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder)
		throw string("flac: decoder_new()");

	FLAC__stream_decoder_set_metadata_respond(decoder, (FLAC__MetadataType)(FLAC__METADATA_TYPE_STREAMINFO | FLAC__METADATA_TYPE_VORBIS_COMMENT));

	UncompressData d;
	d.buf = &b;
	d.channels = 2;
	d.bits = 16;
	d.data = &data;
	d.byte_offset = 0;
	d.sample_offset = 0;

	FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_stream(
							decoder,
							&FlacUncompressReadCallback,
							NULL, NULL, NULL, NULL,
							&FlacUncompressWriteCallback,
							&FlacUncompressMetaCallback,
							flac_error_callback, &d);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK){
		tsunami->log->error(string("flac: initializing decoder: ") + FLAC__StreamDecoderInitStatusString[init_status]);
		ok = false;
	}

	if (ok){
		ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
		if (!ok){
			tsunami->log->error("flac: decoding FAILED");
			tsunami->log->error(string("   state: ") + FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
		}
	}
	FLAC__stream_decoder_delete(decoder);
}

void strip(string &s)
{
	while((s.num > 0) && (s.back() == ' '))
		s.resize(s.num - 1);
}

void FormatNami::BeginChunk(const string &name)
{
	string s = name + "        ";
	f->WriteBuffer(s.data, 8);
	f->WriteInt(0); // temporary
	ChunkPos.add(f->GetPos());
}

void FormatNami::EndChunk()
{
	int pos = ChunkPos.back();
	ChunkPos.pop();

	int pos0 = f->GetPos();
	f->SetPos(pos - 4, true);
	f->WriteInt(pos0 - pos);
	f->SetPos(pos0, true);
}

void FormatNami::WriteTag(Tag *t)
{
	BeginChunk("tag");
	f->WriteStr(t->key);
	f->WriteStr(t->value);
	EndChunk();
}

void FormatNami::WriteEffect(Effect *e)
{
	BeginChunk("effect");
	f->WriteStr(e->name);
	f->WriteBool(e->only_on_selection);
	f->WriteInt(e->range.offset);
	f->WriteInt(e->range.num);
	f->WriteStr(e->configToString());
	f->WriteStr(e->enabled ? "" : "disabled");
	EndChunk();
}

void FormatNami::WriteBufferBox(BufferBox *b)
{
	BeginChunk("bufbox");
	int channels = 2;
	f->WriteInt(b->offset);
	f->WriteInt(b->num);
	f->WriteInt(channels);
	f->WriteInt(format_get_bits(audio->default_format));

	string data;
	msg_write(audio->compression);
	if (audio->compression == 0){
		if (!b->exports(data, channels, audio->default_format))
			tsunami->log->warning(_("Amplitude zu gro&s, Signal &ubersteuert."));
	}else{

		int uncompressed_size = b->num * channels * format_get_bits(audio->default_format) / 8;
		data = compress_buffer(*b);
		msg_write(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
	}
	f->WriteBuffer(data.data, data.num);

	EndChunk();
}

void FormatNami::WriteSample(Sample *s)
{
	BeginChunk("sample");
	f->WriteStr(s->name);
	f->WriteFloat(s->volume);
	f->WriteInt(s->offset);
	f->WriteInt(s->type); // reserved
	f->WriteInt(0);
	if (s->type == Track::TYPE_AUDIO)
		WriteBufferBox(&s->buf);
	else if (s->type == Track::TYPE_MIDI)
		WriteMidi(s->midi);
	EndChunk();
}

void FormatNami::WriteSampleRef(SampleRef *s)
{
	BeginChunk("samref");

	f->WriteStr(s->origin->name);
	f->WriteInt(s->pos);
	f->WriteInt(s->origin->get_index());
	f->WriteFloat(s->volume);
	f->WriteBool(s->muted);
	f->WriteInt(s->rep_num);
	f->WriteInt(s->rep_delay);
	f->WriteInt(0); // reserved
	f->WriteInt(0);

	EndChunk();
}

void FormatNami::WriteBar(BarPattern &b)
{
	BeginChunk("bar");

	f->WriteInt(b.type);
	f->WriteInt(b.length);
	f->WriteInt(b.num_beats);
	f->WriteInt(b.count);
	f->WriteInt(0); // reserved

	EndChunk();
}

void FormatNami::WriteMarker(TrackMarker &m)
{
	BeginChunk("marker");
	f->WriteInt(m.pos);
	f->WriteStr(m.text);
	f->WriteInt(0); // reserved
	EndChunk();
}

void FormatNami::WriteMidiEvent(MidiEvent &e)
{
	BeginChunk("event");

	f->WriteInt(e.pos);
	f->WriteInt(e.pitch);
	f->WriteFloat(e.volume);
	f->WriteInt(0); // reserved

	EndChunk();
}

void FormatNami::WriteMidiEffect(MidiEffect *e)
{
	BeginChunk("effect");
	f->WriteStr(e->name);
	f->WriteBool(e->only_on_selection);
	f->WriteInt(e->range.offset);
	f->WriteInt(e->range.num);
	f->WriteStr(e->configToString());
	f->WriteStr(e->enabled ? "" : "disabled");
	EndChunk();
}

void FormatNami::WriteMidi(MidiData &m)
{
	BeginChunk("midi");

	f->WriteStr("");
	f->WriteStr("");
	f->WriteStr("");
	f->WriteInt(0); // reserved

	foreach(MidiEvent &e, m)
		WriteMidiEvent(e);

	foreach(MidiEffect *e, m.fx)
		WriteMidiEffect(e);

	EndChunk();
}

void FormatNami::WriteSynth(Synthesizer *s)
{
	BeginChunk("synth");

	f->WriteStr(s->name);
	f->WriteStr(s->configToString());
	f->WriteStr("");
	f->WriteInt(0); // reserved

	EndChunk();
}

void FormatNami::WriteTrackLevel(TrackLevel *l, int level_no)
{
	BeginChunk("level");
	f->WriteInt(level_no);

	foreach(BufferBox &b, l->buffers)
		WriteBufferBox(&b);

	EndChunk();
}

void FormatNami::WriteTrack(Track *t)
{
	BeginChunk("track");

	f->WriteStr(t->name);
	f->WriteFloat(t->volume);
	f->WriteBool(t->muted);
	f->WriteInt(t->type);
	f->WriteFloat(t->panning);
	f->WriteInt(0); // reserved
	f->WriteInt(0);

	foreach(BarPattern &b, t->bars)
		WriteBar(b);

	foreachi(TrackLevel &l, t->levels, i)
		WriteTrackLevel(&l, i);

	foreach(SampleRef *s, t->samples)
		WriteSampleRef(s);

	foreach(Effect *effect, t->fx)
		WriteEffect(effect);

	foreach(TrackMarker &m, t->markers)
		WriteMarker(m);

	if ((t->type == t->TYPE_TIME) or (t->type == t->TYPE_MIDI))
		WriteSynth(t->synth);

	if (t->midi.num > 0)
		WriteMidi(t->midi);

	EndChunk();
}

void FormatNami::WriteLevelName()
{
	BeginChunk("lvlname");

	f->WriteInt(audio->level_names.num);
	foreach(string &l, audio->level_names)
		f->WriteStr(l);

	EndChunk();
}

void FormatNami::WriteFormat()
{
	BeginChunk("format");

	f->WriteInt(audio->sample_rate);
	f->WriteInt(audio->default_format);
	f->WriteInt(2); // channels
	f->WriteInt(audio->compression);
	f->WriteInt(0); // reserved
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);

	EndChunk();
}

void FormatNami::saveAudio(AudioFile *a, const string & filename)
{
	tsunami->progress->start(_("speichere nami"), 0);
	a->filename = filename;
	audio = a;

//	int length = a->GetLength();
//	int min = a->GetMin();
	f = FileCreate(filename);
	f->SetBinaryMode(true);

	BeginChunk("nami");

	f->WriteInt(a->sample_rate);
	WriteFormat();

	foreach(Tag &tag, a->tags)
		WriteTag(&tag);

	WriteLevelName();

	foreach(Sample *sample, a->samples)
		WriteSample(sample);

	foreachi(Track *track, a->tracks, i){
		WriteTrack(track);
		tsunami->progress->set(_("speichere nami"), ((float)i + 0.5f) / (float)a->tracks.num);
	}

	foreach(Effect *effect, a->fx)
		WriteEffect(effect);

	EndChunk();

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
	return t->addSample(r.start(), t->root->samples.num - 1);
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
	ChunkLevelData(const string &_tag, int _pos, int _size)
	{ tag = _tag; pos = _pos; size = _size; }
	int pos;
	int size;
	string tag;
	Array<ChunkHandler> handler;
	int end()
	{
		return pos + size;
	}
};

struct ChunkStack
{
	Array<ChunkLevelData> chunk_data;
	CFile *f;
	AudioFile *a;


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
		ChunkLevelData cur = chunk_data.back();
		string cname;
		cname.resize(8);
		f->ReadBuffer(cname.data, 8);
		strip(cname);
		int size = f->ReadInt();
		chunk_data.add(ChunkLevelData(cname, f->GetPos(), size));
		if (size < 0)
			throw string("chunk with negative size found");
		if (chunk_data.back().end() > cur.end())
			throw string("inner chunk is larger than its parent");


		bool handled = false;
		foreach(ChunkHandler &h, cur.handler)
			if (cname == h.tag){
				h.reader(this, h.data);
				handled = true;
				break;
			}

		if (handled){

			// read nested chunks
			while (f->GetPos() < chunk_data.back().end())
				ReadChunk(f);

		}else
			tsunami->log->error("unknown nami chunk: " + cname + " (within " + chunk_data[chunk_data.num - 2].tag + ")");


		f->SetPos(chunk_data.back().end(), true);
		chunk_data.pop();
	}
};

void ReadChunkFormat(ChunkStack *s, AudioFile *a)
{
	a->sample_rate = s->f->ReadInt();
	a->default_format = (SampleFormat)s->f->ReadInt();
	s->f->ReadInt(); // channels
	a->compression = s->f->ReadInt();
	s->f->ReadInt();
	s->f->ReadInt();
	s->f->ReadInt();
	s->f->ReadInt();
}

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
	a->level_names.clear();
	for (int i=0;i<num;i++)
		a->level_names.add(s->f->ReadStr());
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
	l->buffers.add(dummy);
	BufferBox *b = &l->buffers.back();
	b->offset = s->f->ReadInt();
	int num = s->f->ReadInt();
	b->resize(num);
	int channels = s->f->ReadInt(); // channels (2)
	int bits = s->f->ReadInt(); // bit (16)

	string data;

	int bytes = s->chunk_data.back().size - 16;
	data.resize(bytes);//num * (bits / 8) * channels);

	// read chunk'ed
	int offset = 0;
	for (int n=0; n<data.num / CHUNK_SIZE; n++){
		s->f->ReadBuffer(&data[offset], CHUNK_SIZE);
		tsunami->progress->set((float)s->f->GetPos() / (float)s->f->GetSize());
		offset += CHUNK_SIZE;
	}
	s->f->ReadBuffer(&data[offset], data.num % CHUNK_SIZE);

	// insert

	if (s->a->compression > 0){
		//throw string("can't read compressed nami files yet");
		uncompress_buffer(*b, data);

	}else{
		b->import(data.data, channels, format_for_bits(bits), num);
	}
}


void ReadChunkSampleBufferBox(ChunkStack *s, BufferBox *b)
{
	b->offset = s->f->ReadInt();
	int num = s->f->ReadInt();
	b->resize(num);
	s->f->ReadInt(); // channels (2)
	s->f->ReadInt(); // bit (16)

	string data;
	data.resize(num * 4);
	s->f->ReadBuffer(data.data, data.num);
	b->import(data.data, 2, SAMPLE_FORMAT_16, num);
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

void ReadChunkMarker(ChunkStack *s, Array<TrackMarker> *markers)
{
	TrackMarker m;
	m.pos = s->f->ReadInt();
	m.text = s->f->ReadStr();
	s->f->ReadInt(); // reserved
	markers->add(m);
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
	a->samples.add(sam);
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
	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkBufferBox, &t->levels[l]);
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
	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkBufferBox, &t->levels[0]);
	s->AddChunkHandler("samref", (chunk_reader*)&ReadChunkSampleRef, t);
	s->AddChunkHandler("sub", (chunk_reader*)&ReadChunkSub, t);
	s->AddChunkHandler("effect", (chunk_reader*)&ReadChunkEffect, &t->fx);
	s->AddChunkHandler("bar", (chunk_reader*)&ReadChunkBar, &t->bars);
	s->AddChunkHandler("midi", (chunk_reader*)&ReadChunkMidiData, &t->midi);
	s->AddChunkHandler("synth", (chunk_reader*)&ReadChunkSynth, t);
	s->AddChunkHandler("marker", (chunk_reader*)&ReadChunkMarker, &t->markers);
}

void ReadChunkNami(ChunkStack *s, AudioFile *a)
{
	a->sample_rate = s->f->ReadInt();

	s->AddChunkHandler("format", (chunk_reader*)&ReadChunkFormat, a);
	s->AddChunkHandler("tag", (chunk_reader*)&ReadChunkTag, &a->tags);
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
	s.a = a;
	s.chunk_data.add(ChunkLevelData("-top level-", 0, f->GetSize()));
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
	a->tags.clear();

	CFile *f = FileOpen(a->filename);
	f->SetBinaryMode(true);

	try{
		load_nami_file_new(f, a);
	}catch(string &s){
		tsunami->log->error("loading nami: " + s);
	}

	FileClose(f);

	// some post processing
	check_empty_subs(a);

	a->updateSelection(Range(0, 0));
}



void FormatNami::loadTrack(Track *t, const string &filename, int offset, int level)
{
}

