/*
 * Diff.h
 *
 *  Created on: Sep 28, 2020
 *      Author: michi
 */

#ifndef SRC_STUFF_DIFF_H_
#define SRC_STUFF_DIFF_H_

#include "../lib/base/base.h"

class Song;
class Track;

Array<string> diff_song(Song *a, Song *b);

#endif /* SRC_STUFF_DIFF_H_ */
