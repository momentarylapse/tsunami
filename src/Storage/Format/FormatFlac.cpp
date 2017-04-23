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
int flac_offset, flac_layer;
int flac_channels, flac_bits, flac_samples, flac_freq, flac_file_size;
SampleFormat flac_format;
int flac_read_samples;

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

// -> FormatOgg.cpp
string tag_from_vorbis(const string &key);
string tag_to_vorbis(const string &key);


FLAC__StreamDecoderWriteStatus flac_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	if (frame->header.number.sample_number == 0)
		flac_read_samples = 0;
	StorageOperationData *od = (StorageOperationData*)client_data;

	if (frame->header.number.sample_number % (1<<14) == 0){
		if (flac_tells_samples)
			od->set((float)(flac_read_samples / flac_channels) / (float)(flac_samples));
		else // estimate... via increasingly compressed size
			od->set(( 1 - exp(- (float)(flac_read_samples * flac_channels * (flac_bits / 8)) / (float)flac_file_size ) ));
	}

	// read decoded PCM samples
	Range range = Range(flac_read_samples + flac_offset, frame->header.blocksize);
	BufferBox buf = od->track->getBuffers(flac_layer, range);

	Action *a;
	if (od->song->action_manager->isEnabled())
		a = new ActionTrackEditBuffer(od->track, 0, range);

	float scale = pow(2.0f, flac_bits-1);
	for (int i=0;i<(int)frame->header.blocksize;i++)
		for (int j=0;j<flac_channels;j++)
			buf.c[j][i] = buffer[j][i] / scale;

	if (od->song->action_manager->isEnabled())
		od->song->execute(a);

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
		StorageOperationData *od = (StorageOperationData*)client_data;
		for (int i=0;i<(int)metadata->data.vorbis_comment.num_comments;i++){
			string s = (char*)metadata->data.vorbis_comment.comments[i].entry;
			int pos = s.find("=");
			if (pos >= 0)
				od->song->addTag(tag_from_vorbis(s.head(pos)), s.tail(s.num - pos - 1));
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
	Track *t = od->track;
	t->song->action_manager->beginActionGroup();

	FLAC__StreamDecoder *decoder = NULL;

	try{

		File *f = FileOpen(od->filename);
		if (!f)
			throw string("can not open file");
		flac_file_size = f->GetSize();
		FileClose(f);

		flac_layer = od->layer;
		flac_offset = od->offset;
		flac_read_samples = 0;
		//bits = channels = samples = freq = 0;

		decoder = FLAC__stream_decoder_new();
		if (!decoder)
			throw string("could not create decoder");

		FLAC__stream_decoder_set_metadata_respond(decoder, (FLAC__MetadataType)(FLAC__METADATA_TYPE_STREAMINFO | FLAC__METADATA_TYPE_VORBIS_COMMENT));

		FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_file(
								decoder,
								od->filename.c_str(),
								flac_write_callback,
								flac_metadata_callback,
								flac_error_callback, od);
		if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
			throw string("initializing decoder: ") + FLAC__StreamDecoderInitStatusString[init_status];

		if (!FLAC__stream_decoder_process_until_end_of_stream(decoder))
			throw string("decoding failed. State: ") + FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)];


		if (t->get_index() == 0){
			t->song->setSampleRate(flac_freq);
			t->song->setDefaultFormat(format_for_bits(flac_bits));
		}

	}catch(string &s){
		od->error(s);
	}

	if (decoder)
		FLAC__stream_decoder_delete(decoder);

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

	FLAC__StreamEncoder *encoder = NULL;
	FLAC__StreamMetadata *metadata = NULL;

	try{
		FLAC__StreamEncoderInitStatus init_status;
		FLAC__StreamMetadata_VorbisComment_Entry entry;

		int channels = 2;
		SampleFormat format = SAMPLE_FORMAT_16;
		if (od->song)
			format = od->song->default_format;
		int bits = format_get_bits(format);
		if (bits > 24)
			bits = 24;

		// allocate the encoder
		encoder = FLAC__stream_encoder_new();
		if (!encoder)
			throw string("allocating encoder");

//		if (!FLAC__stream_encoder_set_verify(encoder, true))
//			throw string("FLAC__stream_encoder_set_verify");
		if (!FLAC__stream_encoder_set_compression_level(encoder, 5))
			throw string("could not set compression level");
		if (!FLAC__stream_encoder_set_channels(encoder, channels))
			throw string("could not set channels");
		if (!FLAC__stream_encoder_set_bits_per_sample(encoder, bits))
			throw string("could not set bits per sample");
		if (!FLAC__stream_encoder_set_sample_rate(encoder, r->getSampleRate()))
			throw string("could not set sample rate");
		if (!FLAC__stream_encoder_set_total_samples_estimate(encoder, r->getNumSamples()))
			throw string("could not set total samples estimate");

		// metadata
		metadata = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
		if (!metadata)
			throw string("could not add meta data");
		Array<Tag> tags = r->getTags();
		for (Tag &t : tags){
			FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, tag_to_vorbis(t.key).c_str(), t.value.c_str());
			FLAC__metadata_object_vorbiscomment_append_comment(metadata, entry, true);
		}

		if (!FLAC__stream_encoder_set_metadata(encoder, &metadata, 1))
			throw string("could not set meta data");

		// initialize encoder
		init_status = FLAC__stream_encoder_init_file(encoder, od->filename.c_str(), flac_progress_callback, od);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
			throw string("initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status];

		// read blocks of samples from WAVE file and feed to encoder
		float scale = (float)(1 << (bits-1));
		BufferBox buf;
		buf.resize(FLAC_READSIZE);
		while (r->readResize(buf)){
			/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
			for (int i=0;i<buf.length;i++){
				flac_pcm[i * 2 + 0] = (int)(buf.c[0][i] * scale);
				flac_pcm[i * 2 + 1] = (int)(buf.c[1][i] * scale);
			}
			/* feed samples to encoder */
			if (!FLAC__stream_encoder_process_interleaved(encoder, flac_pcm, buf.length))
				throw string("error while encoding. State: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)];
		}

		if (!FLAC__stream_encoder_finish(encoder))
			throw string("error while encoding. State: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)];

	}catch(string &s){
		od->error(s);
	}

	// now that encoding is finished, the metadata can be freed
	if (metadata)
		FLAC__metadata_object_delete(metadata);

	if (encoder)
		FLAC__stream_encoder_delete(encoder);
}

#endif
