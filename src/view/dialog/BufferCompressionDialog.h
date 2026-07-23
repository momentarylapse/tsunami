/*
 * BufferCompressionDialog.h
 *
 *  Created on: Apr 22, 2021
 *      Author: michi
 */

#pragma once

#include "../../lib/hui/Window.h"

namespace tsunami {

struct Song;
struct Range;

class BufferCompressionDialog : public hui::Dialog {
public:
	string codec;
	float quality;

	BufferCompressionDialog(hui::Window *parent);
};

}
