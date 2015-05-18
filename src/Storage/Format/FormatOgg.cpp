/*
 * FormatOgg.cpp
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#include "FormatOgg.h"
#include "../../Tsunami.h"
#include "../../View/Helper/Progress.h"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>


#define _error(msg, level)	{msg_error(msg); msg_db_l(level); return;}

const int CHUNK_SIZE = 1 << 15;


OggVorbis_File vf;
char ogg_buffer[4096];

FormatOgg::FormatOgg() :
	Format("Ogg vorbis", "ogg", FLAG_AUDIO | FLAG_SINGLE_TRACK | FLAG_TAGS | FLAG_READ | FLAG_WRITE)
{
}

FormatOgg::~FormatOgg()
{
}

void FormatOgg::saveAudio(AudioFile *a, const string & filename)
{
	exportAudioAsTrack(a, filename);
}




int oe_write_page(ogg_page *page, FILE *fp)
{
    int written;
    written = fwrite(page->header,1,page->header_len, fp);
    written += fwrite(page->body,1,page->body_len, fp);

    return written;
}


void FormatOgg::saveBuffer(AudioFile *a, BufferBox *b, const string & filename)
{
	msg_db_r("write_ogg_file", 1);
	tsunami->progress->set(_("exportiere ogg"), 0);
	int size = b->num * 4;

	float OggQuality = HuiConfig.getFloat("OggQuality", 0.5f);

	FILE *f = fopen(filename.c_str(), "wb");

	vorbis_info vi;
	vorbis_info_init(&vi);
	if (vorbis_encode_setup_vbr(&vi, 2, a->sample_rate, OggQuality))
		_error("vorbis_encode_setup_vbr", 1);
	vorbis_encode_setup_init(&vi); // ?

	vorbis_dsp_state vd;
	vorbis_block vb;
	ogg_stream_state os;
	if (vorbis_analysis_init(&vd, &vi))
		_error("vorbis_analysis_init", 1);
	if (vorbis_block_init(&vd, &vb))
		_error("vorbis_block_init", 1);
	if (ogg_stream_init(&os, 0))
		_error("ogg_stream_init", 1);

	vorbis_comment vc;
	vorbis_comment_init(&vc);
	foreach(Tag &tag, a->tags)
		vorbis_comment_add_tag(&vc, (char*)tag.key.c_str(), (char*)tag.value.c_str());
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
        while((result = ogg_stream_flush(&os, &og)))
        {
            if(!result) break;
            int ret = oe_write_page(&og, f);
            if(ret != og.header_len + og.body_len)
            {
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
#define READSIZE		2048
	int nn = 0;

				int eos = 0;
	while(!eos){
		//msg_write(written);

		int num = READSIZE;
		if (num + written > b->num)
			num = b->num - written;
		float **buffer = vorbis_analysis_buffer(&vd, READSIZE);
		for (int i=0;i<num;i++){
			buffer[0][i] = b->r[written + i];
			buffer[1][i] = b->l[written + i];
		}
		written += num;
		vorbis_analysis_wrote(&vd, num);
		//msg_write(num);


		nn ++;
		if (nn > 8){
			tsunami->progress->set(float(written) / (float)b->num);
			nn = 0;
		}

        /* While we can get enough data from the library to analyse, one
           block at a time... */
        while(vorbis_analysis_blockout(&vd,&vb)==1)
        {
				//msg_write("b");

            /* Do the main analysis, creating a packet */
            vorbis_analysis(&vb, NULL);
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
                        msg_error("Failed writing data to output stream");
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

	ogg_stream_clear(&os);
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_info_clear(&vi);
	fclose(f);

	msg_db_l(1);
}



void FormatOgg::loadAudio(AudioFile *a, const string & filename)
{
	Track *t = a->addTrack(Track::TYPE_AUDIO);
	loadTrack(t, filename);
}



void FormatOgg::loadTrack(Track *t, const string & filename, int offset, int level)
{
	msg_db_f("Ogg.LoadTracl", 1);
	tsunami->progress->set(_("lade ogg"), 0);
	if (ov_fopen((char*)filename.c_str(), &vf)){
		msg_error("ogg: ov_fopen failed");
		return;
	}
	vorbis_info *vi = ov_info(&vf, -1);
	int bits = 16;
	int channels = 2;
	int freq = 44100;
	if (vi){
		channels = vi->channels;
		freq = vi->rate;
	}

	// tags
	t->root->tags.clear();
	char **ptr = ov_comment(&vf,-1)->user_comments;
	while(*ptr){
		string s = *ptr;
		if (s.find("=") > 0){
			Tag tag;
			tag.key = s.substr(0, s.find("=")).lower();
			tag.value = s.substr(s.find("=") + 1, -1);
			t->root->tags.add(tag);
		}
		++ptr;
    }

	int samples = (int)ov_pcm_total(&vf, -1);
	char *data = new char[CHUNK_SIZE];
	int current_section;
	int read = 0;
	int nn = 0;
	while(true){
		int toread = CHUNK_SIZE;
		int r = ov_read(&vf, data, toread, 0, 2, 1, &current_section); // 0,2,1 = little endian, 16bit, signed
		if (r == 0)
			break;
		else if (r < 0){
			msg_error("ogg: ov_read failed");
			break;
		}else{
			int dsamples = r / 4;
			int _offset = read / 4 + offset;
			importData(t, data, channels, SAMPLE_FORMAT_16, dsamples, _offset, level);
			read += r;
			nn ++;
			if (nn > 256){
				tsunami->progress->set((float)read / (float)(samples * 4));
				nn = 0;
			}
		}
	}
	delete[](data);
	ov_clear(&vf);
}


