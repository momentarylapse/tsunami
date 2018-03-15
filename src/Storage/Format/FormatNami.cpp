/*
 * FormatNami.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatNami.h"
#include "../../Session.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Data/Curve.h"
#include "../../Rhythm/Bar.h"
#ifndef OS_WINDOWS
#include <FLAC/all.h>
#endif
#include <math.h>
#include "../../lib/xfile/chunked.h"



StorageOperationData *cur_op(FileChunkBasic *c);



FormatDescriptorNami::FormatDescriptorNami() :
	FormatDescriptor("Tsunami", "nami", FLAG_AUDIO | FLAG_MIDI | FLAG_FX | FLAG_MULTITRACK | FLAG_TAGS | FLAG_SUBS | FLAG_READ | FLAG_WRITE)
{}


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
		me->key = f->read_str();
		me->value = f->read_str();
	}
	virtual void write(File *f)
	{
		f->write_str(me->key);
		f->write_str(me->value);
	}
};

class FileChunkLayerName : public FileChunk<Song,Song>
{
public:
	FileChunkLayerName() : FileChunk<Song,Song>("lvlname"){}
	virtual void create(){ me = parent; }
	virtual void read(File *f)
	{
		int num = f->read_int();
		me->layers.clear();
		for (int i=0;i<num;i++)
			me->layers.add(new Song::Layer(f->read_str()));
	}
	virtual void write(File *f)
	{
		f->write_int(me->layers.num);
		for (auto l: me->layers)
			f->write_str(l->name);
	}
};

class FileChunkFormat : public FileChunk<Song,Song>
{
public:
	FileChunkFormat() : FileChunk<Song,Song>("format"){}
	virtual void create(){ me = parent; }
	virtual void read(File *f)
	{
		me->sample_rate = f->read_int();
		me->default_format = (SampleFormat)f->read_int();
		f->read_int(); // channels
		me->compression = f->read_int();
		f->read_int();
		f->read_int();
		f->read_int();
		f->read_int();
	}
	virtual void write(File *f)
	{
		f->write_int(me->sample_rate);
		f->write_int(me->default_format);
		f->write_int(2); // channels
		f->write_int(me->compression);
		f->write_int(0); // reserved
		f->write_int(0);
		f->write_int(0);
		f->write_int(0);
	}
};

class FileChunkEffect : public FileChunk<Track,Effect>
{
public:
	FileChunkEffect() : FileChunk<Track,Effect>("effect"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		me = CreateEffect(cur_op(this)->session, f->read_str());
		f->read_bool();
		f->read_int();
		f->read_int();
		string params = f->read_str();
		me->configFromString(params);
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_bool(false);
		f->write_int(0);
		f->write_int(0);
		f->write_str(me->configToString());
		f->write_str(me->enabled ? "" : "disabled");
	}
};

class FileChunkGlobalEffect : public FileChunk<Song,Effect>
{
public:
	FileChunkGlobalEffect() : FileChunk<Song,Effect>("effect"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		me = CreateEffect(cur_op(this)->session, f->read_str());
		f->read_bool();
		f->read_int();
		f->read_int();
		string params = f->read_str();
		me->configFromString(params);
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_bool(false);
		f->write_int(0);
		f->write_int(0);
		f->write_str(me->configToString());
		f->write_str(me->enabled ? "" : "disabled");
	}
};

class FileChunkCurve : public FileChunk<Song,Curve>
{
public:
	FileChunkCurve() : FileChunk<Song,Curve>("curve"){}
	virtual void create(){
		me = new Curve;
		parent->curves.add(me);
	}
	virtual void read(File *f)
	{
		f->read_int();
		me->name = f->read_str();
		int n = f->read_int();
		for (int i=0; i<n; i++){
			Curve::Target t;
			t.fromString(f->read_str(), parent);
			me->targets.add(t);
		}
		me->min = f->read_float();
		me->max = f->read_float();
		n = f->read_int();
		for (int i=0; i<n; i++){
			Curve::Point p;
			p.pos = f->read_int();
			p.value = f->read_float();
			me->points.add(p);
		}
		parent->notify(parent->MESSAGE_ADD_CURVE);
	}
	virtual void write(File *f)
	{
		f->write_int(0); // version
		f->write_str(me->name);
		f->write_int(me->targets.num);
		for (auto t: me->targets)
			f->write_str(t.str(parent));
		f->write_float(me->min);
		f->write_float(me->max);
		f->write_int(me->points.num);
		for (auto p: me->points){
			f->write_int(p.pos);
			f->write_float(p.value);
		}
	}
};

#ifdef OS_WINDOWS

string compress_buffer(AudioBuffer &b, Song *song, FileChunkBasic *p)
{
	return "";
}

void uncompress_buffer(AudioBuffer &b, string &data, FileChunkBasic *p)
{
}

#else

FLAC__StreamEncoderWriteStatus FlacCompressWriteCallback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
{
	string *data = (string*)client_data;
	for (unsigned int i=0; i<bytes; i++)
		data->add(buffer[i]);

	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

string compress_buffer(AudioBuffer &b, Song *song, FileChunkBasic *p)
{
	string data;


	bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;

	int channels = 2;
	int bits = min(format_get_bits(song->default_format), 24);
	float scale = pow(2.0f, bits-1);

	// allocate the encoder
	FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
	if (!encoder){
		p->error("flac: allocating encoder");
		return "";
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bits);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, DEFAULT_SAMPLE_RATE); // we don't really care...
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, b.length);

	// initialize encoder
	if (ok){
		init_status = FLAC__stream_encoder_init_stream(encoder, &FlacCompressWriteCallback, NULL, NULL, NULL, (void*)&data);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			p->error(string("flac: initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok){
		FLAC__int32 *flac_pcm = new FLAC__int32 [CHUNK_SAMPLES/*samples*/ * 2/*channels*/];

		int p0 = 0;
		size_t left = (size_t)b.length;
		while (ok and left){
			size_t need = (left > CHUNK_SAMPLES ? (size_t)CHUNK_SAMPLES : (size_t)left);
			{
				/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
				for (unsigned int i=0;i<need;i++){
					flac_pcm[i * 2 + 0] = (int)(b.c[0][p0 + i] * scale);
					flac_pcm[i * 2 + 1] = (int)(b.c[1][p0 + i] * scale);
				}
				/* feed samples to encoder */
				ok = FLAC__stream_encoder_process_interleaved(encoder, flac_pcm, need);
			}
			left -= need;
			p0 += CHUNK_SAMPLES;
		}

		delete[] flac_pcm;
	}

	ok &= FLAC__stream_encoder_finish(encoder);

	if (!ok){
		p->error("flac: encoding: FAILED");
		p->error(string("   state: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
	}

	FLAC__stream_encoder_delete(encoder);

	return data;
}

struct UncompressData
{
	AudioBuffer *buf;
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
	AudioBuffer *buf = d->buf;
	float scale = pow(2.0f, d->bits-1);
	int offset = d->sample_offset;
	int n = min((int)frame->header.blocksize, buf->length - offset);
	//msg_write(format("write %d  offset=%d  buf=%d", n, offset, buf->num));
	for (int i=0; i<n; i++)
		for (int j=0;j<d->channels;j++)
			buf->c[j][offset + i] = buffer[j][i] / scale;
	d->sample_offset += frame->header.blocksize;

	//flac_read_samples += frame->header.blocksize;
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderReadStatus FlacUncompressReadCallback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	UncompressData *d = (UncompressData*)client_data;
	*bytes = min((int)*bytes, d->data->num - d->byte_offset);
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

void uncompress_buffer(AudioBuffer &b, string &data, FileChunkBasic *p)
{
	bool ok = true;

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder)
		throw Exception("flac: decoder_new()");

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
		p->error(string("flac: initializing decoder: ") + FLAC__StreamDecoderInitStatusString[init_status]);
		ok = false;
	}

	if (ok){
		ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
		if (!ok){
			p->error("flac: decoding FAILED");
			p->error(string("   state: ") + FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
		}
	}
	FLAC__stream_decoder_delete(decoder);
}
#endif

class FileChunkBufferBox : public FileChunk<TrackLayer,AudioBuffer>
{
public:
	FileChunkBufferBox() : FileChunk<TrackLayer,AudioBuffer>("bufbox"){}
	virtual void create()
	{
		AudioBuffer dummy;
		parent->buffers.add(dummy);
		me = &parent->buffers.back();
	}
	virtual void read(File *f)
	{
		me->offset = f->read_int();
		int num = f->read_int();
		me->resize(num);
		int channels = f->read_int(); // channels (2)
		int bits = f->read_int(); // bit (16)

		string data;

		int bytes = context->layers.back().size - 16;
		data.resize(bytes);//num * (bits / 8) * channels);

		// read chunk'ed
		int offset = 0;
		for (int n=0; n<data.num / CHUNK_SIZE; n++){
			f->read_buffer(&data[offset], CHUNK_SIZE);
			notify();
			offset += CHUNK_SIZE;
		}
		f->read_buffer(&data[offset], data.num % CHUNK_SIZE);

		// insert

		Song *song = (Song*)root->base->get();
		if (song->compression > 0){
			//throw Exception("can't read compressed nami files yet");
			uncompress_buffer(*me, data, this);

		}else{
			me->import(data.data, channels, format_for_bits(bits), num);
		}
	}
	virtual void write(File *f)
	{
		Song *song = (Song*)root->base->get();

		int channels = 2;
		f->write_int(me->offset);
		f->write_int(me->length);
		f->write_int(channels);
		f->write_int(format_get_bits(song->default_format));

		string data;
		if (song->compression == 0){
			if (!me->exports(data, channels, song->default_format))
				warn(_("Amplitude too large, signal distorted."));
		}else{

			int uncompressed_size = me->length * channels * format_get_bits(song->default_format) / 8;
			data = compress_buffer(*me, song, this);
			msg_write(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
		}
		f->write_buffer(data.data, data.num);
	}
};

class FileChunkSampleBufferBox : public FileChunk<Sample,AudioBuffer>
{
public:
	FileChunkSampleBufferBox() : FileChunk<Sample,AudioBuffer>("bufbox"){}
	virtual void create()
	{
		me = &parent->buf;
	}
	virtual void read(File *f)
	{
		me->offset = f->read_int();
		int num = f->read_int();
		me->resize(num);
		int channels = f->read_int(); // channels (2)
		int bits = f->read_int(); // bit (16)

		string data;

		int bytes = context->layers.back().size - 16;
		data.resize(bytes);//num * (bits / 8) * channels);

		// read chunk'ed
		int offset = 0;
		for (int n=0; n<data.num / CHUNK_SIZE; n++){
			f->read_buffer(&data[offset], CHUNK_SIZE);
			notify();
			offset += CHUNK_SIZE;
		}
		f->read_buffer(&data[offset], data.num % CHUNK_SIZE);

		// insert

		Song *song = (Song*)root->base->get();
		if (song->compression > 0){
			//throw Exception("can't read compressed nami files yet");
			uncompress_buffer(*me, data, this);

		}else{
			me->import(data.data, channels, format_for_bits(bits), num);
		}
	}
	virtual void write(File *f)
	{
		Song *song = (Song*)root->base->get();

		int channels = 2;
		f->write_int(me->offset);
		f->write_int(me->length);
		f->write_int(channels);
		f->write_int(format_get_bits(song->default_format));

		string data;
		if (song->compression == 0){
			if (!me->exports(data, channels, song->default_format))
				warn(_("Amplitude too large, signal distorted."));
		}else{

			int uncompressed_size = me->length * channels * format_get_bits(song->default_format) / 8;
			data = compress_buffer(*me, song, this);
			msg_write(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
		}
		f->write_buffer(data.data, data.num);
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
		string name = f->read_str();
		int pos = f->read_int();
		int index = f->read_int();
		me = parent->addSampleRef(pos, parent->song->samples[index]);
		me->volume = f->read_float();
		me->muted = f->read_bool();
		f->read_int();
		f->read_int();
		f->read_int(); // reserved
		f->read_int();
	}
	virtual void write(File *f)
	{
		f->write_str(me->origin->name);
		f->write_int(me->pos);
		f->write_int(me->origin->get_index());
		f->write_float(me->volume);
		f->write_bool(me->muted);
		f->write_int(0);
		f->write_int(0);
		f->write_int(0); // reserved
		f->write_int(0);
	}
};

#if 0
void ReadChunkSub(ChunkStack *s, Track *t)
{
	string name = s->f->read_str();
	int pos = s->f->read_int();
	int length = s->f->read_int();
	SampleRef *r = __AddEmptySubTrack(t, Range(pos, length), name);
	r->volume = s->f->read_float();
	r->muted = s->f->read_bool();
	r->rep_num = s->f->read_int();
	r->rep_delay = s->f->read_int();
	s->f->read_int(); // reserved
	s->f->read_int();

	s->AddChunkHandler("bufbox", (chunk_reader*)&ReadChunkSampleBufferBox, &r->buf);
	tsunami->log->error("\"sub\" chunk is deprecated!");
}
#endif


class FileChunkMidiEvent : public FileChunk<MidiNoteBuffer,MidiEvent>
{
public:
	FileChunkMidiEvent() : FileChunk<MidiNoteBuffer,MidiEvent>("event"){}
	virtual void create()
	{
		//parent->add(MidiEvent());
		//me = &parent->back();
	}
	virtual void read(File *f)
	{
		MidiEvent e;
		e.pos = f->read_int();
		e.pitch = f->read_int();
		e.volume = f->read_float();
		f->read_int(); // reserved

		int unended = -1;
		foreachi(MidiNote *n, *parent, i)
			if ((n->pitch == e.pitch) and (n->range.length == -1))
				unended = i;

		if ((unended >= 0) and (e.volume == 0)){
			(*parent)[unended]->range.set_end(e.pos);
		}else if ((unended < 0) and (e.volume > 0)){
			parent->add(new MidiNote(Range(e.pos, -1), e.pitch, e.volume));
		}else if (unended >= 0){
			error("nami/midi: starting new note without ending old one");
		}else{
			error("nami/midi: no note to end");
		}
	}
	virtual void write(File *f)
	{
		f->write_int(me->pos);
		f->write_int(me->pitch);
		f->write_float(me->volume);
		f->write_int(0); // reserved
	}
};

class FileChunkMidiEffect : public FileChunk<MidiNoteBuffer,MidiEffect>
{
public:
	FileChunkMidiEffect() : FileChunk<MidiNoteBuffer,MidiEffect>("effect"){}
	virtual void create()
	{}
	virtual void read(File *f)
	{
		me = CreateMidiEffect(cur_op(this)->session, f->read_str());
		me->only_on_selection = f->read_bool();
		me->range.offset = f->read_int();
		me->range.length = f->read_int();
		string params = f->read_str();
		me->configFromString(params);
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_bool(me->only_on_selection);
		f->write_int(me->range.offset);
		f->write_int(me->range.length);
		f->write_str(me->configToString());
		f->write_str(me->enabled ? "" : "disabled");
	}
};


class FileChunkMidiNote : public FileChunk<MidiNoteBuffer,MidiNote>
{
public:
	FileChunkMidiNote() : FileChunk<MidiNoteBuffer,MidiNote>("note"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		MidiNote *n = new MidiNote;
		n->range.offset = f->read_int();
		n->range.length = f->read_int();
		n->pitch = f->read_int();
		n->volume = f->read_float();
		f->read_int(); // reserved
		parent->add(n);
	}
	virtual void write(File *f)
	{
		f->write_int(me->range.offset);
		f->write_int(me->range.length);
		f->write_int(me->pitch);
		f->write_float(me->volume);
		f->write_int(0); // reserved
	}
};

class FileChunkSampleMidiData : public FileChunk<Sample,MidiNoteBuffer>
{
public:
	FileChunkSampleMidiData() : FileChunk<Sample,MidiNoteBuffer>("midi")
	{
		add_child(new FileChunkMidiEvent);
		add_child(new FileChunkMidiNote);
		add_child(new FileChunkMidiEffect);
	}
	virtual void create(){ me = &parent->midi; }
	virtual void read(File *f)
	{
		f->read_str();
		f->read_str();
		f->read_str();
		int version = f->read_int();
		if (version < 1)
			return;
		int num = f->read_int();
		int meta = f->read_int();
		for (int i=0; i<num; i++){
			MidiNote *n = new MidiNote;
			n->range.offset = f->read_int();
			n->range.length = f->read_int();
			n->pitch = f->read_int();
			n->volume = f->read_float();
			if (meta & 1)
				n->stringno = f->read_int();
			if (meta & 2)
				n->clef_position = f->read_int();
			me->add(n);
		}
	}
	virtual void write(File *f)
	{
		f->write_str("");
		f->write_str("");
		f->write_str("");
		f->write_int(1); // version

		f->write_int(me->num);
		f->write_int(3); // stringno + clef_position
		for (MidiNote *n : *me){
			f->write_int(n->range.offset);
			f->write_int(n->range.length);
			f->write_int(n->pitch);
			f->write_float(n->volume);
			f->write_int(n->stringno);
			f->write_int(n->clef_position);
		}
		f->write_int(0); // reserved
	}
	virtual void write_subs()
	{
		write_sub_parray("effect", me->fx);
	}
};

class FileChunkTrackMidiData : public FileChunk<Track,MidiNoteBuffer>
{
public:
	FileChunkTrackMidiData() : FileChunk<Track,MidiNoteBuffer>("midi")
	{
		add_child(new FileChunkMidiEvent);
		add_child(new FileChunkMidiNote);
		add_child(new FileChunkMidiEffect);
	}
	virtual void create(){ me = &parent->midi; }
	virtual void read(File *f)
	{
		f->read_str();
		f->read_str();
		f->read_str();
		int version = f->read_int();
		if (version < 1)
			return;
		int num = f->read_int();
		int meta = f->read_int();
		for (int i=0; i<num; i++){
			MidiNote *n = new MidiNote;
			n->range.offset = f->read_int();
			n->range.length = f->read_int();
			n->pitch = f->read_int();
			n->volume = f->read_float();
			if (meta & 1)
				n->stringno = f->read_int();
			if (meta & 2)
				n->clef_position = f->read_int();
			me->add(n);
		}
		f->read_int(); // reserved
	}
	virtual void write(File *f)
	{
		f->write_str("");
		f->write_str("");
		f->write_str("");
		f->write_int(1); // version

		f->write_int(me->num);
		f->write_int(3); // stringno + clef_position
		for (MidiNote *n : *me){
			f->write_int(n->range.offset);
			f->write_int(n->range.length);
			f->write_int(n->pitch);
			f->write_float(n->volume);
			f->write_int(n->stringno);
			f->write_int(n->clef_position);
		}
		f->write_int(0); // reserved
	}
	virtual void write_subs()
	{
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
		me->set_owner(parent);
		parent->samples.add(me);
	}
	virtual void read(File *f)
	{
		me->name = f->read_str();
		me->volume = f->read_float();
		me->offset = f->read_int();
		me->type = f->read_int();
		f->read_int(); // reserved
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_float(me->volume);
		f->write_int(me->offset);
		f->write_int(me->type);
		f->write_int(0); // reserved
	}
	virtual void write_subs()
	{
		if (me->type == Track::TYPE_AUDIO)
			write_sub("bufbox", &me->buf);
		else if (me->type == Track::TYPE_MIDI)
			write_sub("midi", &me->midi);
	}
};

class FileChunkTrackLayer : public FileChunk<Track,TrackLayer>
{
public:
	int n;
	FileChunkTrackLayer() : FileChunk<Track,TrackLayer>("level")
	{
		n = 0;
		add_child(new FileChunkBufferBox);
	}
	virtual void create()
	{
		//me = &parent->layers[n];
	}
	virtual void read(File *f)
	{
		n = f->read_int();
		me = &parent->layers[n];
	}
	virtual void write(File *f)
	{
		f->write_int(parent->layers.index(me));
	}
	virtual void write_subs()
	{
		write_sub_array("bufbox", me->buffers);
	}
};

class FileChunkSynthesizerTuning : public FileChunk<Synthesizer,Synthesizer::Tuning>
{
public:
	FileChunkSynthesizerTuning() : FileChunk<Synthesizer,Synthesizer::Tuning>("tuning"){}
	virtual void create(){ me = &parent->tuning; }
	virtual void read(File *f)
	{
		for (int i=0; i<MAX_PITCH; i++)
			me->freq[i] = f->read_float();
		parent->update_delta_phi();
	}
	virtual void write(File *f)
	{
		for (int i=0; i<MAX_PITCH; i++)
			f->write_float(me->freq[i]);
	}
};

class FileChunkSynthesizer : public FileChunk<Track,Synthesizer>
{
public:
	FileChunkSynthesizer() : FileChunk<Track,Synthesizer>("synth")
	{
		add_child(new FileChunkSynthesizerTuning);
	}
	virtual void create(){}
	virtual void read(File *f)
	{
		Session *session = cur_op(this)->session;
		me = session->plugin_manager->CreateSynthesizer(session, f->read_str());
		me->configFromString(f->read_str());
		f->read_str();
		f->read_int();

		delete(parent->synth);
		parent->synth = me;
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_str(me->configToString());
		f->write_str("");
		f->write_int(0); // reserved
	}
	virtual void write_subs()
	{
		if (!me->tuning.is_default())
			write_sub("tuning", &me->tuning);
	}
};

class FileChunkBar : public FileChunk<Song,Bar>
{
public:
	FileChunkBar() : FileChunk<Song,Bar>("bar"){}
	virtual void create(){ me = NULL; }
	virtual void read(File *f)
	{
		int type = f->read_int();
		int length = f->read_int();
		int num_beats = f->read_int();
		if (type == BarPattern::TYPE_PAUSE)
			num_beats = 0;
		int count = f->read_int();
		int sub_beats = f->read_int();
		if (sub_beats <= 0)
			sub_beats = 1;
		for (int i=0; i<count; i++)
			parent->bars.add(new Bar(length, num_beats, sub_beats));
	}
	virtual void write(File *f)
	{
		if (me->is_pause())
			f->write_int(BarPattern::TYPE_PAUSE);
		else
			f->write_int(BarPattern::TYPE_BAR);
		f->write_int(me->length);
		f->write_int(me->num_beats);
		f->write_int(1);
		f->write_int(me->num_sub_beats);
	}
};

/*class FileChunkTrackBar : public FileChunk<Track,Bar>
{
public:
	FileChunkTrackBar() : FileChunk<Track,Bar>("bar"){}
	virtual void create(){ me = NULL; }
	virtual void read(File *f)
	{
		int type = f->read_int();
		int length = f->read_int();
		int num_beats = f->read_int();
		if (type == BarPattern::TYPE_PAUSE)
			num_beats = 0;
		int count = f->read_int();
		int sub_beats = f->read_int();
		if (sub_beats <= 0)
			sub_beats = 1;
		for (int i=0; i<count; i++)
			parent->song->bars.add(new Bar(length, num_beats, sub_beats));
	}
	virtual void write(File *f)
	{
		root->on_error("deprecated... TrackBar.write");
		if (me->is_pause())
			f->write_int(BarPattern::TYPE_PAUSE);
		else
			f->write_int(BarPattern::TYPE_BAR);
		f->write_int(me->length);
		f->write_int(me->num_beats);
		f->write_int(1);
		f->write_int(0); // reserved
	}
};*/

class FileChunkMarker : public FileChunk<Track,TrackMarker>
{
public:
	FileChunkMarker() : FileChunk<Track,TrackMarker>("marker"){}
	virtual void create()
	{
		me = new TrackMarker();
		parent->markers.add(me);
	}
	virtual void read(File *f)
	{
		me->range.offset = f->read_int();
		me->text = f->read_str();
		int version = f->read_int();
		if (version > 0){
			me->range.length = f->read_int();
			int nfx = f->read_int();
		}
	}
	virtual void write(File *f)
	{
		f->write_int(me->range.offset);
		f->write_str(me->text);
		f->write_int(1);
		f->write_int(me->range.length);
		f->write_int(0);//me->fx.num);
	}
};

class FileChunkTuning : public FileChunk<Track,Instrument>
{
public:
	FileChunkTuning() : FileChunk<Track,Instrument>("tuning"){}
	virtual void create()
	{
		me = &parent->instrument;
	}
	virtual void read(File *f)
	{
		me->string_pitch.resize(f->read_int());
		for (int i=0; i<me->string_pitch.num; i++)
			me->string_pitch[i] = f->read_int();
	}
	virtual void write(File *f)
	{
		f->write_int(me->string_pitch.num);
		for (int i=0; i<me->string_pitch.num; i++)
			f->write_int(me->string_pitch[i]);
	}
};

class FileChunkTrack : public FileChunk<Song,Track>
{
public:
	FileChunkTrack() : FileChunk<Song,Track>("track")
	{
		add_child(new FileChunkTuning);
		add_child(new FileChunkTrackLayer);
		add_child(new FileChunkSynthesizer);
		add_child(new FileChunkEffect);
		add_child(new FileChunkTrackMidiData);
		//add_child(new FileChunkTrackBar);
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
		me->name = f->read_str();
		me->volume = f->read_float();
		me->muted = f->read_bool();
		me->type = f->read_int();
		me->panning = f->read_float();
		me->instrument = Instrument(f->read_int());
		f->read_int(); // reserved

		notify();
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_float(me->volume);
		f->write_bool(me->muted);
		f->write_int(me->type);
		f->write_float(me->panning);
		f->write_int(me->instrument.type);
		f->write_int(0); // reserved

		notify();
	}
	virtual void write_subs()
	{
		if (!me->instrument.has_default_tuning())
			write_sub("tuning", &me->instrument);
		write_sub_array("level", me->layers);
		write_sub_parray("samref", me->samples);
		write_sub_parray("effect", me->fx);
		write_sub_parray("marker", me->markers);
		if ((me->type == me->TYPE_TIME) or (me->type == me->TYPE_MIDI))
			if (!me->synth->isDefault())
				write_sub("synth", me->synth);
		if (me->midi.num > 0)
			write_sub("midi", &me->midi);
	}
};

class FileChunkNami : public FileChunk<Song,Song>
{
public:
	FileChunkNami() :
		FileChunk<Song,Song>("nami")
	{
		add_child(new FileChunkFormat);
		add_child(new FileChunkTag);
		add_child(new FileChunkLayerName);
		add_child(new FileChunkBar);
		add_child(new FileChunkSample);
		add_child(new FileChunkTrack);
		add_child(new FileChunkGlobalEffect);
		add_child(new FileChunkCurve);
	}
	virtual void create(){ me = parent; }
	virtual void read(File *f)
	{
		me->sample_rate = f->read_int();
	}
	virtual void write(File *f)
	{
		f->write_int(me->sample_rate);
	}
	virtual void write_subs()
	{
		write_sub("format", me);
		write_sub_array("tag", me->tags);
		write_sub("lvlname", me);
		write_sub_parray("bar", me->bars);
		write_sub_parray("sample", me->samples);
		write_sub_parray("track", me->tracks);
		write_sub_parray("effect", me->fx);
		write_sub_parray("curve", me->curves);
	}
};

class ChunkedFileFormatNami : public ChunkedFileParser
{
public:
	ChunkedFileFormatNami() :
		ChunkedFileParser(8)
	{
		od = NULL;
		base = new FileChunkNami;
	}
	StorageOperationData *od;

	bool read_file(StorageOperationData *_od)
	{
		od = _od;
		return this->read(od->filename, od->song);
	}
	bool write_file(StorageOperationData *_od)
	{
		od = _od;
		return this->write(od->filename, od->song);
	}
	virtual void on_notify()
	{
		od->set((float)context.f->get_pos() / (float)context.f->get_size());
	}
	virtual void on_unhandled()
	{
		od->error("unhandled nami chunk: " + context.str());
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

StorageOperationData *cur_op(FileChunkBasic *c)
{
	return ((ChunkedFileFormatNami*)c->root)->od;
}


void FormatNami::saveSong(StorageOperationData *_od)
{
	od = _od;
	song = od->song;

	try{
		ChunkedFileFormatNami n;
		n.write_file(od);
	}catch(Exception &s){
		od->error("saving nami: " + s.message());
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
	for (Sample *s : a->samples){
		if (s->type == Track::TYPE_MIDI){
			if ((s->midi.samples == 0) and (s->midi.num > 0)){
				s->midi.samples = s->midi.back()->range.end();
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
		n.read_file(od);
	}catch(Exception &e){
		od->error("loading nami: " + e.message());
	}

	// some post processing
	make_consistent(od->song);

	//od->song->updateSelection(Range(0, 0));
}


