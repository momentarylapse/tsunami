/*
 * FormatOgg.cpp
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */



#include "FormatOgg.h"

#include "../../Module/Port/AudioPort.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/Audio/AudioBuffer.h"

#if HAS_LIB_OGG
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>



string tag_from_vorbis(const string &key)
{
	if (key == "TRACKNUMBER")
		return "track";
	/*if (key == "DATE")
		return "year";*/
	return key.lower();
}

string tag_to_vorbis(const string &key)
{
	return key.upper();
}


OggVorbis_File vf;

FormatDescriptorOgg::FormatDescriptorOgg() :
	FormatDescriptor("Ogg vorbis", "ogg", Flag::AUDIO | Flag::SINGLE_TRACK | Flag::TAGS | Flag::READ | Flag::WRITE)
{
}



int oe_write_page(ogg_page *page, FILE *fp)
{
    int written;
    written = fwrite(page->header,1,page->header_len, fp);
    written += fwrite(page->body,1,page->body_len, fp);

    return written;
}


void FormatOgg::save_via_renderer(StorageOperationData *od)
{
	AudioPort *r = od->renderer;

	float OggQuality = hui::Config.get_float("OggQuality", 0.5f);

	FILE *f = fopen(od->filename.c_str(), "wb");
	if (!f){
		od->error("can not create file");
		return;
	}

	vorbis_info vi;
	vorbis_info_init(&vi);
	int channels = od->channels_suggested;
	if (vorbis_encode_setup_vbr(&vi, channels, od->session->sample_rate(), OggQuality)){
		od->error("vorbis_encode_setup_vbr");
		return;
	}
	vorbis_encode_setup_init(&vi); // ?

	vorbis_dsp_state vd;
	vorbis_block vb;
	ogg_stream_state os;
	if (vorbis_analysis_init(&vd, &vi)){
		od->error("vorbis_analysis_init");
		return;
	}
	if (vorbis_block_init(&vd, &vb)){
		od->error("vorbis_block_init");
		return;
	}
	if (ogg_stream_init(&os, 0)){
		od->error("ogg_stream_init");
		return;
	}

	vorbis_comment vc;
	vorbis_comment_init(&vc);
	for (Tag &tag : od->tags)
		vorbis_comment_add_tag(&vc, tag.key.c_str(), tag.value.c_str());
	ogg_packet header_main;
	ogg_packet header_comments;
	ogg_packet header_codebooks;
	vorbis_analysis_headerout(&vd, &vc, &header_main, &header_comments, &header_codebooks);
	ogg_stream_packetin(&os, &header_main);
	ogg_stream_packetin(&os,&header_comments);
	ogg_stream_packetin(&os,&header_codebooks);
	ogg_page og;
	ogg_packet op;

	int result;
	while((result = ogg_stream_flush(&os, &og))){
		if (!result)
			break;
		int ret = oe_write_page(&og, f);
		if (ret != og.header_len + og.body_len){
			/*opt->error(_("Failed writing header to output stream\n"));
			ret = 1;
			goto cleanup;*/
			msg_error("ssss");
			return;
		}
	}

//#if 1
	//int eos = 0;
	int written = 0;
	int samples = od->num_samples;
#define READSIZE		1<<12
	int nn = 0;

	AudioBuffer buf(READSIZE, channels);

	int eos = 0;
	while(!eos){
		//msg_write(written);

		int samples_read = r->read(buf);
		if (samples_read <= 0)
			break;

		float **buffer = vorbis_analysis_buffer(&vd, samples_read);
		for (int c=0; c<channels; c++)
			for (int i=0;i<samples_read;i++)
				buffer[c][i] = buf.c[c][i];

		written += samples_read;
		vorbis_analysis_wrote(&vd, samples_read);


		nn ++;
		if (nn > 8){
			od->set(float(written) / (float)samples);
			nn = 0;
		}

        /* While we can get enough data from the library to analyse, one
           block at a time... */
        while (vorbis_analysis_blockout(&vd,&vb) == 1){
				//msg_write("b");

            /* Do the main analysis, creating a packet */
            vorbis_analysis(&vb, nullptr);
            vorbis_bitrate_addblock(&vb);

            while (vorbis_bitrate_flushpacket(&vd, &op)){
				//msg_write("f");
                /* Add packet to bitstream */
                ogg_stream_packetin(&os,&op);
                //packetsdone++;

                /* If we've gone over a page boundary, we can do actual output,
                   so do so (for however many pages are available) */

				//eos = 0;
                while( !eos){
				//msg_write("p");
                    int result = ogg_stream_pageout(&os,&og);
                    if (!result)
						break;

                    int ret = oe_write_page(&og, f);
                    if (ret != og.header_len + og.body_len){
                        od->error("Failed writing data to output stream");
                        ret = 1;
                        return;
                    }
                    //else
                      //  bytes_written += ret;

                    if (ogg_page_eos(&og))
                        eos = 1;
                }
            }
        }
	}

	ogg_stream_flush(&os, &og);
	ogg_stream_clear(&os);
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_info_clear(&vi);
	fclose(f);
}


void FormatOgg::load_track(StorageOperationData *od)
{
	Track *t = od->track;

	if (ov_fopen((char*)od->filename.c_str(), &vf)){
		od->error("can not open file");
		return;
	}
	vorbis_info *vi = ov_info(&vf, -1);
	//int bits = 16;
	int channels = 2;
	int freq = DEFAULT_SAMPLE_RATE;
	if (vi){
		channels = vi->channels;
		freq = vi->rate;
	}
	od->suggest_samplerate(freq);
	od->suggest_channels(channels);

	// tags
	t->song->tags.clear();
	char **ptr = ov_comment(&vf,-1)->user_comments;
	while (*ptr){
		string s = *ptr;
		int offset = s.find("=");
		if (offset > 0)
			t->song->tags.add(Tag(tag_from_vorbis(s.head(offset)), s.substr(offset + 1, -1)));
		++ptr;
    }

	int samples = (int)ov_pcm_total(&vf, -1);
	if (od->only_load_metadata){
		ov_clear(&vf);
		t->add_marker(Range(0, samples), "dummy");
		return;
	}

	char *data = new char[CHUNK_SIZE];
	int current_section;
	int read = 0;
	int nn = 0;

	while (true){

		int chunk_read = 0;
		bool error = false;
		while (true){
			int toread = CHUNK_SIZE - chunk_read;
			int r = ov_read(&vf, &data[chunk_read], toread, 0, 2, 1, &current_section); // 0,2,1 = little endian, 16bit, signed
			//msg_write(r);

			if (r == 0){
				break;
			}else if (r < 0){
				od->error("ov_read failed");
				error = true;
				break;
			}
			chunk_read += r;
			if (chunk_read >= CHUNK_SIZE - 1024)
				break;
		}

		if ((error) or (chunk_read == 0))
			break;

		int bytes_per_sample = 2 * channels;
		int dsamples = chunk_read / bytes_per_sample;
		int _offset = read / bytes_per_sample + od->offset;
		import_data(od->layer, data, channels, SampleFormat::SAMPLE_FORMAT_16, dsamples, _offset);
		read += chunk_read;
		nn ++;
		if (nn > 8){
			od->set((float)read / (float)(samples * 4));
			nn = 0;
		}
	}
	delete[](data);
	ov_clear(&vf);
}

#endif
