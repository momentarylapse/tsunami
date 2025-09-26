/*
 * xml.cpp
 *
 *  Created on: Jan 15, 2018
 *      Author: michi
 */

#include "xml.h"
#include "../base/error.h"
#include "../os/file.h"
#include "../os/msg.h"
#include "lib/base/optional.h"

namespace xml{

int nn = 0;
const int NMAX = 1000000;


class EndOfFile {};

string encode_attribute(const string& s) {
	return s.replace("&", "&amp;").replace("\"", "&quot;");
}
string decode_text(const string& s) {
	return s.replace("&quot;", "\"").replace("&apos;", "\'").replace("&gt;", ">").replace("&lt;", "<").replace("&amp;", "&");
}

bool skip_until_char(Stream *f, char c) {
	while (!f->is_end()) {
		char cc;
		f->read(&cc, 1);
		if (cc == c)
			return true;
	}
	return false;
}

bool is_whitespace(char c) {
	return (c == ' ' or c == '\t' or c == '\n' or c == '\r');
}

base::optional<char> skip_whitespace(Stream *f, bool back = true) {
	while (!f->is_end()) {
		char c;
		f->read(&c, 1);
		if (is_whitespace(c))
			continue;
		if (back)
			f->seek(-1);
		return c;
	}
	return base::None;
}

base::optional<char> next_non_whitespace(Stream *f) {
	return skip_whitespace(f, false);
}

base::expected<string, Error> read_next_exp(Stream *f) {
	string e;
	auto cc=  next_non_whitespace(f);
	if (!cc.has_value())
		return Error::EndOfFile;
	char c0 = *cc;
	if (c0 == '=')
		return string("=");
	if (c0 == '>')
		return string(">");
	if (c0 == '<')
		return string("<");
	if (c0 == '/')
		return string("/");
	if (c0 == '?')
		return string("?");
	bool in_string = (c0 == '\"') or (c0 == '\'');
	if (in_string) {
		while (!f->is_end()) {
			char c;
			f->read(&c, 1);
			if (c == c0)
				return e;
			e.add(c);
		}
		return Error::EndOfFile;
	}

	e.add(c0);
	while (!f->is_end()) {
		char c;
		f->read(&c, 1);
		if (is_whitespace(c) or (c == '=') or (c == '>') or (c == '<')) {
			f->seek(-1);
			return e;
		}
		e.add(c);
	}
	return Error::EndOfFile;
}

Element::Element(const string &_tag, const string &_text) {
	tag = _tag;
	text = _text;
}

void Element::add_attribute(const string &key, const string &value) {
	Attribute a;
	a.key = key;
	a.value = value;
	attributes.add(a);
}

Element Element::with(const string &tag, const string &text) {
	add(Element(tag, text));
	return *this;
}

Element Element::witha(const string &key, const string &value) {
	add_attribute(key, value);
	return *this;
}

void Element::add(const Element &child) {
	elements.add(child);
}

Element dummy_element;

Element* Element::find(const string &tag) {
	for (auto &e: elements)
		if (e.tag == tag)
			return &e;
	return &dummy_element;
}

string Element::value(const string &key, const string &def) const {
	for (auto &a: attributes)
		if (a.key == key)
			return a.value;
	for (auto &e: elements)
		if (e.tag == key)
			return e.text;
	return def;
}

Error Parser::load(const Path &filename) {
	auto *f = os::fs::open(filename, "rb");

	while(true) {
		auto e = read_element(f);
		if (e.has_value()) {
			if ((e->tag != "!--") and (e->tag != "!DOCTYPE") and (e->tag != "?xml"))
				elements.add(*e);
		} else {
			//return e.error();
			break;
		}
	}
	return Error::None;
}

void Parser::write_element(Stream *f, Element &e, int indent) {
	for (int i=0; i<indent; i++)
		f->write("\t");

	f->write("<" + e.tag);
	for (auto &a: e.attributes)
		f->write(" " + a.key + "=\"" + encode_attribute(a.value) + "\"");

	if (e.text.num + e.elements.num == 0) {
		f->write(" />\n");
	} else if (e.elements.num > 0) {
		f->write(">\n");

		for (Element &child: e.elements)
			write_element(f, child, indent + 1);

		for (int i=0; i<indent; i++)
			f->write("\t");
		f->write("</" + e.tag + ">\n");
	} else if (e.elements.num == 0) {
		f->write(">" + e.text + "</" + e.tag + ">\n");
	}
}

void Parser::save(const Path &filename) {
	auto *f = os::fs::open(filename, "wb");
	f->write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	// should be exactly one root!
	for (Element &e: elements)
		write_element(f, e, 0);

	delete f;
}

void Parser::show() {
	msg_write("-------");
	msg_write(elements.num);
	for (Element &e: elements) {
		show_element(e, "");
	}
}

void Parser::show_element(Element &e, const string &pre) {
	msg_write(pre + e.tag);
	if (e.text.num > 0)
		msg_write(pre + " content: " + e.text);
	for (Attribute &a: e.attributes)
		msg_write(pre + " " + a.key + " = '" + a.value + "'");
	for (Element &c: e.elements)
		show_element(c, pre + "    ");
}

bool skip_recursive(Stream *f) {
	int level = 0;
	while(!f->is_end()) {
		char c;
		f->read(&c, 1);
		if (c == '<') {
			level += 1;
			//msg_write("<<");
		} else if (c == '>') {
			//msg_write(">>");
			level -= 1;
		}//else
			//msg_write(string(&c, 1));
		if (level < 0)
			return true;
	}
	return false;
}

base::expected<Element, Error> Parser::read_element(Stream *f) {
	//msg_write("<< element");
	//msg_right();
	auto e = read_tag(f);
	if (!e.has_value())
		return e;
	//msg_write(e->tag);
	if (e->single or e->closing) {
		/*msg_write("-----single/closing");
		msg_left();
		msg_write(">> element");*/
		return e;
	}
	if (nn > NMAX)
		return e;
	//msg_write("content....");
	while (!f->is_end()) {
		char c;
		f->read(&c, 1);
		if (c == '<') {
			f->seek(-1);
			break;
		}
		e->text.add(c);
	}
	e->text = e->text.trim();
	while (!f->is_end()) {
		auto ee = read_element(f);
		if (!ee.has_value())
			return ee;
		if (ee->closing and (ee->tag == e->tag)) {
			//msg_left();
			//msg_write(">> element (with closing) " + e->tag);
			return e;
		}
		//msg_write(">>>>  child");
		if (ee->tag != "!--")
			e->elements.add(*ee);
		if (nn > NMAX)
			return e;
	}
	//msg_left();
	//msg_write(">> element " + e->tag);
	return e;
}

base::expected<Element, Error> Parser::read_tag(Stream *f) {
	nn ++;
	//msg_write("--tag");
	Element e;
	e.single = false;
	e.closing = false;
	if (!skip_until_char(f, '<'))
		return Error::EndOfFile;
	auto ee = read_next_exp(f);
	if (!ee.has_value())
		return ee.error();
	e.tag = *ee;
	if (e.tag == "?") {
		auto x = read_next_exp(f);
		if (!x.has_value())
			return x.error();
		e.tag += *x;
		e.single = true;
	}
	if (e.tag == "/") {
		e.closing = true;
		auto x = read_next_exp(f);
		if (!x.has_value())
			return x.error();
		e.tag = *x;
	}
	//msg_write("             tag: " + (e.closing ? string("/") : string("")) + e.tag);

	if (e.tag == "!--") {
		e.single = true;
		return e;
	}

	if ((e.tag == "!ELEMENT") or (e.tag == "!DOCTYPE")) {
		if (!skip_recursive(f))
			return Error::EndOfFile;
		e.single = true;
		return e;
	}

	//msg_write("aaa");

	// attributes
	while (!f->is_end()) {
		auto s = read_next_exp(f);
		if (!s.has_value())
			return s.error();
		//msg_write(s);
		if (*s == "?")
			continue;
		if (*s == ">") {
			//msg_write("-end");
			return e;
		}

		// />
		if (*s == "/") {
			s = read_next_exp(f);
			if (!s.has_value())
				return s.error();
			if (*s != ">")
				return Error::SyntaxError;
			e.single = true;
			return e;
		}

		//msg_write("attr...");
		Attribute a;
		a.key = *s;
		s = read_next_exp(f);
		if (!s.has_value())
			return s.error();
		if (*s != "=")
			return Error::SyntaxError;
		s = read_next_exp(f);
		if (!s.has_value())
			return s.error();
		a.value = decode_text(*s);
		e.attributes.add(a);
	}

	//skip_until_char(f, '>');
	return Error::EndOfFile;
}


}



