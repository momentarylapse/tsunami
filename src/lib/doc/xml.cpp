/*
 * xml.cpp
 *
 *  Created on: Jan 15, 2018
 *      Author: michi
 */

#include "xml.h"
#include "../os/file.h"
#include "../os/msg.h"

namespace xml{

int nn = 0;
const int NMAX = 1000000;

SyntaxError::SyntaxError() : Exception("xml syntax error") {}


class EndOfFile {};

void skip_until_char(Stream *f, char c) {
	while (!f->is_end()) {
		char cc;
		f->read(&cc, 1);
		if (cc == c)
			return;
	}
	throw EndOfFile();
}

bool is_whitespace(char c) {
	return (c == ' ' or c == '\t' or c == '\n' or c == '\r');
}

char skip_whitespace(Stream *f, bool back = true) {
	while (!f->is_end()) {
		char c;
		f->read(&c, 1);
		if (is_whitespace(c))
			continue;
		if (back)
			f->seek(-1);
		return c;
	}
	throw EndOfFile();
	return ' ';
}

char next_non_whitespace(Stream *f) {
	return skip_whitespace(f, false);
}

string read_next_exp(Stream *f) {
	string e;
	char c0 = next_non_whitespace(f);
	if (c0 == '=')
		return "=";
	if (c0 == '>')
		return ">";
	if (c0 == '<')
		return "<";
	if (c0 == '/')
		return "/";
	if (c0 == '?')
		return "?";
	bool in_string = (c0 == '\"') or (c0 == '\'');
	if (in_string) {
		while (!f->is_end()) {
			char c;
			f->read(&c, 1);
			if (c == c0)
				return e;
			e.add(c);
		}
		throw EndOfFile();
		return e;
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
	throw EndOfFile();
	return e;
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

string Element::value(const string &key, const string &def) {
	for (auto &a: attributes)
		if (a.key == key)
			return a.value;
	for (auto &e: elements)
		if (e.tag == key)
			return e.text;
	return def;
}

void Parser::load(const Path &filename) {
	auto *f = os::fs::open(filename, "rb");

	while(true) {
		try{
			Element e = read_element(f);
			//msg_write(e.tag);
			if ((e.tag != "!--") and (e.tag != "!DOCTYPE") and (e.tag != "?xml"))
				elements.add(e);
		}catch (EndOfFile &eof) {
			//msg_write("eof");

			//show();
			return;
		}
	}
}

void Parser::write_element(Stream *f, Element &e, int indent) {
	for (int i=0; i<indent; i++)
		f->write("\t");

	f->write("<" + e.tag);
	for (auto &a: e.attributes)
		f->write(" " + a.key + "=\"" + a.value + "\"");

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

void skip_recursive(Stream *f) {
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
			return;
	}
	throw EndOfFile();
}

Element Parser::read_element(Stream *f) {
	//msg_write("<< element");
	Element e;
	e = read_tag(f);
	if (e.single or e.closing) {
		//msg_write("-----single/cl");
		//msg_write(">> element");
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
		e.text.add(c);
	}
	e.text = e.text.trim();
	while (!f->is_end()) {
		Element ee = read_element(f);
		if (ee.closing and (ee.tag == e.tag)) {
			//msg_write(">> element (with closing)");
			return e;
		}
		//msg_write(">>>>  child");
		if (ee.tag != "!--")
			e.elements.add(ee);
		if (nn > NMAX)
			return e;
	}
	//msg_write(">> element");
	return e;
}

Element Parser::read_tag(Stream *f) {
	nn ++;
	//msg_write("--tag");
	Element e;
	e.single = false;
	e.closing = false;
	skip_until_char(f, '<');
	e.tag = read_next_exp(f);
	if (e.tag == "?") {
		e.tag += read_next_exp(f);
		e.single = true;
	}
	if (e.tag == "/") {
		e.closing = true;
		e.tag = read_next_exp(f);
	}
	//msg_write("             tag: " + (e.closing ? string("/") : string("")) + e.tag);

	if (e.tag == "!--") {
		e.single = true;
		return e;
	}

	if ((e.tag == "!ELEMENT") or (e.tag == "!DOCTYPE")) {
		skip_recursive(f);
		e.single = true;
		return e;
	}

	//msg_write("aaa");

	// attributes
	while (!f->is_end()) {
		string s = read_next_exp(f);
		//msg_write(s);
		if (s == "?")
			continue;
		if (s == ">") {
			//msg_write("-end");
			return e;
		}

		// />
		if (s == "/") {
			s = read_next_exp(f);
			if (s != ">")
				throw SyntaxError();
			e.single = true;
			return e;
		}

		//msg_write("attr...");
		Attribute a;
		a.key = s;
		s = read_next_exp(f);
		if (s != "=")
			throw SyntaxError();
		a.value = read_next_exp(f);
		e.attributes.add(a);
	}

	//skip_until_char(f, '>');
	throw EndOfFile();
	return e;
}


}



