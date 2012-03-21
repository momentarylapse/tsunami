#include "hui.h"


// file dialogs
string HuiFilename;
int HuiColor[4];



// additional application properties
string HuiPropName;
string HuiPropVersion;
string HuiPropComment;
string HuiPropLogo;
string HuiPropCopyright;
string HuiPropWebsite;
string HuiPropLicense;
Array<string> HuiPropAuthors;



void HuiSetProperty(const string &name, const string &value)
{
	if (name == "name")
		HuiPropName = value;
	else if (name == "version")
		HuiPropVersion = value;
	else if (name == "comment")
		HuiPropComment = value;
	else if (name == "logo")
		HuiPropLogo = value;
	else if (name == "copyright")
		HuiPropCopyright = value;
	else if (name == "website")
		HuiPropWebsite = value;
	else if (name == "license")
		HuiPropLicense = value;
	else if (name == "author")
		HuiPropAuthors.add(value);
}


