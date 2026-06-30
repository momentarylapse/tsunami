/*
 * ttf.h
 *
 *  Created on: 06.11.2022
 *      Author: michi
 */

#ifndef SRC_LIB_DOC_TTF_H_
#define SRC_LIB_DOC_TTF_H_

#include "../base/base.h"
#include "../base/pointer.h"

namespace os::fs {
    class FileStream;
}
class Path;

namespace ttf {

extern Array<Path> font_paths;


class TTF : public Sharable<base::Empty> {
public:

	struct BEUShort {
		unsigned char c[2];
		int _int() const;
	};
	struct BESShort {
		unsigned char c[2];
		int _int() const;
	};
	struct BELong {
		unsigned char x[4];
		int _int() const;
	};

	struct Fixed {
		BEUShort a, b;
	};


	static int readUS(os::fs::FileStream *f);
	static int readSS(os::fs::FileStream *f);
	static int readUB(os::fs::FileStream *f);
	static int readL(os::fs::FileStream *f);
	struct TableDirectory {
		Fixed version;
		BEUShort num_tables;
		BEUShort seach_range;
		BEUShort entry_selector;
		BEUShort range_shift;
	};
	TableDirectory td;

	struct TableDirectoryEntry {
		BELong tag, chksum, offset, length;
		string name();
	};
	Array<TableDirectoryEntry> tdentries;

	struct Header {
		Fixed version;
		Fixed revision;
		BELong chksm;
		BELong magic;
		BEUShort flags;
		BEUShort units_per_em;
		// more
	};
	Header head;


	struct MapHeader {
		BEUShort version, num_tables;
	};
	MapHeader mh;

	struct MapTable {
		int platform, encoding;
		int offset;
		int format;

		// format 4
		Array<int> end_code;
		Array<int> start_code;
		Array<int> id_delta;
		Array<int> id_range_offset;

		// format 6
		int first_code;
		Array<int> glyph_id_array;

		int map(int code);
	};
	Array<MapTable> map_tables;

	struct Glyph {
		int code;
		int xmin, ymin, xmax, ymax;
		//Contour[] contours;
		Array<int> codes;
		float lsb, width;
	};
	Array<Glyph> glyphs;

	Array<int> lsb, advance;


	TableDirectoryEntry* get_table(const string &tag);
	void read_table_directory(os::fs::FileStream *f);
	bool read_head(os::fs::FileStream *f);
	bool try_read_hhead(os::fs::FileStream *f);
	bool try_read_hmetrix(os::fs::FileStream *f);
	bool read_mapping(os::fs::FileStream *f);
	void map_code(int index, int code);
	void update_codes();
	bool read_glyphs(os::fs::FileStream *f);
	void update_size();
	void load(const Path &filename);
	int map(int code);
	Glyph* get(int code);
    float scale() const;
	Array<int> get_widths();
};

Path find_ttf(const string &name);
void load_ttf(const string &name);


void add_font_directory(const Path &dir);

}


#endif /* SRC_LIB_DOC_TTF_H_ */
