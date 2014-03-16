#include "hui.h"
#include "hui_internal.h"
#include "Controls/HuiControl.h"
#include "../math/math.h"

//----------------------------------------------------------------------------------
// resource functions

string str_unescape(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++){
		if ((str[i]=='\\')&&(str[i+1]=='n')){
			r += "\n";
			i ++;
		}else if ((str[i]=='\\')&&(str[i+1]=='\\')){
			r += "\\";
			i++;
		}else if ((str[i]=='\\')&&(str[i+1]=='?')){
			r += "?";
			i++;
		}else if ((str[i]=='\\')&&(str[i+1]=='t')){
			r += "\t";
			i++;
		}else if ((str[i]=='\\')&&(str[i+1]=='"')){
			r += "\"";
			i++;
		}else
			r.add(str[i]);
	}
	return r;
}


string str_escape(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++){
		if (str[i] == '\t')
			r += "\\t";
		else if (str[i] == '\n')
			r += "\\n";
		else if (str[i] == '\\')
			r += "\\\\";
		else if (str[i] == '\"')
			r += "\\\"";
		else
			r.add(str[i]);
	}
	return r;
}

extern Array<HuiLanguage> _HuiLanguage_;
Array<HuiResource> _HuiResource_;

void LoadResourceCommand(CFile *f, HuiResource *c)
{
	c->type = f->ReadStr();
	c->id = f->ReadStr();
	c->image = f->ReadStr();
	c->enabled = f->ReadBool();
	c->i_param[0] = f->ReadInt();
	c->i_param[1] = f->ReadInt();
	c->i_param[2] = f->ReadInt();
	c->i_param[3] = f->ReadInt();
	c->i_param[4] = f->ReadInt();
	c->b_param[0] = f->ReadBool();
	c->b_param[1] = f->ReadBool();
	c->s_param[0] = f->ReadStr();
	c->s_param[1] = f->ReadStr();
}

void HuiLoadResource(const string &filename)
{
	msg_db_r("HuiLoadResource", 1);
	// dirty...
	_HuiResource_.clear();
	_HuiLanguage_.clear();

	CFile *f = FileOpen(filename);
	if (f){
		int ffv = f->ReadFileFormatVersion();
		int nres = f->ReadIntC();
		for (int i=0;i<nres;i++){
			HuiResource res;
			res.children.clear();
			f->ReadComment();
			LoadResourceCommand(f, &res);
			int n = f->ReadInt();
			for (int j=0;j<n;j++){
				HuiResource child;
				LoadResourceCommand(f, &child);
				res.children.add(child);
			}
			_HuiResource_.add(res);
		}

		// languages
		int nl = f->ReadIntC();
		for (int l=0;l<nl;l++){
			HuiLanguage hl;

			// Language
			hl.name = f->ReadStrC();

			//  NumIDs
			int n = f->ReadIntC();
			f->ReadComment(); // Text
			for (int i=0;i<n;i++){
				HuiLanguageCommand c;
				c.id = f->ReadStr();
				c.text = str_unescape(f->ReadStr());
				hl.cmd.add(c);
			}
			// Num Language Strings
			n = f->ReadIntC();
			// Text
			f->ReadComment();
			for (int i=0;i<n;i++){
				HuiLanguageTranslation s;
				s.orig = str_unescape(f->ReadStr());
				s.trans = str_unescape(f->ReadStr());
				hl.trans.add(s);
			}
			_HuiLanguage_.add(hl);
		}
		FileClose(f);
	}
}

HuiResource *HuiGetResource(const string &id)
{
	foreach(HuiResource &r, _HuiResource_)
		if (r.id == id)
			return &r;
	return NULL;
}

HuiWindow *HuiCreateResourceDialog(const string &id, HuiWindow *root)
{
	//return HuiCreateDialog("-dialog not found in resource-",200,100,root,true,mf);
	msg_db_f("HuiCreateResourceDialog",1);
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error(format("HuiCreateResourceDialog  (id=%s)  m(-_-)m",id.c_str()));
		return NULL;
	}
	
	msg_db_m("HuiResDialog",2);
	msg_db_m(i2s(res->i_param[0]).c_str(),2);
	msg_db_m(i2s(res->i_param[1]).c_str(),2);

	// dialog
	HuiWindow *dlg;
	if (res->type == "SizableDialog")
		dlg = new HuiDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);
	else
		dlg = new HuiFixedDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);

	// menu?
	if (res->s_param[0].num > 0)
		dlg->SetMenu(HuiCreateResourceMenu(res->s_param[0]));

	// toolbar?
	if (res->s_param[1].num > 0)
		dlg->toolbar[HuiToolbarTop]->SetByID(res->s_param[1]);

	// controls
	foreach(HuiResource &cmd, res->children){
		//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
		if (res->type == "Dialog"){
			dlg->SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( dlg, cmd.type, HuiGetLanguage(cmd.id),
								cmd.i_param[0], cmd.i_param[1],
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
		}else if (res->type == "SizableDialog"){
			//msg_write("insert " + cmd.id + " (" + cmd.type + ") into " + cmd.s_param[0]);
			dlg->SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( dlg, cmd.type, HuiGetLanguage(cmd.id),
								cmd.i_param[0], cmd.i_param[1],
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
		}
		dlg->Enable(cmd.id, cmd.enabled);
		if (cmd.image.num > 0)
			dlg->SetImage(cmd.id, cmd.image);
	}
	msg_db_m("  \\(^_^)/",1);
	return dlg;
	
	/*msg_error(format("HuiCreateResourceDialog  (id=%d)  m(-_-)m",id));
	CHuiWindow *d=HuiCreateDialog(format("-dialog (id=%d) not found in resource-",id),300,200,root,true,mf);
	return d;*/
}

HuiMenu *_create_res_menu_(HuiResource *res, int &index, int num)
{
	msg_db_f("_create_res_menu_",2);
	HuiMenu *menu = new HuiMenu();

	//msg_db_out(2,i2s(n));
	for (int i=0;i<num;i++){
		//msg_db_out(2,i2s(j));
		HuiResource *cmd = &res->children[index];
		if (cmd->type == "Item")
			menu->AddItem(get_lang(cmd->id, "", true), cmd->id);
		if (cmd->type == "ItemImage")
			menu->AddItemImage(get_lang(cmd->id, "", true), cmd->image, cmd->id);
		if (cmd->type == "ItemCheckable")
			menu->AddItemCheckable(get_lang(cmd->id, "", true), cmd->id);
		if (cmd->type == "ItemSeparator")
			menu->AddSeparator();
		if (cmd->type == "ItemPopup"){
			index ++;
			HuiMenu *sub = _create_res_menu_(res, index, cmd->i_param[0]);
			menu->AddSubMenu(get_lang(cmd->id, "", true), cmd->id, sub);
			index --;
		}
		menu->item.back()->Enable(cmd->enabled);
		index ++;
	}
	return menu;
}

HuiMenu *HuiCreateResourceMenu(const string &id)
{
	msg_db_f("HuiCreateResourceMenu",1);
	msg_db_m(id.c_str(),2);
	
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error(format("HuiCreateResourceMenu  (id=%d)  m(-_-)m", id.c_str()).c_str());
		return NULL;
	}

	int i = 0;
	msg_db_m("  \\(^_^)/",1);
	HuiMenu *m = _create_res_menu_(res, i, res->i_param[0]);
	return m;
}



inline bool res_is_letter(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'z'));
}

inline bool res_is_number(char c)
{
	return (c >= '0') && (c <= '9');
}

inline bool res_is_alphanum(char c)
{
	return res_is_letter(c) || res_is_number(c);
}

Array<string> res_tokenize(const string &s)
{
	Array<string> a;
	for (int i=0;i<s.num;i++){
		// ignore whitespace
		if ((s[i] == ' ') || (s[i] == '\t'))
			continue;

		// read token
		string token;
		if ((s[i] == '\"') || (s[i] == '\'')){
			// string
			i ++;
			for (;i<s.num;i++){
				if ((s[i] == '\"') || (s[i] == '\''))
					break;
				token.add(s[i]);
			}
		}else if (res_is_letter(s[i])){
			// word
			for (;i<s.num;i++){
				token.add(s[i]);
				if (i < s.num - 1)
					if (!res_is_alphanum(s[i + 1]))
						break;
			}
		}else if ((res_is_number(s[i])) || ((s[i] == '-') && (res_is_number(s[i + 1])))){
			// number
			for (;i<s.num;i++){
				token.add(s[i]);
				if (i < s.num - 1)
					if ((!res_is_number(s[i + 1])) && (s[i + 1] != '.'))
						break;
			}
		}else{
			// operator etc
			token.add(s[i]);
			if ((s[i] == '-') && (s[i + 1] == '>'))
				token.add(s[++ i]);
		}
		a.add(token);
	}
	return a;
}

bool res_sa_contains(Array<string> &a, const string &s)
{
	foreach(string &aa, a)
		if (aa == s)
			return true;
	return false;
}

int res_get_indent(const string &line)
{
	int indent = 0;
	for (int i=0;i<line.num;i++)
		if (line[i] != '\t')
			break;
		else
			indent ++;
	return indent;
}

void res_parse_new(const string &line, Array<string> &tokens)
{
	int indent = res_get_indent(line);
	string temp;
	for (int i=indent;i<line.num;i++){
		if (line[i] == ' '){
			if (temp.num > 0)
				tokens.add(temp);
			temp = "";
		}else if ((temp.num == 0) && ((line[i] == '\"') || (line[i] == '\''))){
			// string
			for (int j=i+1;j<line.num;j++){
				if (line[j] == '\\'){
					temp.add(line[j ++]);
					temp.add(line[j]);
				}else if ((line[j] == '\"') || (line[j] == '\'')){
					i = j;
					tokens.add(str_unescape(temp));
					temp = "";
					break;
				}else
					temp.add(line[j]);
			}
		}else
			temp.add(line[i]);
	}
	if (temp.num > 0)
		tokens.add(temp);
}

void res_add_option(HuiResourceNew &c, const string &option)
{
	if (option.head(6) == "image="){
		c.image = option.substr(6, -1);
		return;
	}
	if (option == "disabled"){
		c.enabled = false;
		return;
	}
	c.options.add(option);
}

bool res_load_line(string &l, HuiResourceNew &c)
{
	// parse line
	Array<string> tokens;
	res_parse_new(l, tokens);
	if (tokens.num == 0)
		return false;

	// id
	string id;
	if (tokens.num > 1)
		id = tokens[1];
	if (id == "?")
		id = "rand_id_" + i2s(randi(1000000));

	// dummy
	if (tokens[0] == ".")
		return false;
	if (tokens.num < 3)
		return false;

	// interpret tokens
	c.x = 0;
	c.y = 0;
	c.w = 1;
	c.h = 1;
	c.enabled = true;
	c.type = tokens[0];
	/*if (cur_indent == 0)
		c.type = "Dialog";*/
	c.id = id;
	c.title = tokens[2];
	int n_used = 3;
	if ((c.type == "Grid") || (c.type == "Dialog")){
		c.w = tokens[3]._int();
		c.h = tokens[4]._int();
		n_used = 5;
	}
	for (int i=n_used; i<tokens.num; i++)
		res_add_option(c, tokens[i]);
	return true;
}

bool res_load_rec(Array<string> &lines, int &cur_line, HuiResourceNew &c)
{
	int cur_indent = res_get_indent(lines[cur_line]);
	bool r = res_load_line(lines[cur_line], c);
	cur_line ++;

	for (int n=0; n<100; n++){
		if (cur_line >= lines.num)
			break;
		int indent = res_get_indent(lines[cur_line]);
		if (indent <= cur_indent)
			break;
		HuiResourceNew child;
		if (res_load_rec(lines, cur_line, child)){
			if (c.type == "Grid"){
				if (c.w > 0){
					child.x = n % c.w;
					child.y = n / c.w;
					//msg_write(format("%d %d", c.x, c.y));
				}
			}else if (c.type == "TabControl"){
				child.x = n;
			}
			c.children.add(child);
		}

	}
	return r;
}

void HuiResourceNew::show(int indent)
{
	string nn;
	for (int i=0;i<indent;i++)
		nn += "    ";
	msg_write(nn + type + " - " + id + format(" - %d %d %d %d - ", x, y, w, h) + sa2s(options));
	foreach(HuiResourceNew &child, children)
		child.show(indent + 1);
}

void HuiResourceNew::load(const string &buffer)
{
	Array<string> lines = buffer.explode("\n");
	for (int i=lines.num-1; i>=0; i--)
		if (lines[i].num == 0)
			lines.erase(i);
	int cur_line = 0;

	//HuiResourceNew c;
	res_load_rec(lines, cur_line, *this);
}

