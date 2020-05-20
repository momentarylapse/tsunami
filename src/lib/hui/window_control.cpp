#include "Controls/Control.h"
#include "hui.h"

namespace hui
{


//    for all
bool Panel::is_enabled(const string &id)
{
	bool r = false;;
	apply_foreach(id, [&](Control *c){ r = c->enabled; });
	return r;
}



//----------------------------------------------------------------------------------
// creating control items


/*string ScanOptions(int id, const string &title)
{
	string title2 = get_lang(id, title);
	HuiFormatString = "";
	hui_option_multiline = false;
	hui_option_alpha = false;
	hui_option_nobar = false;
	if ((title2.num > 0) && (title2[0] == '!')){
		for (int i=0;i<title2.num;i++){
			if (title2[i] == HuiComboBoxSeparator){
				OptionString = title2.substr(0, i);
				break;
			}
		}
		Array<string> opt = OptionString.substr(1, -1).explode(",");
		for (int i=0;i<opt.num;i++){
			hui_option_multiline |= (opt[i] == "multiline");
			hui_option_alpha |= (opt[i] == "alpha");
			hui_option_nobar |= (opt[i] == "nobar");
			if (opt[i].find("format=") >= 0){
				HuiFormatString = opt[i].substr(7, -1);
				//msg_write(HuiFormatString);
				// t-T-c-C-B-i
			}
		}
		return title2.substr(OptionString.num + 1, title2.num);
	}
	return title2;
}*/

Array<string> split_title(const string &title) {
	auto r = title.explode(ComboBoxSeparator);
//	msg_write(sa2s(PartString));
	if (r.num > 0)
		if (r[0].head(1) == "!") {
			r.erase(0);
				/*OptionString = PartString[0].substr(1, -1);
				PartString.erase(0);

				int a = OptionString.find("format=");
				if (a >= 0){
					HuiFormatString = OptionString.substr(a + 7, -1);
					int b = HuiFormatString.find(",");
					if (b >= 0)
						HuiFormatString = HuiFormatString.substr(0, b);
				}*/
			}
	if (r.num == 0)
		return {""};
	return r;
}


string get_option_from_title(const string &title) {
	auto r = title.explode(ComboBoxSeparator);
	if (r.num > 0)
		if (r[0].head(1) == "!")
			return r[0].substr(1, -1);
	return "";

}

bool option_has(const string &options, const string &key) {
	return sa_contains(options.explode(","), key);
}
string option_value(const string &options, const string &key) {
	auto r = options.explode(",");
	for (auto &x: r) {
		int p = x.find("=", 0);
		if (p >= 0)
			if (x.head(p) == key)
				return x.substr(p + 1, -1);
	}
	return "";
}

};
