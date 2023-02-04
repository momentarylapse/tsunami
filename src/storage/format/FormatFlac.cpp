/*
 * FormatFlac.cpp
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#include "FormatFlac.h"
#include <math.h>

#include "../../module/port/Port.h"
#include "../../Session.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/Song.h"
#include "../../data/base.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/os/file.h"
#include "../../lib/os/msg.h"
#if HAS_LIB_FLAC
#include <FLAC/all.h>

bool flac_tells_samples;
int flac_offset;
int flac_samples, flac_file_size;
int flac_read_samples;


// -> FormatOgg.cpp
string tag_from_vorbis(const string &key);
string tag_to_vorbis(const string &key);


FLAC__StreamDecoderWriteStatus flac_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data) {
	if (frame->header.number.sample_number == 0)
		flac_read_samples = 0;
	StorageOperationData *od = (StorageOperationData*)client_data;

	int channels = frame->header.channels;
	int bits = frame->header.bits_per_sample;
	int freq = frame->header.sample_rate;

	if (flac_tells_samples)
		od->set((float)flac_read_samples / (float)flac_samples);
	else // estimate... via increasingly compressed size
		od->set(( 1 - exp(- (float)(flac_read_samples * channels * (bits / 8)) / (float)flac_file_size ) ));

	// read decoded PCM samples
	Range range = Range(flac_read_samples + flac_offset, frame->header.blocksize);
	AudioBuffer buf;
	auto *a = od->layer->edit_buffers(buf, range);

	float scale = pow(2.0f, bits-1);
	for (int ci=0; ci<buf.channels; ci++) {
		const FLAC__int32 *source = buffer[min(ci, channels)];
		for (int i=0;i<(int)frame->header.blocksize;i++)
			buf.c[ci][i] = source[i] / scale;
	}

	od->layer->edit_buffers_finish(a);

	flac_read_samples += frame->header.blocksize;
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flac_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
	StorageOperationData *od = (StorageOperationData*)client_data;
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		int bits = metadata->data.stream_info.bits_per_sample;
		flac_samples = metadata->data.stream_info.total_samples;
		flac_tells_samples = (flac_samples != 0);
		od->suggest_channels(metadata->data.stream_info.channels);
		od->suggest_samplerate(metadata->data.stream_info.sample_rate);
		od->suggest_default_format(format_for_bits(bits, true));
		//printf("%d %d %d %d\n", flac_channels, flac_bits, (int)flac_samples, flac_freq);
	} else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
		for (int i=0; i<(int)metadata->data.vorbis_comment.num_comments; i++){
			string s = (char*)metadata->data.vorbis_comment.comments[i].entry;
			int pos = s.find("=");
			if (pos >= 0)
				od->suggest_tag(tag_from_vorbis(s.head(pos)), s.tail(s.num - pos - 1));
		}
	} else {
		od->warn("flac_metadata_callback: unhandled type: " + i2s(metadata->type));
	}
}

void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
	//msg_write("error");
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

FormatDescriptorFlac::FormatDescriptorFlac() :
	FormatDescriptor("Flac", "flac", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::TAGS | Flag::READ | Flag::WRITE)
{
}

void FormatFlac::load_track(StorageOperationData *od)
{
	Track *t = od->track;
	t->song->begin_action_group("flac load track");

	FLAC__StreamDecoder *decoder = nullptr;

	try{

		auto *f = os::fs::open(od->filename, "rb");
		flac_file_size = f->get_size();
		delete f;

		flac_offset = od->offset;
		flac_read_samples = 0;
		//bits = channels = samples = freq = 0;

		decoder = FLAC__stream_decoder_new();
		if (!decoder)
			throw Exception("could not create decoder");

		//FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_STREAMINFO);
		FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

		FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_file(
								decoder,
								od->filename.c_str(),
								flac_write_callback,
								flac_metadata_callback,
								flac_error_callback, od);
		if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
			throw Exception(string("initializing decoder: ") + FLAC__StreamDecoderInitStatusString[init_status]);

		if (od->only_load_metadata){
			if (!FLAC__stream_decoder_process_until_end_of_metadata(decoder))
				throw Exception(string("decoding failed. State: ") + FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
			od->layer->add_marker(new TrackMarker(Range(0, flac_samples), "dummy"));
		}else{
			if (!FLAC__stream_decoder_process_until_end_of_stream(decoder))
				throw Exception(string("decoding failed. State: ") + FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
		}


	}catch(Exception &e){
		od->error(e.message());
	}

	if (decoder)
		FLAC__stream_decoder_delete(decoder);

	t->song->end_action_group();
}


void flac_progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
	StorageOperationData *od = (StorageOperationData*)client_data;
	od->set((float)samples_written / (float)od->get_num_samples());
}




void FormatFlac::save_via_renderer(StorageOperationData *od)
{
	Port *r = od->renderer;

	FLAC__StreamEncoder *encoder = nullptr;
	FLAC__StreamMetadata *metadata = nullptr;
	msg_write("FLAC");

	try{
		FLAC__StreamEncoderInitStatus init_status;
		FLAC__StreamMetadata_VorbisComment_Entry entry;

		int channels = od->channels_suggested;
		SampleFormat format = SampleFormat::INT_16;
		if (od->song)
			format = od->song->default_format;
		int bits = format_get_bits(format);
		if (bits > 24)
			bits = 24;
		msg_write(channels);

		// allocate the encoder
		encoder = FLAC__stream_encoder_new();
		if (!encoder)
			throw Exception("allocating encoder");

//		if (!FLAC__stream_encoder_set_verify(encoder, true))
//			throw string("FLAC__stream_encoder_set_verify");
		if (!FLAC__stream_encoder_set_compression_level(encoder, 5))
			throw Exception("could not set compression level");
		if (!FLAC__stream_encoder_set_channels(encoder, channels))
			throw Exception("could not set channels");
		if (!FLAC__stream_encoder_set_bits_per_sample(encoder, bits))
			throw Exception("could not set bits per sample");
		if (!FLAC__stream_encoder_set_sample_rate(encoder, od->session->sample_rate()))
			throw Exception("could not set sample rate");
		if (!FLAC__stream_encoder_set_total_samples_estimate(encoder, od->get_num_samples()))
			throw Exception("could not set total samples estimate");

		// metadata
		metadata = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
		if (!metadata)
			throw Exception("could not add meta data");
		for (Tag &t : od->tags){
			FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, tag_to_vorbis(t.key).c_str(), t.value.c_str());
			FLAC__metadata_object_vorbiscomment_append_comment(metadata, entry, true);
		}

		if (!FLAC__stream_encoder_set_metadata(encoder, &metadata, 1))
			throw Exception("could not set meta data");

		// initialize encoder
		init_status = FLAC__stream_encoder_init_file(encoder, od->filename.c_str(), flac_progress_callback, od);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
			throw Exception(string("initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status]);


		FLAC__int32 *flac_pcm = new FLAC__int32[CHUNK_SAMPLES/*samples*/ * channels];

		// read blocks of samples from WAVE file and feed to encoder
		float scale = (float)(1 << (bits-1));
		AudioBuffer buf(CHUNK_SAMPLES, channels);
		int samples_read;
		while ((samples_read = r->read_audio(buf)) > 0){
			/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
			FLAC__int32 *pp = flac_pcm;
			for (int i=0;i<samples_read;i++){
				for (int c=0; c<channels; c++)
					*(pp ++) = (int)(buf.c[c][i] * scale);
			}
			/* feed samples to encoder */
			if (!FLAC__stream_encoder_process_interleaved(encoder, flac_pcm, samples_read))
				throw Exception(string("error while encoding. State: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
		}

		delete[] flac_pcm;

		if (!FLAC__stream_encoder_finish(encoder))
			throw Exception(string("error while encoding. State: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);

	}catch(Exception &e){
		od->error(e.message());
	}

	// now that encoding is finished, the metadata can be freed
	if (metadata)
		FLAC__metadata_object_delete(metadata);

	if (encoder)
		FLAC__stream_encoder_delete(encoder);
}

#endif
