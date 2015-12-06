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
		chunk_pos = 0;
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
	void add(const string &name, void *p, bool is_array = false)
	{
		foreach(FileChunkBasic *c, children)
			if (c->name == name)
				subs.add(Sub(c, p, is_array));
	}

	virtual void set(void *t){};
	virtual void set_parent(void *t){};
	virtual void *get(){ return NULL; }

	struct Context
	{
		Context(File *_f)
		{
			f = _f;
		}
		void pop()
		{
			names.pop();
			pos0.pop();
			sizes.pop();
		}
		void push(const string &name, int pos, int size)
		{
			names.add(name);
			pos0.add(pos);
			sizes.add(size);
		}
		int end()
		{
			if (names.num > 0)
				return pos0.back() + sizes.back();
			return 2000000000;
		}
		string str()
		{
			return implode(names, "/");
		}
		File *f;
		Array<string> names;
		Array<int> pos0, sizes;
	};
	Context *context;

	struct Sub
	{
		FileChunkBasic *c;
		void *p;
		bool is_array;

		Sub(){}
		Sub(FileChunkBasic *_c, void *_p, bool _is_array)
		{
			c = _c;
			p = _p;
			is_array = _is_array;
		}
	};
	Array<FileChunkBasic*> children;
	FileChunkBasic *root;
	Array<Sub> subs;

	void write_subs(Context *context)
	{
		foreach(Sub &s, subs){
			if (s.is_array){
				DynamicArray *a = (DynamicArray*)s.p;
				for (int i=0; i<a->num; i++){
					s.c->set((char*)a->data + a->element_size * i);
					s.c->write_complete(context);
				}
			}else{
				s.c->set(s.p);
				s.c->write_complete(context);
			}
		}
	}

	int chunk_pos;
	void write_begin_chunk(File *f)
	{
		string s = name + "        ";
		f->WriteBuffer(s.data, 8);
		f->WriteInt(0); // temporary
		chunk_pos = f->GetPos();
	}

	void write_end_chunk(File *f)
	{
		int pos0 = f->GetPos();
		f->SetPos(chunk_pos - 4, true);
		f->WriteInt(pos0 - chunk_pos);
		f->SetPos(pos0, true);
	}
	void write_complete(Context *context)
	{
		File *f = context->f;
		write_begin_chunk(f);
		write(f);
		write_subs(context);
		write_end_chunk(f);
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

			context->push(cname, pos0, size);



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

		int bytes = context->sizes.back() - 16;
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
	{}
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
		parent->samples.add(new Sample(Track::TYPE_AUDIO));
		me = parent->samples.back();
		me->owner = parent;
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
		context->push(name, 0, f->GetSize());

		set(od);
		read_contents();
		delete(context);

		delete(f);

		return true;
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
	f->WriteInt(format_get_bits(song->default_format));

	string data;
	if (song->compression == 0){
		if (!b->exports(data, channels, song->default_format))
			od->warn(_("Amplitude zu gro&s, Signal &ubersteuert."));
	}else{

		int uncompressed_size = b->num * channels * format_get_bits(song->default_format) / 8;
		data = compress_buffer(*b, od);
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
	f->WriteInt(1);
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

void FormatNami::WriteMidi(MidiNoteData &m)
{
	BeginChunk("midi");

	f->WriteStr("");
	f->WriteStr("");
	f->WriteStr("");
	f->WriteInt(0); // reserved

	MidiRawData midi = midi_notes_to_events(m);

	foreach(MidiEvent &e, midi)
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

void FormatNami::WriteTuning(Array<int> &tuning)
{
	BeginChunk("tuning");
	f->WriteInt(tuning.num);

	for (int i=0; i<tuning.num; i++)
		f->WriteInt(tuning[i]);

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
	f->WriteInt(t->instrument.type);
	f->WriteInt(0); // reserved

	if (!t->instrument.is_default_tuning(t->tuning))
		WriteTuning(t->tuning);

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

	f->WriteInt(song->level_names.num);
	foreach(string &l, song->level_names)
		f->WriteStr(l);

	EndChunk();
}

void FormatNami::WriteFormat()
{
	BeginChunk("format");

	f->WriteInt(song->sample_rate);
	f->WriteInt(song->default_format);
	f->WriteInt(2); // channels
	f->WriteInt(song->compression);
	f->WriteInt(0); // reserved
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteInt(0);

	EndChunk();
}

void FormatNami::saveSong(StorageOperationData *_od)
{
	od = _od;
	song = od->song;

//	int length = a->GetLength();
//	int min = a->GetMin();
	f = FileCreate(od->filename);
	f->SetBinaryMode(true);

	BeginChunk("nami");

	f->WriteInt(song->sample_rate);
	WriteFormat();

	foreach(Tag &tag, song->tags)
		WriteTag(&tag);

	WriteLevelName();

	foreach(BarPattern &b, song->bars)
		WriteBar(b);

	foreach(Sample *sample, song->samples)
		WriteSample(sample);

	foreachi(Track *track, song->tracks, i){
		WriteTrack(track);
		od->set(((float)i + 0.5f) / (float)song->tracks.num);
	}

	foreach(Effect *effect, song->fx)
		WriteEffect(effect);

	EndChunk();

	FileClose(f);
}



#if 0
void ReadCompressed(File *f, char *data, int size)
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
	t->song->addSample(name, buf);
	return t->addSample(r.start(), t->song->samples.num - 1);
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
	File *f;
	Song *s;
	StorageOperationData *od;


	void AddChunkHandler(const string &tag, chunk_reader *reader, void *data)
	{
		ChunkHandler h;
		h.tag = tag;
		h.reader = reader;
		h.data = data;
		chunk_data.back().handler.add(h);
	}



	void ReadChunk(File *f)
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
			od->error("unknown nami chunk: " + cname + " (within " + chunk_data[chunk_data.num - 2].tag + ")");


		f->SetPos(chunk_data.back().end(), true);
		chunk_data.pop();
	}
};

void ReadChunkFormat(ChunkStack *s, Song *a)
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

void ReadChunkLevelName(ChunkStack *s, Song *a)
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
		s->od->set((float)s->f->GetPos() / (float)s->f->GetSize());
		offset += CHUNK_SIZE;
	}
	s->f->ReadBuffer(&data[offset], data.num % CHUNK_SIZE);

	// insert

	if (s->s->compression > 0){
		//throw string("can't read compressed nami files yet");
		uncompress_buffer(*b, data, s->od);

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
	s->od->error("\"sub\" chunk is deprecated!");
}

void ReadChunkBar(ChunkStack *s, Array<BarPattern> *bar)
{
	BarPattern b;
	b.type = s->f->ReadInt();
	b.length = s->f->ReadInt();
	b.num_beats = s->f->ReadInt();
	if (b.type == BarPattern::TYPE_PAUSE)
		b.num_beats = 0;
	int count = s->f->ReadInt();
	s->f->ReadInt(); // reserved
	for (int i=0; i<count; i++)
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

void ReadChunkMidiNote(ChunkStack *s, MidiNoteData *m)
{
	MidiNote n;
	n.range.offset = s->f->ReadInt();
	n.range.num = s->f->ReadInt();
	n.pitch = s->f->ReadInt();
	n.volume = s->f->ReadFloat();
	s->f->ReadInt(); // reserved
	m->add(n);
}

void ReadChunkMidiEvent(ChunkStack *s, MidiNoteData *m)
{
	MidiEvent e;
	e.pos = s->f->ReadInt();
	e.pitch = s->f->ReadInt();
	e.volume = s->f->ReadFloat();
	s->f->ReadInt(); // reserved
	//m->add(e);
	s->od->error("ReadChunkMidiEvent");
}

void ReadChunkMidiEffect(ChunkStack *s, MidiNoteData *m)
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

void ReadChunkMidiData(ChunkStack *s, MidiNoteData *midi)
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

void ReadChunkSample(ChunkStack *s, Song *a)
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

void ReadChunkTrack(ChunkStack *s, Song *a)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO);
	t->name = s->f->ReadStr();
	t->volume = s->f->ReadFloat();
	t->muted = s->f->ReadBool();
	t->type = s->f->ReadInt();
	t->panning = s->f->ReadFloat();
	t->instrument = Instrument(s->f->ReadInt());
	s->f->ReadInt(); // reserved
	s->od->set((float)s->f->GetPos() / (float)s->f->GetSize());

	t->tuning = t->instrument.default_tuning();

	s->AddChunkHandler("level", (chunk_reader*)&ReadChunkTrackLevel, t);
	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkBufferBox, &t->levels[0]);
	s->AddChunkHandler("samref", (chunk_reader*)&ReadChunkSampleRef, t);
	s->AddChunkHandler("sub", (chunk_reader*)&ReadChunkSub, t);
	s->AddChunkHandler("effect", (chunk_reader*)&ReadChunkEffect, &t->fx);
	s->AddChunkHandler("bar", (chunk_reader*)&ReadChunkBar, &a->bars);
	s->AddChunkHandler("midi", (chunk_reader*)&ReadChunkMidiData, &t->midi);
	s->AddChunkHandler("synth", (chunk_reader*)&ReadChunkSynth, t);
	s->AddChunkHandler("marker", (chunk_reader*)&ReadChunkMarker, &t->markers);
}

void ReadChunkNami(ChunkStack *s, Song *a)
{
	a->sample_rate = s->f->ReadInt();

	s->AddChunkHandler("format", (chunk_reader*)&ReadChunkFormat, a);
	s->AddChunkHandler("tag", (chunk_reader*)&ReadChunkTag, &a->tags);
	s->AddChunkHandler("effect", (chunk_reader*)&ReadChunkEffect, &a->fx);
	s->AddChunkHandler("lvlname", (chunk_reader*)&ReadChunkLevelName, a);
	s->AddChunkHandler("sample", (chunk_reader*)&ReadChunkSample, a);
	s->AddChunkHandler("track", (chunk_reader*)&ReadChunkTrack, a);
}


void load_nami_file_new(StorageOperationData *od, File *f)
{
	Song *old = tsunami->song;
	tsunami->song = od->song;

	ChunkStack s;
	s.f = f;
	s.s = od->song;
	s.od = od;
	s.chunk_data.add(ChunkLevelData("-top level-", 0, f->GetSize()));
	s.AddChunkHandler("nami", (chunk_reader*)&ReadChunkNami, od->song);

	s.ReadChunk(f);

	tsunami->song = old;
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

#if 0
	File *f = FileOpen(od->filename);
	f->SetBinaryMode(true);

	try{
		load_nami_file_new(od, f);
	}catch(string &s){
		tsunami->log->error("loading nami: " + s);
	}

	FileClose(f);
#else

	try{
		ChunkedFileFormatNami n;
		n.read_file(od->filename, od);
	}catch(string &s){
		od->error("loading nami: " + s);
	}
#endif

	// some post processing
	make_consistent(od->song);

	od->song->updateSelection(Range(0, 0));
}


