#include "ttf.h"
#include "../base/iter.h"
#include "../os/path.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "../os/msg.h"

using namespace os::fs;

namespace ttf {

Array<Path> font_paths = {"./"};


int TTF::BEUShort::_int() const {
	unsigned int a = c[0];
	unsigned int b = c[1];
	return (a << 8) + b;
}

int TTF::BESShort::_int() const {
	unsigned int a = c[0];
	unsigned int b = c[1];
	unsigned int r = (a << 8) + b;
	if ((r & 0x8000) != 0)
		r -= 1<<16;
	return r;
}

int TTF::BELong::_int() const {
    unsigned int a = x[0];
    unsigned int b = x[1];
    unsigned int c = x[2];
    unsigned int d = x[3];
    return (a << 24) + (b << 16) + (c << 8) + d;
}

int TTF::readUS(FileStream *f) {
    BEUShort t;
    f->read(&t, 2);
    return t._int();
}

int TTF::readSS(FileStream *f) {
    BESShort t;
    f->read(&t, 2);
    return t._int();
}

int TTF::readUB(FileStream *f) {
    int t = 0;
    f->read(&t, 1);
    return t;
}

int TTF::readL(FileStream *f) {
    BELong t;
    f->read(&t, 4);
    return t._int();
}

string TTF::TableDirectoryEntry::name() {
    string s;
    s.add(tag.x[0]);
    s.add(tag.x[1]);
    s.add(tag.x[2]);
    s.add(tag.x[3]);
    return s;
}

int TTF::MapTable::map(int code) {
    if (format == 4) {
        for (int i=0; i<end_code.num; i++)
            if (code >= start_code[i] and code <= end_code[i])
                return (code + id_delta[i]) & 0x0000ffff;
        return -1;
    } else if (format == 6) {
        if (code < first_code or code >= (first_code + glyph_id_array.num))
            return -1;
        return glyph_id_array[code - first_code];
    }
    return -1;
}

TTF::TableDirectoryEntry* TTF::get_table(const string &tag) {
    for (auto &ee: tdentries)
        if (tag == ee.name())
            return &ee;
    return nullptr;
}

void TTF::read_table_directory(FileStream *f) {
	f->read(&td, 12);
	int n = td.num_tables._int();
	if (n > 1000)
		throw Exception("argh");
	for (int i=0; i<n; i++) {
		TableDirectoryEntry e;
		f->read(&e, 16);
		tdentries.add(e);
	}

	//for (auto &ee: tdentries)
	//	msg_write(format("%s  %d", ee.name(), ee.offset._int()));
}

bool TTF::read_head(FileStream *f) {
	auto te = get_table("head");
	if (!te) {
		msg_error("no head table");
		return false;
	}
	f->set_pos(te->offset._int());
	//msg_write("head-----------------------------");

	f->read(&head, 32);
	//msg_write(head.units_per_em._int());
	return true;
}

bool TTF::try_read_hhead(FileStream *f) {
    auto te = get_table("hhea");
    if (!te)
        return false;
    f->set_pos(te->offset._int());
    //msg_write("hhead-----------------------------");

    f->seek(34);
    int n = readUS(f);
    //msg_write(n);
    lsb.resize(n);
    advance.resize(n);
    return true;
}

bool TTF::try_read_hmetrix(FileStream *f) {
    auto te = get_table("hmtx");
    if (!te)
        return false;
    f->set_pos(te->offset._int());
    //msg_write("hmetrix-----------------------------");
    for (int i=0; i<lsb.num; i++) {
        advance[i] = readUS(f);
        lsb[i] = readSS(f);
    }
    //msg_write(ia2s(advance));
    return true;
}


bool TTF::read_mapping(FileStream *f) {
    auto te = get_table("cmap");
    if (!te) {
        msg_error("no cmap table");
        return false;
    }
    f->set_pos(te->offset._int());

    f->read(&mh, 4);
    int n = mh.num_tables._int();
    //msg_write("map-----------------------------");
    //msg_write(i2h(f->get_pos(), 4));
    //msg_write mh.version.int();
    //msg_write n;
    if (n > 100000) {
        msg_error("too many mapping tables");
        return false;
    }

    for (int i=0; i<n; i++) {
        MapTable t;
        t.platform = readUS(f);
        t.encoding = readUS(f);
        t.offset = readL(f);
        map_tables.add(t);
    }

    for (auto &m: map_tables) {
        /*msg_write("--------------");
        msg_write(m.offset);
        msg_write(m.platform);
        msg_write(m.encoding);*/
        f->set_pos(te->offset._int() + m.offset);
        m.format = readUS(f);

        if (m.format == 0) {
        	[[maybe_unused]] int length = readUS(f);
            //msg_write("l {{length}}");
        	[[maybe_unused]] int version = readUS(f);
            for (int i=0; i<256; i++)
                m.glyph_id_array.add(readUB(f));
            //msg_write m.glyph_id_array;
        } else if (m.format == 4) {
            int length = readUS(f);
            //msg_write("l {{length}}");
            [[maybe_unused]] int version = readUS(f);
            int seg_count = readUS(f) / 2;
            //msg_write("s {{seg_count}}");
            [[maybe_unused]] int search_range = readUS(f);
            [[maybe_unused]] int entry_selector = readUS(f);
            [[maybe_unused]] int range_shift = readUS(f);
            for (int i=0; i<seg_count; i++)
                m.end_code.add(readUS(f));
            [[maybe_unused]] int rr = readUS(f);
            for (int i=0; i<seg_count; i++)
                m.start_code.add(readUS(f));
            for (int i=0; i<seg_count; i++)
                m.id_delta.add(readUS(f));
            for (int i=0; i<seg_count; i++)
                m.id_range_offset.add(readUS(f));
            [[maybe_unused]] int rs = seg_count * 8 + 16;
            //msg_write("r: {{length - rs}}");
            int na = (length - rs) / 2;
            for (int i=0; i<na; i++)
                m.glyph_id_array.add(readUS(f));
        } else if (m.format == 6) {
        	[[maybe_unused]] int length = readUS(f);
            //msg_write("l {{length}}");
        	[[maybe_unused]] int version = readUS(f);
            m.first_code = readUS(f);
            int entry_count = readUS(f);
            for (int i=0; i<entry_count; i++)
                m.glyph_id_array.add(readUS(f));
            //msg_write m.glyph_id_array

        } else if (m.format == 12) {
            readUS(f); // "padding"
            [[maybe_unused]] int length = readL(f);
            //msg_write("l {{length}}");
            [[maybe_unused]] int version = readL(f);

            int group_count = readL(f) / 2;
            //msg_write("s {{group_count}}");

            for (int i=0; i<group_count; i++) {
                int start_code = readL(f);
                m.start_code.add(start_code);
                m.end_code.add(readL(f));
                m.id_delta.add(readL(f) - start_code);
            }

        } else {
            msg_error(format("unhandled format: %d", m.format));
            return false;
        }
    }


    return true;
}

void TTF::map_code(int index, int code) {
    if (index < glyphs.num) {
        glyphs[index].code = code;
        if (glyphs[index].codes.find(code) >= 0)
            glyphs[index].codes.add(code);
        //msg_write(format("E: %d => %d", index, code));
    }
}

void TTF::update_codes() {

    for (auto &m: map_tables) {
        if (m.format == 6) {
            //msg_write("----------------------- 666");
            for (auto&& [i,n]: enumerate(m.glyph_id_array))
                map_code(n, m.first_code + i);
        } else if (m.format == 4) {
            //msg_write("----------------------- 444");
            //msg_write(m.end_code.num);
            for (int i=0; i<m.end_code.num; i++) {
                /*msg_write("----");
                msg_write(m.start_code[i]);
                msg_write(m.end_code[i]);
                msg_write(m.id_delta[i]);*/
                for (int code=m.start_code[i]; code<m.end_code[i]+1; code++) {
                    int n;
                    if (m.id_range_offset[i] == 0) {
                        n = (code + m.id_delta[i]) & 0x0000ffff;
                    } else {
                        int index = (code - m.start_code[i]) + m.id_range_offset[i]/2 - (m.id_range_offset.num - i);
                        n = m.glyph_id_array[index];
                    }
                    map_code(n, code);
                }
            }
        } else if (m.format == 12) {
            //msg_write("----------------------- 12 12 12");
            //msg_write(m.end_code.num);
            for (int i=0; i<m.end_code.num; i++) {
                /*msg_write("----");
                msg_write(m.start_code[i]);
                msg_write(m.end_code[i]);
                msg_write(m.id_delta[i]);*/
                for (int code=m.start_code[i]; code<m.end_code[i]+1; code++)
                    map_code(code + m.id_delta[i], code);
            }
        } else {
            msg_error(format("---------------- UNHANDLED %d", m.format));
        }
    }
}


bool TTF::read_glyphs(FileStream *f) {
    auto te = get_table("glyf");
    if (!te) {
        msg_error("no glyf table");
        return false;
    }
    f->set_pos(te->offset._int());
    int _max = te->offset._int() + te->length._int() - 32;

    Glyph gg;
    glyphs.add(gg);
    glyphs.add(gg);
    glyphs.add(gg);

    /*while (f->get_pos() < _max) {
        Glyph g;
        if (!read_glyph(g))
            return false;
        glyphs.add(g);
    }*/
    glyphs.resize(_max + 3);
    //print("Glyphs: {{len(glyphs)}}");


    return true;
}

void TTF::update_size() {
	[[maybe_unused]] float scale = 1.0 / head.units_per_em._int();
    float w = 0;
    for (auto&& [i,g]: enumerate(glyphs)) {
        g.lsb = 0;
        g.width = g.xmax - g.xmin;
        if (i < lsb.num) {
            g.lsb = lsb[i];
            g.width = advance[i];
            w = g.width;
        } else if (lsb.num > 0) {
            g.width = w;
        }
    }
}

void TTF::load(const Path &filename) {
    auto f = os::fs::open(filename, "rb");

    read_table_directory(f);
    read_head(f);
    read_mapping(f);
    try_read_hhead(f);
    try_read_hmetrix(f);
    read_glyphs(f);

    update_codes();
    update_size();

    delete f;
}

int TTF::map(int code) {
    for (auto &m: map_tables) {
        int n = m.map(code);
        if (n >= glyphs.num) {
            msg_error("mapping error: {{code}} => {{n}}");
            return -1;
        }
        if (n >= 0)
            return n;
    }
    return -1;
}

TTF::Glyph* TTF::get(int code) {
    //print("get {{code}}")
    int n = map(code);
    //print(n);
    if (n < 0)
        return &glyphs[0];
    return &glyphs[n];
}

float TTF::scale() const {
    return 1000.0f / (float)head.units_per_em._int();
}

Array<int> TTF::get_widths() {
    float _scale = scale();
	Array<int> widths;
	for (int code=0; code<256; code++) {
		auto g = get(code);
		widths.add((int)(g->width * _scale));
	}
	return widths;
}

Path find_ttf(const string &name) {
	string fname = name + ".ttf";
	for (auto &dir: font_paths)
		if (os::fs::exists(dir | fname)) {
			return dir | fname;
		}
	return "";
}

void load_ttf(const string &name) {
	auto filename = find_ttf(name);
	if (!filename.is_empty()) {
		//msg_write(filename.str());
		TTF ttf;
		ttf.load(filename);
	} else {
		msg_error(format("ttf font not found: %s", name));
	}
}


void add_font_directory(const Path &dir) {
	font_paths.add(dir);
}


}

