#include "hui.h"
#include "../base/map.h"

namespace hui
{

// file dialogs
string Filename;
int Color[4];

string Fontname;



// additional application properties
static Map<string, string> HuiProperties;
Array<string> PropAuthors;



void SetProperty(const string &name, const string &value)
{
	if (name == "author")
		PropAuthors.add(value);
	else
		HuiProperties[name] = value;
}

string GetProperty(const string &name)
{
	return HuiProperties[name];
}

};

