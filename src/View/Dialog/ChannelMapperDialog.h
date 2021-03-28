/*
 * ChannelMapperDialog.h
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_CHANNELMAPPERDIALOG_H_
#define SRC_VIEW_DIALOG_CHANNELMAPPERDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../lib/base/pointer.h"

class PeakMeter;
class PeakMeterDisplay;



class ChannelMapDialog : public hui::Dialog {
public:
	int num_in, num_out;
	Array<int> &map;
	owned<PeakMeterDisplay> peak_meter_display_in;
	owned<PeakMeterDisplay> peak_meter_display_out;
	PeakMeter *peak_meter;

	ChannelMapDialog(hui::Panel *parent, int n_in, int n_out, Array<int> &map, PeakMeter *pm);
};

#endif /* SRC_VIEW_DIALOG_CHANNELMAPPERDIALOG_H_ */
