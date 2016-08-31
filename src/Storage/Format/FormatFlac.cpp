/*
 * FormatFlac.cpp
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#include "FormatFlac.h"
#include <math.h>
#include "../../Audio/Renderer/AudioRenderer.h"
#ifndef OS_WINDOWS
#include <FLAC/all.h>

bool flac_tells_samples;
int flac_offset, flac_level;
int flac_channels, flac_bits, flac_samples, flac_freq, flac_file_size;
SampleFormat flac_format;
int flac_read_samples;
Track *flac_track;

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

// -> FormatOgg.cpp
string tag_from_vorbis(const string &key);
string tag_to_vorbis(const string &key);


FLAC__StreamDecoderWriteStatus flac_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	if (frame->header.number.sample_number == 0)
		flac_read_samples = 0;
	StorageOperationData *od = (StorageOperationData*)client_data;

	if (frame->header.number.sample_number % 1024 == 0){
		if (flac_tells_samples)
			od->set((float)(flac_read_samples / flac_channels) / (float)(flac_samples));
		else // estimate... via increasingly compressed size
			od->set(( 1 - exp(- (float)(flac_read_samples * flac_channels * (flac_bits / 8)) / (float)flac_file_size ) ));
	}

	// read decoded PCM samples
	Range range = Range(flac_read_samples + flac_offset, frame->header.blocksize);
	BufferBox buf = flac_track->getBuffers(flac_level, range);
	Action *a = new ActionTrackEditBuffer(flac_track, 0, range);
	float scale = pow(2.0f, flac_bits-1);
	for (int i=0;i<(int)frame->header.blocksize;i++)
		for (int j=0;j<flac_channels;j++)
			buf.c[j][i] = buffer[j][i] / scale;
	flac_track->song->execute(a);

	flac_read_samples += frame->header.blocksize;
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flac_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO){
		flac_freq = metadata->data.stream_info.sample_rate;
		flac_bits = metadata->data.stream_info.bits_per_sample;
		flac_format = format_for_bits(flac_bits);
		flac_samples = metadata->data.stream_info.total_samples;
		flac_tells_samples = (flac_samples != 0);
		flac_channels = metadata->data.stream_info.channels;
		//printf("%d %d %d %d\n", flac_channels, flac_bits, (int)flac_samples, flac_freq);
	}else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT){
		for (int i=0;i<(int)metadata->data.vorbis_comment.num_comments;i++){
			string s = (char*)metadata->data.vorbis_comment.comments[i].entry;
			int pos = s.find("=");
			if (pos >= 0)
				flac_track->song->addTag(tag_from_vorbis(s.head(pos)), s.tail(s.num - pos - 1));
		}
	}else{
		StorageOperationData *od = (StorageOperationData*)client_data;
		od->warn("flac_metadata_callback: unhandled type: " + i2s(metadata->type));
	}
}

void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//msg_write("error");
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

FormatDescriptorFlac::FormatDescriptorFlac() :
	FormatDescriptor("Flac", "flac", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ | FLAG_WRITE)
{
}

void FormatFlac::loadTrack(StorageOperationData *od)
{
	msg_db_f("load_flac_file", 1);
	Track *t = od->track;
	t->song->action_manager->beginActionGroup();
	bool ok = true;

	flac_file_size = 1000000000;
	File *f = FileOpen(od->filename);
	if (f){
		flac_file_size = f->GetSize();
		FileClose(f);
	}

	flac_level = od->level;
	flac_offset = od->offset;
	flac_read_samples = 0;
	flac_track = t;
	//bits = channels = samples = freq = 0;

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder){
		od->error("flac: decoder_new()");
	}

	FLAC__stream_decoder_set_metadata_respond(decoder, (FLAC__MetadataType)(FLAC__METADATA_TYPE_STREAMINFO | FLAC__METADATA_TYPE_VORBIS_COMMENT));

	FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_file(
							decoder,
							od->filename.c_str(),
							flac_write_callback,
							flac_metadata_callback,
							flac_error_callback, od);
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


	if (t->get_index() == 0){
		t->song->setSampleRate(flac_freq);
		t->song->setDefaultFormat(format_for_bits(flac_bits));
	}
	t->song->action_manager->endActionGroup();
}



#define FLAC_READSIZE 2048

void flac_progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
	StorageOperationData *od = (StorageOperationData*)client_data;
	if (samples_written % (FLAC_READSIZE * 64) == 0)
		od->set((float)samples_written / (float)od->get_num_samples());
}

static FLAC__int32 flac_pcm[FLAC_READSIZE/*samples*/ * 2/*channels*/];




void FormatFlac::saveViaRenderer(StorageOperationData *od)
{
	AudioRenderer *r = od->renderer;

	bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;
	FLAC__StreamMetadata *metadata[1];
	FLAC__StreamMetadata_VorbisComment_Entry entry;

	int channels = 2;
	SampleFormat format = SAMPLE_FORMAT_16;
	if (od->song)
		format = od->song->default_format;
	int bits = format_get_bits(format);
	if (bits > 24)
		bits = 24;

	// allocate the encoder
	FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
	if (!encoder){
		od->error("flac: allocating encoder");
		return;
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bits);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, r->getSampleRate());
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, r->getNumSamples());

	// metadata
	if (ok){
		metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
		if (metadata[0]){
			Array<Tag> tags = r->getTags();
			for (Tag &t : tags){
				FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, tag_to_vorbis(t.key).c_str(), t.value.c_str());
				FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, true);
			}
		}else{
			od->error("flac: could not add metadata");
			ok = false;
		}

		ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 1);
	}

	// initialize encoder
	if (ok){
		init_status = FLAC__stream_encoder_init_file(encoder, od->filename.c_str(), flac_progress_callback, od);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			od->error(string("flac: initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok){
		float scale = pow(2.0f, bits-1);
		BufferBox buf;
		buf.resize(FLAC_READSIZE);
		while (r->readResize(buf)){
			/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
			for (int i=0;i<buf.length;i++){
				flac_pcm[i * 2 + 0] = (int)(buf.c[0][i] * scale);
				flac_pcm[i * 2 + 1] = (int)(buf.c[1][i] * scale);
			}
			/* feed samples to encoder */
			ok = FLAC__stream_encoder_process_interleaved(encoder, flac_pcm, buf.length);
			if (!ok)
				break;
		}
	}

	ok &= FLAC__stream_encoder_finish(encoder);

	if (!ok){
		od->error("flac: encoding: FAILED");
		od->error(string("   state: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
	}

	// now that encoding is finished, the metadata can be freed
	FLAC__metadata_object_delete(metadata[0]);

	FLAC__stream_encoder_delete(encoder);
}

#endif
