#include "hui.h"
#include "../base/map.h"


// file dialogs
string HuiFilename;
int HuiColor[4];

string HuiFontname;



// additional application properties
static Map<string, string> HuiProperties;
Array<string> HuiPropAuthors;



void HuiSetProperty(const string &name, const string &value)
{
	if (name == "author")
		HuiPropAuthors.add(value);
	else
		HuiProperties[name] = value;
}

string HuiGetProperty(const string &name)
{
	return HuiProperties[name];
}


