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
#include "../../Audio/Synth/Synthesizer.h"

#include <FLAC/all.h>
#include <math.h>

const int CHUNK_SIZE = 1 << 16;


FormatNami::FormatNami() :
	Format("Tsunami", "nami", FLAG_AUDIO | FLAG_MIDI | FLAG_FX | FLAG_MULTITRACK | FLAG_TAGS | FLAG_SUBS | FLAG_READ | FLAG_WRITE)
{
	song = NULL;
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

string compress_buffer(BufferBox &b, StorageOperationData *od)
{
	string data;


	bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;

	int channels = 2;
	int bits = min(format_get_bits(od->song->default_format), 24);
	float scale = pow(2.0f, bits-1);

	// allocate the encoder
	FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
	if (!encoder){
		od->error("flac: allocating encoder");
		return "";
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bits);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, od->song->sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, b.num);

	// initialize encoder
	if (ok){
		init_status = FLAC__stream_encoder_init_stream(encoder, &FlacCompressWriteCallback, NULL, NULL, NULL, (void*)&data);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			od->error(string("flac: initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok){
		int p0 = 0;
		size_t left = (size_t)b.num;
		while (ok and left){
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
		od->error("flac: encoding: FAILED");
		od->error(string("   state: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
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
	float scale = pow(2.0f, d->bits-1);
	int offset = d->sample_offset;
	int n = min(frame->header.blocksize, buf->num - offset);
	//msg_write(format("write %d  offset=%d  buf=%d", n, offset, buf->num));
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
	//msg_write(format("read %d", *bytes));
	if (*bytes <= 0)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	memcpy(buffer, (char*)d->data->data + d->byte_offset, *bytes);
	d->byte_offset += *bytes;
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//msg_write("error");
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

void uncompress_buffer(BufferBox &b, string &data, StorageOperationData *od)
{
	bool ok = true;

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder)
		throw string("flac: decoder_new()");

	FLAC__stream_decoder_set_metadata_respond(decoder, (FLAC__MetadataType)(FLAC__METADATA_TYPE_STREAMINFO));

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
		od->error(string("flac: initializing decoder: ") + FLAC__StreamDecoderInitStatusString[init_status]);
		ok = false;
	}

	if (ok){
		ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
		if (!ok){
			od->error("flac: decoding FAILED");
			od->error(string("   state: ") + FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
		}
	}
	FLAC__stream_decoder_delete(decoder);
}

void strip(string &s)
{
	while((s.num > 0) and (s.back() == ' '))
		s.resize(s.num - 1);
}

class FileChunkBasic
{
public:
	FileChunkBasic(const string &_name)
	{
		name = _name;
		root = NULL;
		context = NULL;
	}
	virtual ~FileChunkBasic()
	{
		foreach(FileChunkBasic *c, children)
			delete(c);
	}
	virtual void write(File *f) = 0;
	virtual void read(File *f) = 0;
	virtual void create(){ throw string("no data creation... " + name); }
	virtual void on_notify(){};
	virtual void on_unhandled(){};
	virtual void on_error(const string &message){}
	virtual void on_warn(const string &message){}
	virtual void on_info(const string &message){}
	string name;

	void notify()
	{
		if (root)
			root->on_notify();
	}
	void info(const string &message)
	{
		if (root)
			root->on_info(message);
	}
	void warn(const string &message)
	{
		if (root)
			root->on_warn(message);
	}
	void error(const string &message)
	{
		if (root)
			root->on_error(message);
	}

	void add_child(FileChunkBasic *c)
	{
		children.add(c);
	}
	void set_root(FileChunkBasic *r)
	{
		root = r;
		foreach(FileChunkBasic *c, children)
			c->set_root(r);
	}

	virtual void set(void *t){};
	virtual void set_parent(void *t){};
	virtual void *get(){ return NULL; }

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
		Context(File *_f)
		{
			f = _f;
		}
		void pop()
		{
			layers.pop();
		}
		void push(const Layer &l)
		{
			layers.add(l);
		}
		int end()
		{
			if (layers.num > 0)
				return layers.back().pos0 + layers.back().size;
			return 2000000000;
		}
		string str()
		{
			string s;
			foreach(Layer &l, layers){
				if (s.num > 0)
					s += "/";
				s += l.name;
			}
			return s;
		}
		File *f;
		Array<Layer> layers;
	};
	Context *context;
	Array<FileChunkBasic*> children;
	FileChunkBasic *root;

	virtual void write_subs(){}

	FileChunkBasic *get_sub(const string &name)
	{
		foreach(FileChunkBasic *c, children)
			if (c->name == name){
				c->context = context;
				c->set_parent(get());
				return c;
			}
		throw string("no sub... " + name + " in " + context->str());
	}

	void write_sub(const string &name, void *p)
	{
		FileChunkBasic *c = get_sub(name);
		c->set(p);
		c->write_complete();
	}

	void write_sub_parray(const string &name, DynamicArray &a)
	{
		FileChunkBasic *c = get_sub(name);
		Array<void*> *pa = (Array<void*>*)&a;
		for (int i=0; i<pa->num; i++){
			c->set((*pa)[i]);
			c->write_complete();
		}
	}
	void write_sub_array(const string &name, DynamicArray &a)
	{
		FileChunkBasic *c = get_sub(name);
		for (int i=0; i<a.num; i++){
			c->set((char*)a.data + a.element_size * i);
			c->write_complete();
		}
	}

	//int chunk_pos;
	void write_begin_chunk(File *f)
	{
		string s = name + "        ";
		f->WriteBuffer(s.data, 8);
		f->WriteInt(0); // temporary
		context->push(Context::Layer(name, context->f->GetPos(), 0));
	}

	void write_end_chunk(File *f)
	{
		int pos = f->GetPos();
		int chunk_pos = context->layers.back().pos0;
		f->SetPos(chunk_pos - 4, true);
		f->WriteInt(pos - chunk_pos);
		f->SetPos(pos, true);
		context->pop();
	}
	void write_complete()
	{
		File *f = context->f;
		write_begin_chunk(f);
		write_contents();
		write_end_chunk(f);
	}
	void write_contents()
	{
		File *f = context->f;
		write(f);
		write_subs();
	}


	void read_contents()
	{
		File *f = context->f;

		read(f);

		// read nested chunks
		while (f->GetPos() < context->end()){

			string cname;
			cname.resize(8);
			f->ReadBuffer(cname.data, 8);
			strip(cname);
			int size = f->ReadInt();

			int pos0 = f->GetPos();

			if (size < 0)
				throw string("chunk with negative size found");
			if (pos0 + size > context->end())
				throw string("inner chunk is larger than its parent");

			context->push(Context::Layer(cname, pos0, size));



			bool ok = false;
			foreach(FileChunkBasic *c, children)
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
};

template<class PT, class T>
class FileChunk : public FileChunkBasic
{
public:
	FileChunk(const string &name) : FileChunkBasic(name){ me = NULL; parent = NULL; }
	virtual void *get()
	{
		return me;
	}
	virtual void set(void *_me)
	{
		me = (T*)_me;
	}
	virtual void set_parent(void *_parent)
	{
		parent = (PT*)_parent;
	}
	T *me;
	PT *parent;
};

class FileChunkTag : public FileChunk<Song,Tag>
{
public:
	FileChunkTag() : FileChunk<Song,Tag>("tag"){}
	virtual void create()
	{
		parent->tags.add(Tag());
		me = &parent->tags.back();
	}
	virtual void read(File *f)
	{
		me->key = f->ReadStr();
		me->value = f->ReadStr();
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->key);
		f->WriteStr(me->value);
	}
};

class FileChunkLevelName : public FileChunk<Song,Song>
{
public:
	FileChunkLevelName() : FileChunk<Song,Song>("lvlname"){}
	virtual void create(){ me = parent; }
	virtual void read(File *f)
	{
		int num = f->ReadInt();
		me->level_names.clear();
		for (int i=0;i<num;i++)
			me->level_names.add(f->ReadStr());
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->level_names.num);
		for (int i=0; i<me->level_names.num; i++)
			f->WriteStr(me->level_names[i]);
	}
};

class FileChunkFormat : public FileChunk<Song,Song>
{
public:
	FileChunkFormat() : FileChunk<Song,Song>("format"){}
	virtual void create(){ me = parent; }
	virtual void read(File *f)
	{
		me->sample_rate = f->ReadInt();
		me->default_format = (SampleFormat)f->ReadInt();
		f->ReadInt(); // channels
		me->compression = f->ReadInt();
		f->ReadInt();
		f->ReadInt();
		f->ReadInt();
		f->ReadInt();
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->sample_rate);
		f->WriteInt(me->default_format);
		f->WriteInt(2); // channels
		f->WriteInt(me->compression);
		f->WriteInt(0); // reserved
		f->WriteInt(0);
		f->WriteInt(0);
		f->WriteInt(0);
	}
};

class FileChunkEffect : public FileChunk<Track,Effect>
{
public:
	FileChunkEffect() : FileChunk<Track,Effect>("effect"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		me = CreateEffect(f->ReadStr());
		me->only_on_selection = f->ReadBool();
		me->range.offset = f->ReadInt();
		me->range.num = f->ReadInt();
		string params = f->ReadStr();
		me->configFromString(params);
		string temp = f->ReadStr();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->name);
		f->WriteBool(me->only_on_selection);
		f->WriteInt(me->range.offset);
		f->WriteInt(me->range.num);
		f->WriteStr(me->configToString());
		f->WriteStr(me->enabled ? "" : "disabled");
	}
};

class FileChunkGlobalEffect : public FileChunk<Song,Effect>
{
public:
	FileChunkGlobalEffect() : FileChunk<Song,Effect>("effect"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		me = CreateEffect(f->ReadStr());
		me->only_on_selection = f->ReadBool();
		me->range.offset = f->ReadInt();
		me->range.num = f->ReadInt();
		string params = f->ReadStr();
		me->configFromString(params);
		string temp = f->ReadStr();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->name);
		f->WriteBool(me->only_on_selection);
		f->WriteInt(me->range.offset);
		f->WriteInt(me->range.num);
		f->WriteStr(me->configToString());
		f->WriteStr(me->enabled ? "" : "disabled");
	}
};

class FileChunkBufferBox : public FileChunk<TrackLevel,BufferBox>
{
public:
	FileChunkBufferBox() : FileChunk<TrackLevel,BufferBox>("bufbox"){}
	virtual void create()
	{
		BufferBox dummy;
		parent->buffers.add(dummy);
		me = &parent->buffers.back();
	}
	virtual void read(File *f)
	{
		me->offset = f->ReadInt();
		int num = f->ReadInt();
		me->resize(num);
		int channels = f->ReadInt(); // channels (2)
		int bits = f->ReadInt(); // bit (16)

		string data;

		int bytes = context->layers.back().size - 16;
		data.resize(bytes);//num * (bits / 8) * channels);

		// read chunk'ed
		int offset = 0;
		for (int n=0; n<data.num / CHUNK_SIZE; n++){
			f->ReadBuffer(&data[offset], CHUNK_SIZE);
			notify();
			offset += CHUNK_SIZE;
		}
		f->ReadBuffer(&data[offset], data.num % CHUNK_SIZE);

		// insert

		StorageOperationData *od = (StorageOperationData*)root->get();
		if (od->song->compression > 0){
			//throw string("can't read compressed nami files yet");
			uncompress_buffer(*me, data, od);

		}else{
			me->import(data.data, channels, format_for_bits(bits), num);
		}
	}
	virtual void write(File *f)
	{
		StorageOperationData *od = (StorageOperationData*)root->get();
		Song *song = od->song;

		int channels = 2;
		f->WriteInt(me->offset);
		f->WriteInt(me->num);
		f->WriteInt(channels);
		f->WriteInt(format_get_bits(song->default_format));

		string data;
		if (song->compression == 0){
			if (!me->exports(data, channels, song->default_format))
				warn(_("Amplitude zu gro&s, Signal &ubersteuert."));
		}else{

			int uncompressed_size = me->num * channels * format_get_bits(song->default_format) / 8;
			data = compress_buffer(*me, od);
			msg_write(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
		}
		f->WriteBuffer(data.data, data.num);
	}
};

class FileChunkSampleBufferBox : public FileChunk<Sample,BufferBox>
{
public:
	FileChunkSampleBufferBox() : FileChunk<Sample,BufferBox>("bufbox"){}
	virtual void create()
	{
		me = &parent->buf;
	}
	virtual void read(File *f)
	{
		me->offset = f->ReadInt();
		int num = f->ReadInt();
		me->resize(num);
		f->ReadInt(); // channels (2)
		f->ReadInt(); // bit (16)

		string data;
		data.resize(num * 4);
		f->ReadBuffer(data.data, data.num);
		me->import(data.data, 2, SAMPLE_FORMAT_16, num);

		notify();
	}
	virtual void write(File *f)
	{
		throw string("write SampleBufferBox...");
	}
};

class FileChunkSampleRef : public FileChunk<Track,SampleRef>
{
public:
	FileChunkSampleRef() : FileChunk<Track,SampleRef>("samref"){}
	virtual void create()
	{}
	virtual void read(File *f)
	{
		string name = f->ReadStr();
		int pos = f->ReadInt();
		int index = f->ReadInt();
		me = parent->addSample(pos, index);
		me->volume = f->ReadFloat();
		me->muted = f->ReadBool();
		me->rep_num = f->ReadInt();
		me->rep_delay = f->ReadInt();
		f->ReadInt(); // reserved
		f->ReadInt();
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->origin->name);
		f->WriteInt(me->pos);
		f->WriteInt(me->origin->get_index());
		f->WriteFloat(me->volume);
		f->WriteBool(me->muted);
		f->WriteInt(me->rep_num);
		f->WriteInt(me->rep_delay);
		f->WriteInt(0); // reserved
		f->WriteInt(0);
	}
};

#if 0
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
#endif


class FileChunkMidiEvent : public FileChunk<MidiNoteData,MidiEvent>
{
public:
	FileChunkMidiEvent() : FileChunk<MidiNoteData,MidiEvent>("event"){}
	virtual void create()
	{
		//parent->add(MidiEvent());
		//me = &parent->back();
	}
	virtual void read(File *f)
	{
		MidiEvent e;
		e.pos = f->ReadInt();
		e.pitch = f->ReadInt();
		e.volume = f->ReadFloat();
		f->ReadInt(); // reserved

		int unended = -1;
		foreachi(MidiNote &n, *parent, i)
			if ((n.pitch == e.pitch) and (n.range.num == -1))
				unended = i;

		if ((unended >= 0) and (e.volume == 0)){
			(*parent)[unended].range.set_end(e.pos);
		}else if ((unended < 0) and (e.volume > 0)){
			parent->add(MidiNote(Range(e.pos, -1), e.pitch, e.volume));
		}else if (unended >= 0){
			error("nami/midi: starting new note without ending old one");
		}else{
			error("nami/midi: no note to end");
		}
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->pos);
		f->WriteInt(me->pitch);
		f->WriteFloat(me->volume);
		f->WriteInt(0); // reserved
	}
};

class FileChunkMidiEffect : public FileChunk<MidiNoteData,MidiEffect>
{
public:
	FileChunkMidiEffect() : FileChunk<MidiNoteData,MidiEffect>("effect"){}
	virtual void create()
	{}
	virtual void read(File *f)
	{
		me = CreateMidiEffect(f->ReadStr());
		me->only_on_selection = f->ReadBool();
		me->range.offset = f->ReadInt();
		me->range.num = f->ReadInt();
		string params = f->ReadStr();
		me->configFromString(params);
		string temp = f->ReadStr();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->name);
		f->WriteBool(me->only_on_selection);
		f->WriteInt(me->range.offset);
		f->WriteInt(me->range.num);
		f->WriteStr(me->configToString());
		f->WriteStr(me->enabled ? "" : "disabled");
	}
};

// DEPRECATED
class FileChunkMidiNote : public FileChunk<MidiNoteData,MidiNote>
{
public:
	FileChunkMidiNote() : FileChunk<MidiNoteData,MidiNote>("note"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		MidiNote n;
		n.range.offset = f->ReadInt();
		n.range.num = f->ReadInt();
		n.pitch = f->ReadInt();
		n.volume = f->ReadFloat();
		f->ReadInt(); // reserved
		parent->add(n);
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->range.offset);
		f->WriteInt(me->range.num);
		f->WriteInt(me->pitch);
		f->WriteFloat(me->volume);
		f->WriteInt(0); // reserved
	}
};

class FileChunkSampleMidiData : public FileChunk<Sample,MidiNoteData>
{
public:
	FileChunkSampleMidiData() : FileChunk<Sample,MidiNoteData>("midi")
	{
		add_child(new FileChunkMidiEvent);
		add_child(new FileChunkMidiNote);
		add_child(new FileChunkMidiEffect);
		//s->AddChunkHandler("midinote", (chunk_reader*)&ReadChunkMidiNote, midi);
	}
	virtual void create(){ me = &parent->midi; }
	virtual void read(File *f)
	{
		f->ReadStr();
		f->ReadStr();
		f->ReadStr();
		f->ReadInt(); // reserved
	}
	virtual void write(File *f)
	{
		f->WriteStr("");
		f->WriteStr("");
		f->WriteStr("");
		f->WriteInt(0);
	}
	virtual void write_subs()
	{
		//Array<MidiEvent> events = me->getEvents(Range::ALL);
		//write_sub_array("event", events);
		write_sub_array("note", *me);
		write_sub_parray("effect", me->fx);
	}
};

class FileChunkTrackMidiData : public FileChunk<Track,MidiNoteData>
{
public:
	FileChunkTrackMidiData() : FileChunk<Track,MidiNoteData>("midi")
	{
		add_child(new FileChunkMidiEvent);
		add_child(new FileChunkMidiNote);
		add_child(new FileChunkMidiEffect);
		//s->AddChunkHandler("midinote", (chunk_reader*)&ReadChunkMidiNote, midi);
	}
	virtual void create(){ me = &parent->midi; }
	virtual void read(File *f)
	{
		f->ReadStr();
		f->ReadStr();
		f->ReadStr();
		f->ReadInt(); // reserved
	}
	virtual void write(File *f)
	{
		f->WriteStr("");
		f->WriteStr("");
		f->WriteStr("");
		f->WriteInt(0);
	}
	virtual void write_subs()
	{
		//Array<MidiEvent> events = me->getEvents(Range::ALL);
		//write_sub_array("event", events);
		write_sub_array("note", *me);
		write_sub_parray("effect", me->fx);
	}
};

class FileChunkSample : public FileChunk<Song,Sample>
{
public:
	FileChunkSample() : FileChunk<Song,Sample>("sample")
	{
		add_child(new FileChunkSampleBufferBox);
		add_child(new FileChunkSampleMidiData);
	}
	virtual void create()
	{
		me = new Sample(Track::TYPE_AUDIO);
		me->owner = parent;
		parent->samples.add(me);
	}
	virtual void read(File *f)
	{
		me->name = f->ReadStr();
		me->volume = f->ReadFloat();
		me->offset = f->ReadInt();
		me->type = f->ReadInt();
		f->ReadInt(); // reserved
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->name);
		f->WriteFloat(me->volume);
		f->WriteInt(me->offset);
		f->WriteInt(me->type); // reserved
		f->WriteInt(0);
	}
	virtual void write_subs()
	{
		if (me->type == Track::TYPE_AUDIO)
			write_sub("bufbox", &me->buf);
		else if (me->type == Track::TYPE_MIDI)
			write_sub("midi", &me->midi);
	}
};

class FileChunkTrackLevel : public FileChunk<Track,TrackLevel>
{
public:
	int n;
	FileChunkTrackLevel() : FileChunk<Track,TrackLevel>("level")
	{
		n = 0;
		add_child(new FileChunkBufferBox);
	}
	virtual void create()
	{
		//me = &parent->levels[n];
	}
	virtual void read(File *f)
	{
		n = f->ReadInt();
		me = &parent->levels[n];
	}
	virtual void write(File *f)
	{
		f->WriteInt(parent->levels.index(me));
	}
	virtual void write_subs()
	{
		write_sub_array("bufbox", me->buffers);
	}
};

class FileChunkSynthesizer : public FileChunk<Track,Synthesizer>
{
public:
	FileChunkSynthesizer() : FileChunk<Track,Synthesizer>("synth"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		me = CreateSynthesizer(f->ReadStr());
		me->configFromString(f->ReadStr());
		f->ReadStr();
		f->ReadInt();

		delete(parent->synth);
		parent->synth = me;
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->name);
		f->WriteStr(me->configToString());
		f->WriteStr("");
		f->WriteInt(0); // reserved
	}
};

class FileChunkBar : public FileChunk<Song,BarPattern>
{
public:
	FileChunkBar() : FileChunk<Song,BarPattern>("bar"){}
	virtual void create(){ me = NULL; }
	virtual void read(File *f)
	{
		BarPattern b;
		b.type = f->ReadInt();
		b.length = f->ReadInt();
		b.num_beats = f->ReadInt();
		if (b.type == BarPattern::TYPE_PAUSE)
			b.num_beats = 0;
		int count = f->ReadInt();
		f->ReadInt(); // reserved
		for (int i=0; i<count; i++)
			parent->bars.add(b);
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->type);
		f->WriteInt(me->length);
		f->WriteInt(me->num_beats);
		f->WriteInt(1);
		f->WriteInt(0); // reserved
	}
};

class FileChunkTrackBar : public FileChunk<Track,BarPattern>
{
public:
	FileChunkTrackBar() : FileChunk<Track,BarPattern>("bar"){}
	virtual void create(){ me = NULL; }
	virtual void read(File *f)
	{
		BarPattern b;
		b.type = f->ReadInt();
		b.length = f->ReadInt();
		b.num_beats = f->ReadInt();
		if (b.type == BarPattern::TYPE_PAUSE)
			b.num_beats = 0;
		int count = f->ReadInt();
		f->ReadInt(); // reserved
		for (int i=0; i<count; i++)
			parent->song->bars.add(b);
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->type);
		f->WriteInt(me->length);
		f->WriteInt(me->num_beats);
		f->WriteInt(1);
		f->WriteInt(0); // reserved
	}
};

class FileChunkMarker : public FileChunk<Track,TrackMarker>
{
public:
	FileChunkMarker() : FileChunk<Track,TrackMarker>("marker"){}
	virtual void create()
	{
		parent->markers.add(TrackMarker());
		me = &parent->markers.back();
	}
	virtual void read(File *f)
	{
		me->pos = f->ReadInt();
		me->text = f->ReadStr();
		f->ReadInt(); // reserved
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->pos);
		f->WriteStr(me->text);
		f->WriteInt(0);
	}
};

class FileChunkTuning : public FileChunk<Track,Track>
{
public:
	FileChunkTuning() : FileChunk<Track,Track>("tuning"){}
	virtual void create()
	{
		me = parent;
	}
	virtual void read(File *f)
	{
		me->tuning.resize(f->ReadInt());
		for (int i=0; i<me->tuning.num; i++)
			me->tuning[i] = f->ReadInt();
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->tuning.num);
		for (int i=0; i<me->tuning.num; i++)
			f->WriteInt(me->tuning[i]);
	}
};

class FileChunkTrack : public FileChunk<Song,Track>
{
public:
	FileChunkTrack() : FileChunk<Song,Track>("track")
	{
		add_child(new FileChunkTuning);
		add_child(new FileChunkTrackLevel);
		add_child(new FileChunkSynthesizer);
		add_child(new FileChunkEffect);
		add_child(new FileChunkTrackMidiData);
		add_child(new FileChunkTrackBar);
		add_child(new FileChunkMarker);
		add_child(new FileChunkSampleRef);
			//s->AddChunkHandler("sub", (chunk_reader*)&ReadChunkSub, t);
	}
	virtual void create()
	{
		me = parent->addTrack(Track::TYPE_AUDIO);
	}
	virtual void read(File *f)
	{
		me->name = f->ReadStr();
		me->volume = f->ReadFloat();
		me->muted = f->ReadBool();
		me->type = f->ReadInt();
		me->panning = f->ReadFloat();
		me->instrument = Instrument(f->ReadInt());
		f->ReadInt(); // reserved

		me->tuning = me->instrument.default_tuning();

		notify();
	}
	virtual void write(File *f)
	{
		f->WriteStr(me->name);
		f->WriteFloat(me->volume);
		f->WriteBool(me->muted);
		f->WriteInt(me->type);
		f->WriteFloat(me->panning);
		f->WriteInt(me->instrument.type);
		f->WriteInt(0); // reserved

		notify();
	}
	virtual void write_subs()
	{
		if (!me->instrument.is_default_tuning(me->tuning))
			write_sub("tuning", me);
		write_sub_array("level", me->levels);
		write_sub_parray("samref", me->samples);
		write_sub_parray("effect", me->fx);
		write_sub_array("marker", me->markers);
		if ((me->type == me->TYPE_TIME) or (me->type == me->TYPE_MIDI))
		if (me->synth->name != "Dummy")
			write_sub("synth", me->synth);
		if (me->midi.num > 0)
			write_sub("midi", &me->midi);
	}
};

class FileChunkNami : public FileChunk<StorageOperationData,Song>
{
public:
	FileChunkNami() :
		FileChunk<StorageOperationData,Song>("nami")
	{
		add_child(new FileChunkFormat);
		add_child(new FileChunkTag);
		add_child(new FileChunkLevelName);
		add_child(new FileChunkBar);
		add_child(new FileChunkSample);
		add_child(new FileChunkTrack);
		add_child(new FileChunkGlobalEffect);
	}
	virtual void create(){ me = parent->song; }
	virtual void read(File *f)
	{
		me->sample_rate = f->ReadInt();
	}
	virtual void write(File *f)
	{
		f->WriteInt(me->sample_rate);
	}
	virtual void write_subs()
	{
		write_sub("format", me);
		write_sub_array("tag", me->tags);
		write_sub("lvlname", me);
		write_sub_array("bar", me->bars);
		write_sub_parray("sample", me->samples);
		write_sub_parray("track", me->tracks);
		write_sub_parray("effect", me->fx);
	}
};

class ChunkedFileFormatNami : public FileChunk<void,StorageOperationData>
{
public:
	ChunkedFileFormatNami() :
		FileChunk<void,StorageOperationData>("")
	{
		od = NULL;
		add_child(new FileChunkNami);
		set_root(this);
	}
	virtual void read(File *f){}
	virtual void write(File *f){}
	StorageOperationData *od;

	bool read_file(const string &filename, StorageOperationData *_od)
	{
		od = _od;
		File *f = FileOpen(filename);
		f->SetBinaryMode(true);
		context = new FileChunkBasic::Context(f);
		context->push(Context::Layer(name, 0, f->GetSize()));

		set(od);
		read_contents();
		delete(context);

		delete(f);

		return true;
	}
	bool write_file(const string &filename, StorageOperationData *_od)
	{
		od = _od;
		File *f = FileCreate(filename);
		f->SetBinaryMode(true);
		context = new FileChunkBasic::Context(f);
		context->push(Context::Layer(name, 0, 0));

		set(od);
		write_contents();
		delete(context);

		delete(f);

		return true;
	}
	virtual void write_subs()
	{
		write_sub("nami", od->song);
	}
	virtual void on_notify()
	{
		od->set((float)context->f->GetPos() / (float)context->f->GetSize());
	}
	virtual void on_unhandled()
	{
		od->error("unhandled nami chunk: " + context->str());
	}
	virtual void on_error(const string &message)
	{
		od->error(message);
	}
	virtual void on_warn(const string &message)
	{
		od->warn(message);
	}
};


void FormatNami::saveSong(StorageOperationData *_od)
{
	od = _od;
	song = od->song;

	try{
		ChunkedFileFormatNami n;
		n.write_file(od->filename, od);
	}catch(string &s){
		od->error("saving nami: " + s);
	}
}


void check_empty_subs(Song *a)
{
	/*foreach(Track *t, a->track)
		foreachib(Track *s, t->sub, i)
			if (s->length <= 0){
				tsunami->log->Error("empty sub: " + s->name);
				t->sub.erase(i);
			}*/
}

void FormatNami::make_consistent(Song *a)
{
	foreach(Sample *s, a->samples){
		if (s->type == Track::TYPE_MIDI){
			if ((s->midi.samples == 0) and (s->midi.num > 0)){
				s->midi.samples = s->midi.back().range.end();
			}
		}
	}
}

void FormatNami::loadSong(StorageOperationData *_od)
{
	od = _od;

	// TODO?
	od->song->tags.clear();

	try{
		ChunkedFileFormatNami n;
		n.read_file(od->filename, od);
	}catch(string &s){
		od->error("loading nami: " + s);
	}

	// some post processing
	make_consistent(od->song);

	od->song->updateSelection(Range(0, 0));
}


