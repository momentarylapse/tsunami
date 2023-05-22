/*
 * BreadCrumps.h
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_BREADCRUMPS_H_
#define SRC_VIEW_HELPER_BREADCRUMPS_H_

namespace hui {
class Panel;
}
class Session;

namespace BreadCrumps {
	void add(hui::Panel* panel, Session *session);
}

#endif /* SRC_VIEW_HELPER_BREADCRUMPS_H_ */
