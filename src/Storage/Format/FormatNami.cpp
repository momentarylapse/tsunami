/*
 * FormatNami.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "FormatNami.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"
#include "../../lib/file/file.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/Song.h"
#include "../../Data/base.h"
#include "../../Data/Curve.h"
#include "../../Data/CrossFade.h"
#include "../../Data/SampleRef.h"
#include "../../Data/Sample.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Module/Audio/AudioEffect.h"
#include "../../Module/Midi/MidiEffect.h"
#include "../../Module/Synth/Synthesizer.h"
#if HAS_LIB_FLAC
#include <FLAC/all.h>
#endif
#include <math.h>
#include "../../lib/xfile/chunked.h"



StorageOperationData *cur_op(FileChunkBasic *c);



FormatDescriptorNami::FormatDescriptorNami() :
	FormatDescriptor("Tsunami", "nami", Flag::AUDIO | Flag::MIDI | Flag::FX | Flag::MULTITRACK | Flag::TAGS | Flag::SAMPLES | Flag::READ | Flag::WRITE)
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
		for (int i=0;i<num;i++)
			f->read_str();
		/*me->layers.clear();
		for (int i=0;i<num;i++)
			me->layers.add(new Song::Layer(f->read_str()));*/
	}
	virtual void write(File *f)
	{
		/*f->write_int(me->layers.num);
		for (auto l: me->layers)
			f->write_str(l->name);*/
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
		f->write_int((int)me->default_format);
		f->write_int(2); // channels
		f->write_int(me->compression);
		f->write_int(0); // reserved
		f->write_int(0);
		f->write_int(0);
		f->write_int(0);
	}
};

class FileChunkEffect : public FileChunk<Track,AudioEffect>
{
public:
	FileChunkEffect() : FileChunk<Track,AudioEffect>("effect"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		me = CreateAudioEffect(cur_op(this)->session, f->read_str());
		f->read_bool();
		f->read_int();
		f->read_int();
		string params = f->read_str();
		me->config_from_string(params);
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->write_str(me->module_subtype);
		f->write_bool(false);
		f->write_int(0);
		f->write_int(0);
		f->write_str(me->config_to_string());
		f->write_str(me->enabled ? "" : "disabled");
	}
};

class FileChunkGlobalEffect : public FileChunk<Song,AudioEffect>
{
public:
	FileChunkGlobalEffect() : FileChunk<Song,AudioEffect>("effect"){}
	virtual void create(){}
	virtual void read(File *f)
	{
		me = CreateAudioEffect(cur_op(this)->session, f->read_str());
		f->read_bool();
		f->read_int();
		f->read_int();
		string params = f->read_str();
		me->config_from_string(params);
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->fx.add(me);
	}
	virtual void write(File *f)
	{
		f->write_str(me->module_subtype);
		f->write_bool(false);
		f->write_int(0);
		f->write_int(0);
		f->write_str(me->config_to_string());
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

#if HAS_LIB_FLAC == 0

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

// TODO: allow mono...
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
		init_status = FLAC__stream_encoder_init_stream(encoder, &FlacCompressWriteCallback, nullptr, nullptr, nullptr, (void*)&data);
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
							nullptr, nullptr, nullptr, nullptr,
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
		me->channels = f->read_int();
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
			me->import(data.data, me->channels, format_for_bits(bits), num);
		}
	}
	virtual void write(File *f)
	{
		Song *song = (Song*)root->base->get();

		f->write_int(me->offset);
		f->write_int(me->length);
		f->write_int(me->channels);
		f->write_int(format_get_bits(song->default_format));

		string data;
		if (song->compression == 0){
			if (!me->exports(data, me->channels, song->default_format))
				warn(_("Amplitude too large, signal distorted."));
		}else{

			int uncompressed_size = me->length * me->channels * format_get_bits(song->default_format) / 8;
			data = compress_buffer(*me, song, this);
			msg_write(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
		}
		f->write_buffer(data);
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
		int channels = f->read_int(); // channels (2)
		int bits = f->read_int(); // bit (16)

		me->clear_x(channels);
		me->resize(num);

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

		f->write_int(me->offset);
		f->write_int(me->length);
		f->write_int(me->channels);
		f->write_int(format_get_bits(song->default_format));

		string data;
		if (song->compression == 0){
			if (!me->exports(data, me->channels, song->default_format))
				warn(_("Amplitude too large, signal distorted."));
		}else{

			int uncompressed_size = me->length * me->channels * format_get_bits(song->default_format) / 8;
			data = compress_buffer(*me, song, this);
			msg_write(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
		}
		f->write_buffer(data);
	}
};

class FileChunkSampleRef : public FileChunk<TrackLayer,SampleRef>
{
public:
	FileChunkSampleRef() : FileChunk<TrackLayer,SampleRef>("samref"){}
	virtual void create()
	{}
	virtual void read(File *f)
	{
		string name = f->read_str();
		int pos = f->read_int();
		int index = f->read_int();
		me = parent->add_sample_ref(pos, parent->track->song->samples[index]);
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

// deprecated
class _FileChunkTrackSampleRef : public FileChunk<Track,SampleRef>
{
public:
	_FileChunkTrackSampleRef() : FileChunk<Track,SampleRef>("samref"){}
	virtual void create()
	{}
	virtual void read(File *f)
	{
		string name = f->read_str();
		int pos = f->read_int();
		int index = f->read_int();
		me = parent->layers[0]->add_sample_ref(pos, parent->song->samples[index]);
		me->volume = f->read_float();
		me->muted = f->read_bool();
		f->read_int();
		f->read_int();
		f->read_int(); // reserved
		f->read_int();
	}
	virtual void write(File *f)
	{
	}
};


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
		me->config_from_string(params);
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		//parent->fx.add(me);
		msg_write("todo: nami midi fx");
	}
	virtual void write(File *f)
	{
		f->write_str(me->module_subtype);
		f->write_bool(me->only_on_selection);
		f->write_int(me->range.offset);
		f->write_int(me->range.length);
		f->write_str(me->config_to_string());
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
		//add_child(new FileChunkMidiEffect);
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
		//write_sub_parray("effect", me->fx);
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
	virtual void create(){ me = &parent->layers[0]->midi; }
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
		write_sub_parray("effect", parent->midi_fx);
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
		me = new Sample(SignalType::AUDIO);
		me->set_owner(parent);
		parent->samples.add(me);
	}
	virtual void read(File *f)
	{
		me->name = f->read_str();
		me->volume = f->read_float();
		me->offset = f->read_int();
		me->type = (SignalType)f->read_int();
		f->read_int(); // reserved
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_float(me->volume);
		f->write_int(me->offset);
		f->write_int((int)me->type);
		f->write_int(0); // reserved
	}
	virtual void write_subs()
	{
		if (me->type == SignalType::AUDIO)
			write_sub("bufbox", &me->buf);
		else if (me->type == SignalType::MIDI)
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
		add_child(new FileChunkSampleRef);
	}
	virtual void create()
	{
	}
	virtual void read(File *f)
	{
		n = f->read_int();
		if (n > 0){
			parent->layers.add(new TrackLayer(parent));
		}
		me = parent->layers.back();
	}
	virtual void write(File *f)
	{
		int n = 0;
		foreachi(TrackLayer *l, parent->layers, i)
			if (l == me)
				n = i;
		f->write_int(n);
	}
	virtual void write_subs()
	{
		write_sub_array("bufbox", me->buffers);
		write_sub_parray("samref", me->samples);
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
		me = CreateSynthesizer(session, f->read_str());
		me->config_from_string(f->read_str());
		f->read_str();
		f->read_int();

		parent->set_synthesizer(me);
	}
	virtual void write(File *f)
	{
		f->write_str(me->module_subtype);
		f->write_str(me->config_to_string());
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
	virtual void create(){ me = nullptr; }
	virtual void read(File *f)
	{
		int type = f->read_int();
		int length = f->read_int();
		int num_beats = f->read_int();
		if (type == BarPattern::Type::PAUSE)
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
			f->write_int(BarPattern::Type::PAUSE);
		else
			f->write_int(BarPattern::Type::BAR);
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
		if (type == BarPattern::Type::PAUSE)
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
			f->write_int(BarPattern::Type::PAUSE);
		else
			f->write_int(BarPattern::Type::BAR);
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

class FileChunkFade: public FileChunk<Track,CrossFade>
{
public:
	FileChunkFade() : FileChunk<Track,CrossFade>("fade"){}
	virtual void create()
	{
	}
	virtual void read(File *f)
	{
		CrossFade ff;
		ff.position = f->read_int();
		ff.target = f->read_int();
		ff.samples = f->read_int();
		f->read_int();
		f->read_int();
		parent->fades.add(ff);
	}
	virtual void write(File *f)
	{
		f->write_int(me->position);
		f->write_int(me->target);
		f->write_int(me->samples);
		f->write_int(0);
		f->write_int(0);
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
		add_child(new _FileChunkTrackSampleRef); // deprecated
			//s->AddChunkHandler("sub", (chunk_reader*)&ReadChunkSub, t);
		add_child(new FileChunkFade);
	}
	virtual void create()
	{
		//me = parent->addTrack(SignalType::AUDIO);
	}
	virtual void read(File *f)
	{
		string name = f->read_str();
		float volume = f->read_float();
		float muted = f->read_bool();
		auto type = (SignalType)f->read_int();
		me = parent->add_track(type);
		me->name = name;
		me->volume = volume;
		me->muted = muted;
		me->panning = f->read_float();
		me->instrument = Instrument((Instrument::Type)f->read_int());
		f->read_int(); // reserved

		notify();
	}
	virtual void write(File *f)
	{
		f->write_str(me->name);
		f->write_float(me->volume);
		f->write_bool(me->muted);
		f->write_int((int)me->type);
		f->write_float(me->panning);
		f->write_int((int)me->instrument.type);
		f->write_int(0); // reserved

		notify();
	}
	virtual void write_subs()
	{
		if (!me->instrument.has_default_tuning())
			write_sub("tuning", &me->instrument);
		write_sub_parray("level", me->layers);
		write_sub_parray("effect", me->fx);
		write_sub_parray("marker", me->markers);
		if ((me->type == SignalType::BEATS) or (me->type == SignalType::MIDI))
			if (!me->synth->isDefault())
				write_sub("synth", me->synth);
		if (me->layers[0]->midi.num > 0)
			write_sub("midi", &me->layers[0]->midi);
		if (me->has_version_selection())
			write_sub_array("fade", me->fades);
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
	//	write_sub("lvlname", me);
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
		od = nullptr;
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


void FormatNami::save_song(StorageOperationData *_od)
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
		if (s->type == SignalType::MIDI){
			if ((s->midi.samples == 0) and (s->midi.num > 0)){
				s->midi.samples = s->midi.back()->range.end();
			}
		}
	}

	for (Track *t: a->tracks){
		int n[3] = {0,0,0};
		for (TrackLayer *l: t->layers)
			for (AudioBuffer &b: l->buffers)
				n[b.channels] ++;

		for (int i=t->layers.num-1; i>=1; i--)
			if ((t->layers[i]->buffers.num == 0) and (t->layers[i]->midi.num == 0))
				t->layers.erase(i);

		if (n[2] > 0 and n[1] == 0){
			t->channels = 2;
			for (TrackLayer *l: t->layers)
				l->channels = 2;
		}
	}
}

void FormatNami::load_song(StorageOperationData *_od)
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


