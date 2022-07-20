/*
 * xml.h
 *
 *  Created on: Jan 15, 2018
 *      Author: michi
 */

#ifndef SRC_LIB_XFILE_XML_H_
#define SRC_LIB_XFILE_XML_H_

#include "../base/base.h"

class Stream;
class Path;

namespace xml{

class SyntaxError : public Exception {
public:
	SyntaxError();
};

struct Attribute {
	string key, value;
};

struct Tag {
	string tag;
	Array<Attribute> attributes;
	bool single, closing;
};

struct Element : Tag {
	string text;
	Array<Element> elements;

	Element(){}
	Element(const string &tag, const string &text = "");
	void add_attribute(const string &key, const string &value);
	void add(const Element &child);
	Element with(const string &tag, const string &text = "");
	Element witha(const string &key, const string &value);
	Element* find(const string &tag);
	string value(const string &key, const string &def = "");
};

class Parser {
public:
	void _cdecl load(const Path &filename);

	Element read_element(Stream *f);
	Element read_tag(Stream *f);

	void _cdecl save(const Path &filename);
	void write_element(Stream *f, Element &e, int indent);

	Array<Element> elements;

	void show();
	void show_element(Element &e, const string &pre);
};

}


#endif /* SRC_LIB_XFILE_XML_H_ */
