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


class ChunkedFileFormatNami;

StorageOperationData *cur_op(FileChunkBasic *c);



FormatDescriptorNami::FormatDescriptorNami() :
	FormatDescriptor("Tsunami", "nami", Flag::AUDIO | Flag::MIDI | Flag::FX | Flag::MULTITRACK | Flag::TAGS | Flag::SAMPLES | Flag::READ | Flag::WRITE) {}


class FileChunkTag : public FileChunk<Song,Tag> {
public:
	FileChunkTag() : FileChunk<Song,Tag>("tag") {}
	void create() override {
		parent->tags.add(Tag());
		me = &parent->tags.back();
	}
	void read(File *f) override {
		me->key = f->read_str();
		me->value = f->read_str();
	}
	void write(File *f) override {
		f->write_str(me->key);
		f->write_str(me->value);
	}
};

class FileChunkLayerName : public FileChunk<Song,Song> {
public:
	FileChunkLayerName() : FileChunk<Song,Song>("lvlname") {}
	void create() override { me = parent; }
	void read(File *f) override {
		int num = f->read_int();
		for (int i=0;i<num;i++)
			f->read_str();
		/*me->layers.clear();
		for (int i=0;i<num;i++)
			me->layers.add(new Song::Layer(f->read_str()));*/
	}
	void write(File *f) override {
		/*f->write_int(me->layers.num);
		for (auto l: me->layers)
			f->write_str(l->name);*/
	}
};

class FileChunkFormat : public FileChunk<Song,Song> {
public:
	FileChunkFormat() : FileChunk<Song,Song>("format") {}
	void create() override { me = parent; }
	void read(File *f) override {
		me->sample_rate = f->read_int();
		me->default_format = (SampleFormat)f->read_int();
		f->read_int(); // channels
		me->compression = f->read_int();
		f->read_int();
		f->read_int();
		f->read_int();
		f->read_int();
	}
	void write(File *f) override {
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

class FileChunkSend : public FileChunk<Song,Song> {
public:
	FileChunkSend() : FileChunk<Song,Song>("send") {}
	void create() override { me = parent; }
	void read(File *f) override {
		f->read_int();
		for (Track *t: me->tracks) {
			int i = f->read_int();
			if (i >= 0 and i < me->tracks.num)
				t->send_target = me->tracks[i];
		}
	}
	void write(File *f) override {
		f->write_int(0);
		for (Track *t: me->tracks)
			f->write_int(get_track_index(t->send_target));
	}
};

class FileChunkEffect : public FileChunk<Track,AudioEffect> {
public:
	FileChunkEffect() : FileChunk<Track,AudioEffect>("effect") {}
	void create() override {}
	void read(File *f) override {
		me = CreateAudioEffect(cur_op(this)->session, f->read_str());
		f->read_bool();
		int _chunk_version = f->read_int();
		f->read_int();
		string params = f->read_str();
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		int version = Module::VERSION_LEGACY;
		if (_chunk_version >= 1) {
			version = f->read_int();
		}

		me->config_from_string(version, params);
		me->_config_latest_history = params;
		parent->add_effect(me);
	}
	void write(File *f) override {
		f->write_str(me->module_subtype);
		f->write_bool(false);
		f->write_int(1); // chunk version
		f->write_int(0);
		f->write_str(me->config_to_string());
		f->write_str(me->enabled ? "" : "disabled");
		f->write_int(me->version());
	}
};

// DEPRECATED
class FileChunkGlobalEffect : public FileChunk<Song,AudioEffect> {
public:
	FileChunkGlobalEffect() : FileChunk<Song,AudioEffect>("effect") {}
	void create() override {}
	void read(File *f) override {
		me = CreateAudioEffect(cur_op(this)->session, f->read_str());
		f->read_bool();
		f->read_int();
		f->read_int();
		string params = f->read_str();
		me->config_from_string(Module::VERSION_LEGACY, params);
		string temp = f->read_str();
		if (temp.find("disabled") >= 0)
			me->enabled = false;
		parent->__fx.add(me);
	}
	void write(File *f) override {
		f->write_str(me->module_subtype);
		f->write_bool(false);
		f->write_int(0);
		f->write_int(0);
		f->write_str(me->config_to_string());
		f->write_str(me->enabled ? "" : "disabled");
	}
};

class FileChunkCurve : public FileChunk<Song,Curve> {
public:
	FileChunkCurve() : FileChunk<Song,Curve>("curve") {}
	void create() override {
		me = new Curve;
		parent->curves.add(me);
	}
	void read(File *f) override {
		f->read_int();
		me->name = f->read_str();
		int n = f->read_int();
		for (int i=0; i<n; i++) {
			Curve::Target t;
			t.from_string(f->read_str(), parent);
			me->targets.add(t);
		}
		me->min = f->read_float();
		me->max = f->read_float();
		n = f->read_int();
		for (int i=0; i<n; i++) {
			Curve::Point p;
			p.pos = f->read_int();
			p.value = f->read_float();
			me->points.add(p);
		}
		parent->notify(parent->MESSAGE_ADD_CURVE);
	}
	void write(File *f) override {
		f->write_int(0); // version
		f->write_str(me->name);
		f->write_int(me->targets.num);
		for (auto t: me->targets)
			f->write_str(t.str(parent));
		f->write_float(me->min);
		f->write_float(me->max);
		f->write_int(me->points.num);
		for (auto p: me->points) {
			f->write_int(p.pos);
			f->write_float(p.value);
		}
	}
};

#if HAS_LIB_FLAC == 0

string compress_buffer(AudioBuffer &b, Song *song, FileChunkBasic *p) {
	return "";
}

void uncompress_buffer(AudioBuffer &b, string &data, FileChunkBasic *p) {
}

#else

FLAC__StreamEncoderWriteStatus FlacCompressWriteCallback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data) {
	string *data = (string*)client_data;
	for (unsigned int i=0; i<bytes; i++)
		data->add(buffer[i]);

	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

// TODO: allow mono...
string compress_buffer(AudioBuffer &b, Song *song, FileChunkBasic *p) {
	string data;


	bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;

	int channels = 2;
	int bits = min(format_get_bits(song->default_format), 24);
	float scale = pow(2.0f, bits-1);

	// allocate the encoder
	FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
	if (!encoder) {
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
	if (ok) {
		init_status = FLAC__stream_encoder_init_stream(encoder, &FlacCompressWriteCallback, nullptr, nullptr, nullptr, (void*)&data);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			p->error(string("flac: initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok) {
		FLAC__int32 *flac_pcm = new FLAC__int32 [CHUNK_SAMPLES/*samples*/ * 2/*channels*/];

		int p0 = 0;
		size_t left = (size_t)b.length;
		while (ok and left) {
			size_t need = (left > CHUNK_SAMPLES ? (size_t)CHUNK_SAMPLES : (size_t)left);
			{
				/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
				for (unsigned int i=0;i<need;i++) {
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

	if (!ok) {
		p->error("flac: encoding: FAILED");
		p->error(string("   state: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
	}

	FLAC__stream_encoder_delete(encoder);

	return data;
}

struct UncompressData {
	AudioBuffer *buf;
	string *data;
	int sample_offset;
	int byte_offset;
	int bits;
	int channels;
};

void FlacUncompressMetaCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
	UncompressData *d = (UncompressData*)client_data;
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		d->bits = metadata->data.stream_info.bits_per_sample;
		d->channels = metadata->data.stream_info.channels;
	}
}

FLAC__StreamDecoderWriteStatus FlacUncompressWriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data) {
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

FLAC__StreamDecoderReadStatus FlacUncompressReadCallback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data) {
	UncompressData *d = (UncompressData*)client_data;
	*bytes = min((int)*bytes, d->data->num - d->byte_offset);
	//msg_write(format("read %d", *bytes));
	if (*bytes <= 0)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	memcpy(buffer, (char*)d->data->data + d->byte_offset, *bytes);
	d->byte_offset += *bytes;
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
	//msg_write("error");
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

void uncompress_buffer(AudioBuffer &b, string &data, FileChunkBasic *p) {
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
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		p->error(string("flac: initializing decoder: ") + FLAC__StreamDecoderInitStatusString[init_status]);
		ok = false;
	}

	if (ok) {
		ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
		if (!ok) {
			p->error("flac: decoding FAILED");
			p->error(string("   state: ") + FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
		}
	}
	FLAC__stream_decoder_delete(decoder);
}
#endif

class FileChunkBufferBox : public FileChunk<TrackLayer,AudioBuffer> {
public:
	FileChunkBufferBox() : FileChunk<TrackLayer,AudioBuffer>("bufbox") {}
	void create() override {
		AudioBuffer dummy;
		parent->buffers.add(dummy);
		me = &parent->buffers.back();
	}
	void read(File *f) override {
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
		for (int n=0; n<data.num / CHUNK_SIZE; n++) {
			f->read_buffer(&data[offset], CHUNK_SIZE);
			notify();
			offset += CHUNK_SIZE;
		}
		f->read_buffer(&data[offset], data.num % CHUNK_SIZE);

		// insert

		Song *song = (Song*)root->base->get();
		if (song->compression > 0) {
			//throw Exception("can't read compressed nami files yet");
			uncompress_buffer(*me, data, this);

		}else{
			me->import(data.data, me->channels, format_for_bits(bits), num);
		}
	}
	void write(File *f) override {
		Song *song = (Song*)root->base->get();

		f->write_int(me->offset);
		f->write_int(me->length);
		f->write_int(me->channels);
		f->write_int(format_get_bits(song->default_format));

		string data;
		if (song->compression == 0) {
			if (!me->exports(data, me->channels, song->default_format))
				warn(_("Amplitude too large, signal distorted."));
		}else{

			int uncompressed_size = me->length * me->channels * format_get_bits(song->default_format) / 8;
			data = compress_buffer(*me, song, this);
			cur_op(this)->session->i(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
		}
		f->write_buffer(data);
	}
};

class FileChunkSampleBufferBox : public FileChunk<Sample,AudioBuffer> {
public:
	FileChunkSampleBufferBox() : FileChunk<Sample,AudioBuffer>("bufbox") {}
	void create() override {
		me = parent->buf;
	}
	void read(File *f) override {
		me->offset = f->read_int();
		int num = f->read_int();
		int channels = f->read_int(); // channels (2)
		int bits = f->read_int(); // bit (16)

		me->set_channels(channels);
		me->resize(num);

		string data;

		int bytes = context->layers.back().size - 16;
		data.resize(bytes);//num * (bits / 8) * channels);

		// read chunk'ed
		int offset = 0;
		for (int n=0; n<data.num / CHUNK_SIZE; n++) {
			f->read_buffer(&data[offset], CHUNK_SIZE);
			notify();
			offset += CHUNK_SIZE;
		}
		f->read_buffer(&data[offset], data.num % CHUNK_SIZE);

		// insert

		Song *song = (Song*)root->base->get();
		if (song->compression > 0) {
			//throw Exception("can't read compressed nami files yet");
			uncompress_buffer(*me, data, this);

		}else{
			if (bytes > 0)
				me->import(data.data, channels, format_for_bits(bits), num);
		}
	}
	void write(File *f) override {
		Song *song = (Song*)root->base->get();

		f->write_int(me->offset);
		f->write_int(me->length);
		f->write_int(me->channels);
		f->write_int(format_get_bits(song->default_format));

		string data;
		if (song->compression == 0) {
			if (!me->exports(data, me->channels, song->default_format))
				warn(_("Amplitude too large, signal distorted."));
		}else{

			int uncompressed_size = me->length * me->channels * format_get_bits(song->default_format) / 8;
			data = compress_buffer(*me, song, this);
			cur_op(this)->session->i(format("compress:  %d  -> %d    %.1f%%", uncompressed_size, data.num, (float)data.num / (float)uncompressed_size * 100.0f));
		}
		f->write_buffer(data);
	}
};

class FileChunkSampleRef : public FileChunk<TrackLayer,SampleRef> {
public:
	FileChunkSampleRef() : FileChunk<TrackLayer,SampleRef>("samref") {}
	void create() override {}
	void read(File *f) override {
		string name = f->read_str();
		int pos = f->read_int();
		int index = f->read_int();
		float volume = f->read_float();
		bool muted = f->read_bool();
		int uid = f->read_int();
		if (uid != 0)
			me = parent->add_sample_ref(pos, parent->track->song->samples[index]);
		else
			me = parent->add_sample_ref(pos, parent->track->song->samples[index]);
		me->volume = volume;
		me->muted = muted;
		f->read_int();
		f->read_int(); // reserved
		f->read_int();
	}
	void write(File *f) override {
		f->write_str(me->origin->name);
		f->write_int(me->pos);
		f->write_int(me->origin->get_index());
		f->write_float(me->volume);
		f->write_bool(me->muted);
		f->write_int(me->origin->uid);
		f->write_int(0);
		f->write_int(0); // reserved
		f->write_int(0);
	}
};

// deprecated
class _FileChunkTrackSampleRef : public FileChunk<Track,SampleRef> {
public:
	_FileChunkTrackSampleRef() : FileChunk<Track,SampleRef>("samref") {}
	void create() override {}
	void read(File *f) override {
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
	void write(File *f) override {
	}
};


class FileChunkMidiEvent : public FileChunk<MidiNoteBuffer,MidiEvent> {
public:
	FileChunkMidiEvent() : FileChunk<MidiNoteBuffer,MidiEvent>("event") {}
	void create() override {
		//parent->add(MidiEvent());
		//me = &parent->back();
	}
	void read(File *f) override {
		MidiEvent e;
		e.pos = f->read_int();
		e.pitch = f->read_int();
		e.volume = f->read_float();
		f->read_int(); // reserved

		int unended = -1;
		foreachi(MidiNote *n, *parent, i)
			if ((n->pitch == e.pitch) and (n->range.length == -1))
				unended = i;

		if ((unended >= 0) and (e.volume == 0)) {
			(*parent)[unended]->range.set_end(e.pos);
		}else if ((unended < 0) and (e.volume > 0)) {
			parent->add(new MidiNote(Range(e.pos, -1), e.pitch, e.volume));
		}else if (unended >= 0) {
			error("nami/midi: starting new note without ending old one");
		}else{
			error("nami/midi: no note to end");
		}
	}
	void write(File *f) override {
		f->write_int(me->pos);
		f->write_int(me->pitch);
		f->write_float(me->volume);
		f->write_int(0); // reserved
	}
};

class FileChunkMidiEffect : public FileChunk<MidiNoteBuffer,MidiEffect> {
public:
	FileChunkMidiEffect() : FileChunk<MidiNoteBuffer,MidiEffect>("effect") {}
	void create() override {}
	void read(File *f) override {
		string name = f->read_str();
		f->read_bool();
		f->read_int(); // reserved
		int version = f->read_int();
		string params = f->read_str();
		string temp = f->read_str();

		if (version == 0)
			version = Module::VERSION_LEGACY;

		me = CreateMidiEffect(cur_op(this)->session, name);
		me->config_from_string(version, params);
		me->_config_latest_history = params;
		if (temp.find("disabled") >= 0)
			me->enabled = false;

		Song *song = (Song*)root->base->get();
		song->tracks.back()->midi_fx.add(me);
	}
	void write(File *f) override {
		f->write_str(me->module_subtype);
		f->write_bool(false);
		f->write_int(0); // reserved
		f->write_int(me->version());
		f->write_str(me->config_to_string());
		f->write_str(me->enabled ? "" : "disabled");
	}
};


class FileChunkMidiNote : public FileChunk<MidiNoteBuffer,MidiNote> {
public:
	FileChunkMidiNote(const string &name) : FileChunk<MidiNoteBuffer,MidiNote>(name) {}
	void create() override {}
	void read(File *f) override {
		MidiNote *n = new MidiNote;
		n->range.offset = f->read_int();
		n->range.length = f->read_int();
		n->pitch = f->read_int();
		n->volume = f->read_float();
		f->read_int(); // reserved
		parent->add(n);
	}
	void write(File *f) override {
		f->write_int(me->range.offset);
		f->write_int(me->range.length);
		f->write_int(me->pitch);
		f->write_float(me->volume);
		f->write_int(0); // reserved
	}
};

class FileChunkSampleMidiData : public FileChunk<Sample,MidiNoteBuffer> {
public:
	FileChunkSampleMidiData() : FileChunk<Sample,MidiNoteBuffer>("midi") {}
	void define_children() override {
		add_child(new FileChunkMidiEvent);
		add_child(new FileChunkMidiNote("note"));
		//add_child(new FileChunkMidiEffect);
	}
	void create() override { me = &parent->midi; }
	void read(File *f) override {
		f->read_str();
		f->read_str();
		f->read_str();
		int version = f->read_int();
		if (version < 1)
			return;
		int num = f->read_int();
		int meta = f->read_int();
		for (int i=0; i<num; i++) {
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
	void write(File *f) override {
		f->write_str("");
		f->write_str("");
		f->write_str("");
		f->write_int(1); // version

		f->write_int(me->num);
		f->write_int(3); // stringno + clef_position
		for (MidiNote *n : *me) {
			f->write_int(n->range.offset);
			f->write_int(n->range.length);
			f->write_int(n->pitch);
			f->write_float(n->volume);
			f->write_int(n->stringno);
			f->write_int(n->clef_position);
		}
		f->write_int(0); // reserved
	}
	void write_subs() override {
		//write_sub_parray("effect", me->fx);
	}
};

static bool note_buffer_has_flags(MidiNoteBuffer &buf) {
	for (auto *n: buf)
		if (n->flags > 0)
			return true;
	return false;
}

class FileChunkTrackMidiData : public FileChunk<Track,MidiNoteBuffer> {
public:
	FileChunkTrackMidiData() : FileChunk<Track,MidiNoteBuffer>("midi") {}
	void define_children() override {
		add_child(new FileChunkMidiEvent);
		add_child(new FileChunkMidiNote("note"));
		add_child(new FileChunkMidiNote("midinote")); // deprecated
		add_child(new FileChunkMidiEffect);
	}
	void create() override { me = &parent->layers[0]->midi; }
	void read(File *f) override {
		f->read_str();
		f->read_str();
		f->read_str();
		int version = f->read_int();
		if (version < 1)
			return;
		int num = f->read_int();
		int meta = f->read_int();
		for (int i=0; i<num; i++) {
			MidiNote *n = new MidiNote;
			n->range.offset = f->read_int();
			n->range.length = f->read_int();
			n->pitch = f->read_int();
			n->volume = f->read_float();
			if (meta & 1)
				n->stringno = f->read_int();
			if (meta & 2)
				n->clef_position = f->read_int();
			if (meta & 4)
				n->flags = f->read_int();
			me->add(n);
		}
		f->read_int(); // reserved
	}
	void write(File *f) override {
		bool has_flags = note_buffer_has_flags(*me);
		f->write_str("");
		f->write_str("");
		f->write_str("");
		f->write_int(1); // version

		f->write_int(me->num);
		f->write_int(has_flags ? 7 : 3); // stringno + clef_position (+ flags)
		for (MidiNote *n : *me) {
			f->write_int(n->range.offset);
			f->write_int(n->range.length);
			f->write_int(n->pitch);
			f->write_float(n->volume);
			f->write_int(n->stringno);
			f->write_int(n->clef_position);
			if (has_flags)
				f->write_int(n->flags);
		}
		f->write_int(0); // reserved
	}
	void write_subs() override {
		write_sub_parray("effect", parent->midi_fx);
	}
};

class FileChunkSample : public FileChunk<Song,Sample> {
public:
	FileChunkSample() : FileChunk<Song,Sample>("sample") {}
	void define_children() override {
		add_child(new FileChunkSampleBufferBox);
		add_child(new FileChunkSampleMidiData);
	}
	void create() override {
		me = new Sample(SignalType::AUDIO);
		me->set_owner(parent);
		parent->samples.add(me);
	}
	void read(File *f) override {
		me->name = f->read_str();
		me->volume = f->read_float();
		me->offset = f->read_int();
		me->type = (SignalType)f->read_int();
		int uid = f->read_int();
		if (uid != 0)
			me->uid = uid;
	}
	void write(File *f) override {
		f->write_str(me->name);
		f->write_float(me->volume);
		f->write_int(me->offset);
		f->write_int((int)me->type);
		f->write_int(me->uid);
	}
	void write_subs() override {
		if (me->type == SignalType::AUDIO)
			write_sub("bufbox", me->buf);
		else if (me->type == SignalType::MIDI)
			write_sub("midi", &me->midi);
	}
};

class FileChunkFade: public FileChunk<TrackLayer,CrossFade> {
public:
	FileChunkFade() : FileChunk<TrackLayer,CrossFade>("fade") {}
	void create() override {}
	void read(File *f) override {
		CrossFade ff;
		ff.position = f->read_int();
		ff.mode = (CrossFade::Mode)f->read_int();
		ff.samples = f->read_int();
		f->read_int();
		f->read_int();
		parent->fades.add(ff);
	}
	void write(File *f) override {
		f->write_int(me->position);
		f->write_int((int)me->mode);
		f->write_int(me->samples);
		f->write_int(0);
		f->write_int(0);
	}
};


class FileChunkMarker : public FileChunk<TrackLayer,TrackMarker> {
public:
	FileChunkMarker() : FileChunk<TrackLayer,TrackMarker>("marker") {}
	void create() override {
		me = new TrackMarker();
		parent->markers.add(me);
	}
	void read(File *f) override {
		me->range.offset = f->read_int();
		me->range.length = f->read_int();
		me->text = f->read_str();
		int nfx = f->read_int();
		int version = f->read_int();
	}
	void write(File *f) override {
		f->write_int(me->range.offset);
		f->write_int(me->range.length);
		f->write_str(me->text);
		f->write_int(0);//me->fx.num);
		f->write_int(1);
	}
};

class FileChunkTrackLayer : public FileChunk<Track,TrackLayer> {
public:
	FileChunkTrackLayer() : FileChunk<Track,TrackLayer>("level") {}
	void define_children() override {
		add_child(new FileChunkBufferBox);
		add_child(new FileChunkSampleRef);
		add_child(new FileChunkFade);
		add_child(new FileChunkMarker);
	}
	void create() override {}
	void read(File *f) override {
		int n = f->read_int();
		if (n > 0) {
			parent->layers.add(new TrackLayer(parent));
		}
		me = parent->layers.back();
	}
	void write(File *f) override {
		int n = 0;
		foreachi(TrackLayer *l, parent->layers, i)
			if (l == me)
				n = i;
		f->write_int(n);
	}
	void write_subs() override {
		write_sub_array("bufbox", me->buffers);
		write_sub_parray("samref", me->samples);
		write_sub_parray("marker", me->markers);
		write_sub_array("fade", me->fades);
	}
};

class FileChunkSynthesizerTuning : public FileChunk<Synthesizer,Synthesizer::Tuning> {
public:
	FileChunkSynthesizerTuning() : FileChunk<Synthesizer,Synthesizer::Tuning>("tuning") {}
	void create() override { me = &parent->tuning; }
	void read(File *f) override {
		for (int i=0; i<MAX_PITCH; i++)
			me->freq[i] = f->read_float();
		parent->update_delta_phi();
	}
	void write(File *f) override {
		for (int i=0; i<MAX_PITCH; i++)
			f->write_float(me->freq[i]);
	}
};

class FileChunkSynthesizer : public FileChunk<Track,Synthesizer> {
public:
	FileChunkSynthesizer() : FileChunk<Track,Synthesizer>("synth") {}
	void define_children() override {
		add_child(new FileChunkSynthesizerTuning);
	}
	void create() override {}
	void read(File *f) override {
		Session *session = cur_op(this)->session;
		string name = f->read_str();
		string param = f->read_str();
		int version = Module::VERSION_LEGACY;
		f->read_str();
		int _v = f->read_int();
		if (_v > 0) {
			version = f->read_int();
			f->read_int(); // reserved
		}
		if (name == "Drumset") {
			session->w("converting drumset...");
			name = "Font";
			param = "(\"drumset-1\")";
		}
		me = CreateSynthesizer(session, name);
		me->config_from_string(version, param);
		me->_config_latest_history = param;

		parent->set_synthesizer(me);
	}
	void write(File *f) override {
		f->write_str(me->module_subtype);
		f->write_str(me->config_to_string());
		f->write_str("");
		f->write_int(1);
		f->write_int(me->version());
		f->write_int(0);// reserved
	}
	void write_subs() override {
		if (!me->tuning.is_default())
			write_sub("tuning", &me->tuning);
	}
};

class FileChunkBar : public FileChunk<Song,Bar> {
public:
	FileChunkBar() : FileChunk<Song,Bar>("bar") {}
	void create() override { me = nullptr; }
	void read(File *f) override {
		int type = f->read_int();
		int length = f->read_int();
		int num_beats = f->read_int();
		if (type == BarPattern::Type::PAUSE)
			num_beats = 0;
		int count = f->read_int();
		int divisor = f->read_int();
		if (divisor <= 0)
			divisor = 1;
		BarPattern b = BarPattern(length, num_beats, divisor);

		// pattern
		if (type >= 42) {
			for (int i=0; i<num_beats; i++)
				b.beats[i] = f->read_int();
			b.update_total();
		}

		for (int i=0; i<count; i++) {
			parent->bars.add(new Bar(b));
		}
	}
	void write(File *f) override {
		if (me->is_pause())
			f->write_int(BarPattern::Type::PAUSE);
		else if (me->is_uniform())
			f->write_int(BarPattern::Type::BAR);
		else
			f->write_int(BarPattern::Type::BAR + 42);
		f->write_int(me->length);
		f->write_int(me->beats.num);
		f->write_int(1);
		f->write_int(me->divisor);
		if (!me->is_uniform())
			for (int b: me->beats)
				f->write_int(b);
	}
};

// deprecated
class FileChunkTrackBar : public FileChunk<Track,Bar> {
public:
	FileChunkTrackBar() : FileChunk<Track,Bar>("bar") {}
	void create() override { me = NULL; }
	void read(File *f) override {
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
	void write(File *f) override {
		root->on_error("deprecated... TrackBar.write");
	}
};

class FileChunkMarkerOld : public FileChunk<Track,TrackMarker> {
public:
	FileChunkMarkerOld() : FileChunk<Track,TrackMarker>("marker") {}
	void create() override {
		me = new TrackMarker();
		parent->_markers_old.add(me);
	}
	void read(File *f) override {
		me->range.offset = f->read_int();
		me->text = f->read_str();
		int version = f->read_int();
		if (version > 0) {
			me->range.length = f->read_int();
			int nfx = f->read_int();
		}
	}
	void write(File *f) override {
		f->write_int(me->range.offset);
		f->write_str(me->text);
		f->write_int(1);
		f->write_int(me->range.length);
		f->write_int(0);//me->fx.num);
	}
};

class FileChunkFadeOld: public FileChunk<Track,CrossFadeOld> {
public:
	FileChunkFadeOld() : FileChunk<Track,CrossFadeOld>("fade") {}
	void create() override {
	}
	void read(File *f) override {
		CrossFadeOld ff;
		ff.position = f->read_int();
		ff.target = f->read_int();
		ff.samples = f->read_int();
		f->read_int();
		f->read_int();
		parent->_fades_old.add(ff);
	}
	void write(File *f) override {
		f->write_int(me->position);
		f->write_int(me->target);
		f->write_int(me->samples);
		f->write_int(0);
		f->write_int(0);
	}
};

class FileChunkTuning : public FileChunk<Track,Instrument> {
public:
	FileChunkTuning() : FileChunk<Track,Instrument>("tuning") {}
	void create() override {
		me = &parent->instrument;
	}
	void read(File *f) override {
		me->string_pitch.resize(f->read_int());
		for (int i=0; i<me->string_pitch.num; i++)
			me->string_pitch[i] = f->read_int();
	}
	void write(File *f) override {
		f->write_int(me->string_pitch.num);
		for (int i=0; i<me->string_pitch.num; i++)
			f->write_int(me->string_pitch[i]);
	}
};

class FileChunkTrack : public FileChunk<Song,Track> {
public:
	FileChunkTrack() : FileChunk<Song,Track>("track") {}
	void define_children() override {
		add_child(new FileChunkTuning);
		add_child(new FileChunkTrackLayer);
		add_child(new FileChunkSynthesizer);
		add_child(new FileChunkEffect);
		add_child(new FileChunkTrackMidiData);
		add_child(new FileChunkTrackBar); // deprecated
		add_child(new FileChunkMarkerOld); // deprecated
		add_child(new _FileChunkTrackSampleRef); // deprecated
			//s->AddChunkHandler("sub", (chunk_reader*)&ReadChunkSub, t);
		add_child(new FileChunkFadeOld); // deprecated
	}
	void create() override {
		//me = parent->addTrack(SignalType::AUDIO);
	}
	void read(File *f) override {
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
	void write(File *f) override {
		f->write_str(me->name);
		f->write_float(me->volume);
		f->write_bool(me->muted);
		f->write_int((int)me->type);
		f->write_float(me->panning);
		f->write_int((int)me->instrument.type);
		f->write_int(0); // reserved

		notify();
	}
	void write_subs() override {
		if (!me->instrument.has_default_tuning())
			write_sub("tuning", &me->instrument);
		write_sub_parray("level", me->layers);
		write_sub_parray("effect", me->fx);
		if ((me->type == SignalType::BEATS) or (me->type == SignalType::MIDI))
			if (!me->synth->is_default())
				write_sub("synth", me->synth);
		if (me->layers[0]->midi.num > 0)
			write_sub("midi", &me->layers[0]->midi);
	}
};

class FileChunkSecret : public FileChunk<Song,Any> {
public:
	FileChunkSecret() : FileChunk<Song,Any>("secret") {}
	void read(File *f) override {
		parent->secret_data = Any::parse(f->read_str());
	}
	void write(File *f) override {
		f->write_str(parent->secret_data.str());
	}
};

class FileChunkNami : public FileChunk<Song,Song> {
public:
	FileChunkNami() : FileChunk<Song,Song>("nami") {}
	void define_children() override {
		add_child(new FileChunkFormat);
		add_child(new FileChunkTag);
		add_child(new FileChunkLayerName);
		add_child(new FileChunkBar);
		add_child(new FileChunkSample);
		add_child(new FileChunkTrack);
		add_child(new FileChunkGlobalEffect);
		add_child(new FileChunkCurve);
		add_child(new FileChunkSend);
		add_child(new FileChunkSecret);
	}
	void create() override { me = parent; }
	void read(File *f) override {
		me->sample_rate = f->read_int();
	}
	void write(File *f) override {
		f->write_int(me->sample_rate);
	}
	void write_subs() override {
		write_sub("format", me);
		write_sub_array("tag", me->tags);
	//	write_sub("lvlname", me);
		write_sub_parray("bar", me->bars);
		write_sub_parray("sample", me->samples);
		write_sub_parray("track", me->tracks);
		write_sub_parray("curve", me->curves);
		bool needs_send = false;
		for (Track *t: me->tracks)
			if (t->send_target)
				needs_send = true;
		if (needs_send)
			write_sub("send", me);
		if (!me->secret_data.is_empty())
			write_sub("secret", &me->secret_data);
	}
};

class ChunkedFileFormatNami : public ChunkedFileParser {
public:
	ChunkedFileFormatNami() :
		ChunkedFileParser(8) {
		od = nullptr;
		set_base(new FileChunkNami);
	}
	StorageOperationData *od;

	bool read_file(StorageOperationData *_od) {
		od = _od;
		return this->read(od->filename, od->song);
	}
	bool write_file(StorageOperationData *_od) {
		od = _od;
		return this->write(od->filename, od->song);
	}
	void on_notify() override {
		od->set((float)context.f->get_pos() / (float)context.f->get_size());
	}
	void on_unhandled() override {
		od->error("unhandled nami chunk: " + context.str());
	}
	void on_error(const string &message) override {
		od->error(message);
	}
	void on_warn(const string &message) override {
		od->warn(message);
	}
};

StorageOperationData *cur_op(FileChunkBasic *c) {
	return ((ChunkedFileFormatNami*)c->root)->od;
}


void FormatNami::save_song(StorageOperationData *od) {
	try {
		ChunkedFileFormatNami n;
		n.write_file(od);
	} catch(Exception &s) {
		od->error("saving nami: " + s.message());
	}
}


void check_empty_subs(Song *a) {
	/*foreach(Track *t, a->track)
		foreachib(Track *s, t->sub, i)
			if (s->length <= 0) {
				tsunami->log->Error("empty sub: " + s->name);
				t->sub.erase(i);
			}*/
}

void FormatNami::make_consistent(StorageOperationData *od) {
	Song *a = od->song;
	for (auto *s : a->samples) {
		if (s->type == SignalType::MIDI) {
			if ((s->midi.samples == 0) and (s->midi.num > 0)) {
				s->midi.samples = s->midi.back()->range.end();
			}
		}
	}

	for (auto *t: a->tracks) {
		int n[3] = {0,0,0};
		for (auto *l: t->layers)
			for (auto &b: l->buffers)
				n[b.channels] ++;

		/*for (int i=t->layers.num-1; i>=1; i--)
			if ((t->layers[i]->buffers.num == 0) and (t->layers[i]->midi.num == 0))
				t->layers.erase(i);*/

		// having stereo buffers?
		if (n[2] > 0) {
			t->channels = 2;
			for (auto *l: t->layers)
				l->channels = 2;
		}
	}

	if (a->__fx.num > 0) {
		od->session->w(_("file contains global fx. Will add a mastering track instead"));
		auto *tm = a->add_track(SignalType::GROUP);
		tm->name = "Master";
		tm->fx = a->__fx;
		for (Track *t: a->tracks)
			if (t != tm) {
				t->set_send_target(tm);
			}
		a->__fx.clear();
	}

	for (Track *t: a->tracks) {
		t->layers[0]->markers.append(t->_markers_old);
		
		if (t->_fades_old.num > 0) {
			int previous = 0;
			for (auto &f: t->_fades_old) {
				t->layers[previous]->fades.add({f.position, CrossFade::OUTWARD, f.samples});
				t->layers[f.target]->fades.add({f.position, CrossFade::INWARD, f.samples});
				previous = f.target;
			}
		}
		if (!t->has_version_selection())
			continue;
		bool semi_old_version = false;
		for (auto *l: t->layers) {
			if (l->fades.num > 0) {
				if (l->fades[0].mode == CrossFade::INWARD) {
					od->info("adding missing fade-in");
					l->fades.insert({a->range().start(), CrossFade::OUTWARD, l->fades[0].samples}, 0);
					semi_old_version = true;
				}
				if (l->fades.back().mode == CrossFade::OUTWARD) {
					od->info("adding missing fade-out");
					l->fades.add({a->range().end(), CrossFade::INWARD, l->fades.back().samples});
					semi_old_version = true;
				}
			}
		}
		if (semi_old_version)
			for (auto *l: t->layers) {
				if (l != t->layers[0] and l->fades.num == 0) {
					od->info("disabling non-first version without fades");
					l->fades.add({a->range().start(), CrossFade::OUTWARD, 2000});
					l->fades.add({a->range().end(), CrossFade::INWARD, 2000});
				}
			}
	}
}

void FormatNami::load_song(StorageOperationData *od) {
	// TODO?
	od->song->tags.clear();

	try {
		ChunkedFileFormatNami n;
		n.read_file(od);
	} catch(Exception &e) {
		od->error("loading nami: " + e.message());
	}

	// some post processing
	make_consistent(od);

	//od->song->updateSelection(Range(0, 0));
}


