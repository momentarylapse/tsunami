/*
 * FormatFlac.cpp
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#include "FormatFlac.h"
#include "../Tsunami.h"
#include "../View/Helper/Progress.h"

#include <FLAC/all.h>

bool flac_tells_samples;
int flac_channels, flac_bits, flac_samples, flac_freq, flac_file_size;
int flac_read_samples;
Track *flac_track;

#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"


FLAC__StreamDecoderWriteStatus flac_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	if (frame->header.number.sample_number == 0)
		flac_read_samples = 0;

	if (frame->header.number.sample_number % 1024 == 0){
		if (flac_tells_samples)
			tsunami->progress->Set((float)(flac_read_samples / flac_channels) / (float)(flac_samples));
		else // estimate... via increasingly compressed size
			tsunami->progress->Set(( 1 - exp(- (float)(flac_read_samples * flac_channels * (flac_bits / 8)) / (float)flac_file_size ) ));
	}

	// read decoded PCM samples
	Range range = Range(flac_read_samples, frame->header.blocksize);
	BufferBox buf = flac_track->GetBuffers(0, range);
	Action *a = new ActionTrackEditBuffer(flac_track, 0, range);
	for (int i=0;i<(int)frame->header.blocksize;i++)
		for (int j=0;j<flac_channels;j++)
			if (j == 0)
				buf.r[i] = buffer[j][i] / 32768.0f;
			else
				buf.l[i] = buffer[j][i] / 32768.0f;
	flac_track->root->Execute(a);

	flac_read_samples += frame->header.blocksize;
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flac_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	msg_write("--  metadata  --");
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO){
		msg_write("stream info");
		flac_freq = metadata->data.stream_info.sample_rate;
		flac_bits = metadata->data.stream_info.bits_per_sample;
		flac_samples = metadata->data.stream_info.total_samples;
		flac_tells_samples = (flac_samples != 0);
		flac_channels = metadata->data.stream_info.channels;
		//printf("%d %d %d %d\n", flac_channels, flac_bits, (int)flac_samples, flac_freq);
	}else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT){
		msg_write("comment");
		for (int i=0;i<(int)metadata->data.vorbis_comment.num_comments;i++){
			msg_write((char*)metadata->data.vorbis_comment.comments[i].entry);
		}
	}
}

void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//msg_write("error");
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

FormatFlac::FormatFlac() :
	Format("flac", FLAG_SINGLE_TRACK | FLAG_TAGS)
{
}

FormatFlac::~FormatFlac()
{
}

void FormatFlac::LoadTrack(Track *t, const string & filename)
{
	msg_db_r("load_flac_file", 1);
	tsunami->progress->Set(_("lade flac"), 0);
	t->root->action_manager->BeginActionGroup();
	bool ok = true;

	flac_file_size = 1000000000;
	CFile *f = OpenFile(filename);
	if (f){
		flac_file_size = f->GetSize();
		FileClose(f);
	}

	flac_read_samples = 0;
	flac_track = t;
	//bits = channels = samples = freq = 0;

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder){
		msg_error("flac: decoder_new()");
	}

	FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_file(
							decoder,
							filename.c_str(),
							flac_write_callback,
							flac_metadata_callback,
							flac_error_callback, NULL);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK){
		fprintf(stderr, "ERROR: initializing decoder: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
		ok = false;
	}

	if (ok){
		ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
		fprintf(stderr, "decoding: %s\n", ok? "succeeded" : "FAILED");
		fprintf(stderr, "   state: %s\n", FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
	}
	FLAC__stream_decoder_delete(decoder);


	int bits = flac_bits;
	int channels = flac_channels;
	int freq = flac_freq;
	t->root->sample_rate = freq;
	t->root->action_manager->EndActionGroup();

	msg_db_l(1);
}



#define FLAC_READSIZE 1024

void flac_progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
	long length = (long)client_data;
	if (samples_written % (FLAC_READSIZE * 128) == 0)
		tsunami->progress->Set((float)samples_written / (float)length);
}

static FLAC__int32 flac_pcm[FLAC_READSIZE/*samples*/ * 2/*channels*/];



void FormatFlac::SaveAudio(AudioFile *a, const string & filename)
{
	ExportAudioAsTrack(a, filename);
}



void FormatFlac::SaveBuffer(AudioFile *a, BufferBox *b, const string & filename)
{
	tsunami->progress->Set(_("exportiere flac"), 0);

	bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;
	FLAC__StreamMetadata *metadata[2];
	FLAC__StreamMetadata_VorbisComment_Entry entry;

	int channels = 2;

	// allocate the encoder
	FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
	if (!encoder){
		fprintf(stderr, "ERROR: allocating encoder\n");
		return;
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, 16);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, a->sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, b->num);

	// now add some metadata; we'll add some tags and a padding block
	if (ok){
		if (
			(metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
			(metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
			/* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */
			!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "test") ||
			!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
			!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984") ||
			!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false)
		) {
			fprintf(stderr, "ERROR: out of memory or tag error\n");
			ok = false;
		}

		metadata[1]->length = 1234; /* set the padding length */

		ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);
	}

	// initialize encoder
	if (ok){
		init_status = FLAC__stream_encoder_init_file(encoder, filename.c_str(), flac_progress_callback, /*client_data=*/(void*)b->num);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok){
		int p0 = 0;
		size_t left = (size_t)b->num;
		while (ok && left){
			size_t need = (left>FLAC_READSIZE? (size_t)FLAC_READSIZE : (size_t)left);
			{
				/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
				for (unsigned int i=0;i<need;i++){
					flac_pcm[i * 2 + 0] = (int)(b->r[p0 + i] * 32768.0f);
					flac_pcm[i * 2 + 1] = (int)(b->l[p0 + i] * 32768.0f);
				}
				/* feed samples to encoder */
				ok = FLAC__stream_encoder_process_interleaved(encoder, flac_pcm, need);
			}
			left -= need;
			p0 += FLAC_READSIZE;
		}
	}

	ok &= FLAC__stream_encoder_finish(encoder);

	fprintf(stderr, "encoding: %s\n", ok? "succeeded" : "FAILED");
	fprintf(stderr, "   state: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);

	/* now that encoding is finished, the metadata can be freed */
//	FLAC__metadata_object_delete(metadata[0]);
//	FLAC__metadata_object_delete(metadata[1]);

	FLAC__stream_encoder_delete(encoder);
}



void FormatFlac::LoadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->AddEmptyTrack(0);
	LoadTrack(t, filename);
}


