/*
 * ChannelMapperDialog.h
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_CHANNELMAPPERDIALOG_H_
#define SRC_VIEW_DIALOG_CHANNELMAPPERDIALOG_H_

#include "../../lib/hui/Window.h"

namespace tsunami {

class AudioChannelSelector;

class ChannelMapDialog : public hui::Dialog {
public:
	ChannelMapDialog(hui::Panel *parent, AudioChannelSelector *selector);
};

}

#endif /* SRC_VIEW_DIALOG_CHANNELMAPPERDIALOG_H_ */
