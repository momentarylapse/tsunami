/*
 * BufferCompressionDialog.cpp
 *
 *  Created on: Apr 22, 2021
 *      Author: michi
 */

#include "BufferCompressionDialog.h"

const Array<string> COMPRESSION_CODECS = {"ogg", "flac"};

BufferCompressionDialog::BufferCompressionDialog(hui::Window *parent) : hui::Dialog("buffer-compression-dialog", parent) {
	quality = 0.5f;
	for (auto &c: COMPRESSION_CODECS)
		add_string("codec", c);
	set_int("codec", 0);

	event("codec", [=] {
		//enable("quality", get_int("codec") == 0);
	});
	event("ok", [this] {
		codec = COMPRESSION_CODECS[get_int("codec")];
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
}

