/*
 * FormatFlac.cpp
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#include "FormatFlac.h"
#include "../../Tsunami.h"
#include "../../View/Helper/Progress.h"
#include "../../Stuff/Log.h"
#include <math.h>

#include <FLAC/all.h>

bool flac_tells_samples;
int flac_offset, flac_level;
int flac_channels, flac_bits, flac_samples, flac_freq, flac_file_size;
SampleFormat flac_format;
int flac_read_samples;
Track *flac_track;

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"


FLAC__StreamDecoderWriteStatus flac_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	if (frame->header.number.sample_number == 0)
		flac_read_samples = 0;

	if (frame->header.number.sample_number % 1024 == 0){
		if (flac_tells_samples)
			tsunami->progress->set((float)(flac_read_samples / flac_channels) / (float)(flac_samples));
		else // estimate... via increasingly compressed size
			tsunami->progress->set(( 1 - exp(- (float)(flac_read_samples * flac_channels * (flac_bits / 8)) / (float)flac_file_size ) ));
	}

	// read decoded PCM samples
	Range range = Range(flac_read_samples + flac_offset, frame->header.blocksize);
	BufferBox buf = flac_track->getBuffers(flac_level, range);
	Action *a = new ActionTrackEditBuffer(flac_track, 0, range);
	float scale = pow(2.0f, flac_bits);
	for (int i=0;i<(int)frame->header.blocksize;i++)
		for (int j=0;j<flac_channels;j++)
			if (j == 0)
				buf.r[i] = buffer[j][i] / scale;
			else
				buf.l[i] = buffer[j][i] / scale;
	flac_track->root->execute(a);

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
				flac_track->root->addTag(s.head(pos).lower(), s.tail(s.num - pos - 1));
		}
	}else{
		tsunami->log->warning("flac_metadata_callback: unhandled type: " + i2s(metadata->type));
	}
}

void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//msg_write("error");
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

FormatFlac::FormatFlac() :
	Format("Flac", "flac", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ | FLAG_WRITE)
{
}

FormatFlac::~FormatFlac()
{
}

void FormatFlac::loadTrack(Track *t, const string & filename, int offset, int level)
{
	msg_db_f("load_flac_file", 1);
	tsunami->progress->set(_("lade flac"), 0);
	t->root->action_manager->beginActionGroup();
	bool ok = true;

	flac_file_size = 1000000000;
	CFile *f = FileOpen(filename);
	if (f){
		flac_file_size = f->GetSize();
		FileClose(f);
	}

	flac_level = level;
	flac_offset = offset;
	flac_read_samples = 0;
	flac_track = t;
	//bits = channels = samples = freq = 0;

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder){
		tsunami->log->error("flac: decoder_new()");
	}

	FLAC__stream_decoder_set_metadata_respond(decoder, (FLAC__MetadataType)(FLAC__METADATA_TYPE_STREAMINFO | FLAC__METADATA_TYPE_VORBIS_COMMENT));

	FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_file(
							decoder,
							filename.c_str(),
							flac_write_callback,
							flac_metadata_callback,
							flac_error_callback, NULL);
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


	if (t->get_index() == 0){
		t->root->setSampleRate(flac_freq);
		t->root->setDefaultFormat(format_for_bits(flac_bits));
	}
	t->root->action_manager->endActionGroup();
}



#define FLAC_READSIZE 2048

void flac_progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
	long length = (long)client_data;
	if (samples_written % (FLAC_READSIZE * 64) == 0)
		tsunami->progress->set((float)samples_written / (float)length);
}

static FLAC__int32 flac_pcm[FLAC_READSIZE/*samples*/ * 2/*channels*/];



void FormatFlac::saveAudio(AudioFile *a, const string & filename)
{
	exportAudioAsTrack(a, filename);
}



void FormatFlac::saveBuffer(AudioFile *a, BufferBox *b, const string & filename)
{
	tsunami->progress->set(_("exportiere flac"), 0);

	bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;
	FLAC__StreamMetadata *metadata[1];
	FLAC__StreamMetadata_VorbisComment_Entry entry;

	int channels = 2;
	int bits = format_get_bits(a->default_format);
	if (bits > 24)
		bits = 24;

	// allocate the encoder
	FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
	if (!encoder){
		tsunami->log->error("flac: allocating encoder");
		return;
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bits);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, a->sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, b->num);

	// metadata
	if (ok){
		metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
		if (metadata[0]){
			foreach(Tag &t, a->tags){
				FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, t.key.upper().c_str(), t.value.c_str());
				FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, true);
			}
		}else{
			tsunami->log->error("flac: could not add metadata");
			ok = false;
		}

		ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 1);
	}

	// initialize encoder
	if (ok){
		init_status = FLAC__stream_encoder_init_file(encoder, filename.c_str(), flac_progress_callback, /*client_data=*/(void*)(long)b->num);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			tsunami->log->error(string("flac: initializing encoder: ") + FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok){
		int p0 = 0;
		size_t left = (size_t)b->num;
		float scale = pow(2.0f, bits);
		msg_write(f2s(scale, 3));
		while (ok && left){
			size_t need = (left>FLAC_READSIZE? (size_t)FLAC_READSIZE : (size_t)left);
			{
				/* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
				for (unsigned int i=0;i<need;i++){
					flac_pcm[i * 2 + 0] = (int)(b->r[p0 + i] * scale);
					flac_pcm[i * 2 + 1] = (int)(b->l[p0 + i] * scale);
				}
				/* feed samples to encoder */
				ok = FLAC__stream_encoder_process_interleaved(encoder, flac_pcm, need);
			}
			left -= need;
			p0 += FLAC_READSIZE;
		}
	}

	ok &= FLAC__stream_encoder_finish(encoder);

	if (!ok){
		tsunami->log->error("flac: encoding: FAILED");
		tsunami->log->error(string("   state: ") + FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
	}

	// now that encoding is finished, the metadata can be freed
	FLAC__metadata_object_delete(metadata[0]);

	FLAC__stream_encoder_delete(encoder);
}



void FormatFlac::loadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO, 0);
	loadTrack(t, filename);
}


