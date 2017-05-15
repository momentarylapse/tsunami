#include "hui.h"
#include "../math/math.h"
#include "Controls/Control.h"
#include "internal.h"

namespace hui
{

extern Array<Language> _languages_;
Array<Resource> _resources_;

void Resource::reset()
{
	type = "";
	id = "";
	options.clear();
	image = "";
	enabled = true;
	page = 0;
	children.clear();
	x = y = w = h = 0;
}

Resource *Resource::get_node(const string &id) const
{
	for (Resource &c: children){
		if (c.id == id)
			return &c;
		Resource *ret = c.get_node(id);
		if (ret)
			return ret;
	}
	return NULL;
}

void LoadResourceCommand5(File *f, Resource *c)
{
	c->type = f->ReadStr();
	c->id = f->ReadStr();
	c->options = f->ReadStr().explode(",");
	c->image = f->ReadStr();
	c->enabled = f->ReadBool();
	c->x = f->ReadInt();
	c->y = f->ReadInt();
	c->w = f->ReadInt();
	c->h = f->ReadInt();
	c->page = f->ReadInt();
	int n = f->ReadInt();
	for (int i=0; i<n; i++){
		Resource child;
		LoadResourceCommand5(f, &child);
		c->children.add(child);
	}
}

void LoadResource(const string &filename)
{
	// dirty...
	_resources_.clear();
	_languages_.clear();

	File *f = FileOpen(filename);
	if (f){
		int ffv = f->ReadFileFormatVersion();
		if (ffv != 5){
			FileClose(f);
			msg_error("hui resource version is " + i2s(ffv) + " (5 expected)");
			return;
		}

		f->ReadComment();
		int nres = f->ReadInt();
		for (int i=0;i<nres;i++){
			Resource res;
			res.children.clear();
			f->ReadComment();
			LoadResourceCommand5(f, &res);
			_resources_.add(res);
		}

		// languages
		f->ReadComment();
		int nl = f->ReadInt();
		for (int l=0;l<nl;l++){
			Language hl;

			// Language
			f->ReadComment();
			hl.name = f->ReadStr();

			//  NumIDs
			f->ReadComment();
			int n = f->ReadInt();
			f->ReadComment(); // Text
			for (int i=0;i<n;i++){
				Language::Command c;
				Array<string> ids = f->ReadStr().explode("/");
				if (ids.num >= 2)
					c._namespace = ids[0];
				if (ids.num >= 1)
					c.id = ids.back();
				c.text = str_unescape(f->ReadStr());
				c.tooltip = str_unescape(f->ReadStr());
				hl.cmd.add(c);
			}
			// Num Language Strings
			f->ReadComment();
			n = f->ReadInt();
			// Text
			f->ReadComment();
			for (int i=0;i<n;i++){
				Language::Translation s;
				s.orig = str_unescape(f->ReadStr());
				s.trans = str_unescape(f->ReadStr());
				hl.trans.add(s);
			}
			_languages_.add(hl);
		}
		FileClose(f);
	}
}

Resource *GetResource(const string &id)
{
	for (Resource &r: _resources_)
		if (r.id == id)
			return &r;
	if (id.num > 0)
		msg_error("hui resource not found: " + id);
	return NULL;
}

Window *CreateResourceDialog(const string &id, Window *root)
{
	//return HuiCreateDialog("-dialog not found in resource-",200,100,root,true,mf);
	Resource *res = GetResource(id);
	if (!res){
		msg_error(format("HuiCreateResourceDialog  (id=%s)  m(-_-)m",id.c_str()));
		return NULL;
	}
	
	msg_db_m("HuiResDialog",2);

	string menu_id, toolbar_id;
	bool allow_parent = false;
	for (string &o: res->options){
		if ((o == "allow-root") or (o == "allow-parent"))
			allow_parent = true;
		if (o.head(5) == "menu=")
			menu_id = o.substr(5, -1);
		if (o.head(8) == "toolbar=")
			toolbar_id = o.substr(8, -1);
	}

	// dialog
	Window *dlg;
	if (res->type == "SizableDialog")
		dlg = new Dialog(GetLanguageR(res->id, *res), res->w, res->h, root, allow_parent);
	else
		dlg = new FixedDialog(GetLanguageR(res->id, *res), res->w, res->h, root, allow_parent);

	// menu?
	if (menu_id.num > 0)
		dlg->setMenu(CreateResourceMenu(menu_id));

	// toolbar?
	if (toolbar_id.num > 0)
		dlg->toolbar[TOOLBAR_TOP]->setByID(toolbar_id);

	// controls
	for (Resource &cmd: res->children)
		dlg->_addControl(id, cmd, "");

	msg_db_m("  \\(^_^)/",1);
	return dlg;
	
	/*msg_error(format("HuiCreateResourceDialog  (id=%d)  m(-_-)m",id));
	CHuiWindow *d=HuiCreateDialog(format("-dialog (id=%d) not found in resource-",id),300,200,root,true,mf);
	return d;*/
}

Menu *_create_res_menu_(const string &ns, Resource *res)
{
	Menu *menu = new Menu();

	//msg_db_out(2,i2s(n));
	for (Resource &c: res->children){
		//msg_db_out(2,i2s(j));
		if (c.type == "Item"){
			if (sa_contains(c.options, "checkable"))
				menu->addItemCheckable(get_lang(ns, c.id, "", true), c.id);
			else if (c.image.num > 0)
				menu->addItemImage(get_lang(ns, c.id, "", true), c.image, c.id);
			else
				menu->addItem(get_lang(ns, c.id, "", true), c.id);
		}else if (c.type == "ItemImage")
			menu->addItemImage(get_lang(ns, c.id, "", true), c.image, c.id);
		else if (c.type == "ItemCheckable")
			menu->addItemCheckable(get_lang(ns, c.id, "", true), c.id);
		else if ((c.type == "ItemSeparator") or (c.type == "Separator"))
			menu->addSeparator();
		else if (c.type == "ItemPopup"){
			Menu *sub = _create_res_menu_(ns, &c);
			menu->addSubMenu(get_lang(ns, c.id, "", true), c.id, sub);
		}
		menu->items.back()->enable(c.enabled);
	}
	return menu;
}

Menu *CreateResourceMenu(const string &id)
{
	Resource *res = GetResource(id);
	if (!res){
		msg_error(format("CreateResourceMenu  (id=%d)  m(-_-)m", id.c_str()).c_str());
		return NULL;
	}

	Menu *m = _create_res_menu_(id, res);
	return m;
}



inline bool res_is_letter(char c)
{
	return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'z'));
}

inline bool res_is_number(char c)
{
	return (c >= '0') and (c <= '9');
}

inline bool res_is_alphanum(char c)
{
	return res_is_letter(c) or res_is_number(c);
}

Array<string> res_tokenize(const string &s)
{
	Array<string> a;
	for (int i=0;i<s.num;i++){
		// ignore whitespace
		if ((s[i] == ' ') or (s[i] == '\t'))
			continue;

		// read token
		string token;
		if ((s[i] == '\"') or (s[i] == '\'')){
			// string
			i ++;
			for (;i<s.num;i++){
				if ((s[i] == '\"') or (s[i] == '\''))
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
		}else if ((res_is_number(s[i])) or ((s[i] == '-') and (res_is_number(s[i + 1])))){
			// number
			for (;i<s.num;i++){
				token.add(s[i]);
				if (i < s.num - 1)
					if ((!res_is_number(s[i + 1])) and (s[i + 1] != '.'))
						break;
			}
		}else{
			// operator etc
			token.add(s[i]);
			if ((s[i] == '-') and (s[i + 1] == '>'))
				token.add(s[++ i]);
		}
		a.add(token);
	}
	return a;
}

bool res_sa_contains(Array<string> &a, const string &s)
{
	for (string &aa: a)
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
		}else if ((temp.num == 0) and ((line[i] == '\"') or (line[i] == '\''))){
			// string
			for (int j=i+1;j<line.num;j++){
				if (line[j] == '\\'){
					temp.add(line[j ++]);
					temp.add(line[j]);
				}else if ((line[j] == '\"') or (line[j] == '\'')){
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

void res_add_option(Resource &c, const string &option)
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

bool res_load_line(string &l, Resource &c)
{
	// parse line
	Array<string> tokens;
	res_parse_new(l, tokens);
	if (tokens.num == 0)
		return false;

	c.x = 0;
	c.y = 0;
	c.w = 1;
	c.h = 1;
	c.enabled = true;

	// id
	string id;
	if (tokens.num > 1)
		id = tokens[1];
	if (id == "?")
		id = "rand_id_" + i2s(randi(1000000));

	// dummy
	if (tokens[0] == ".")
		return false;
	if (tokens[0] == "Separator"){
		c.type = tokens[0];
		return true;
	}
	if (tokens.num < 3)
		return false;

	// interpret tokens
	c.type = tokens[0];
	/*if (cur_indent == 0)
		c.type = "Dialog";*/
	c.id = id;
	c.title = tokens[2];
	int n_used = 3;
	if ((c.type == "Grid") or (c.type == "Dialog")){
		c.w = tokens[3]._int();
		c.h = tokens[4]._int();
		n_used = 5;
	}
	for (int i=n_used; i<tokens.num; i++)
		res_add_option(c, tokens[i]);
	return true;
}

bool res_load_rec(Array<string> &lines, int &cur_line, Resource &c)
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
		Resource child;
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

void Resource::show(int indent)
{
	string nn;
	for (int i=0;i<indent;i++)
		nn += "    ";
	msg_write(nn + type + " - " + id + format(" - %d %d %d %d - ", x, y, w, h) + sa2s(options));
	for (Resource &child: children)
		child.show(indent + 1);
}

void Resource::load(const string &buffer)
{
	Array<string> lines = buffer.explode("\n");
	for (int i=lines.num-1; i>=0; i--)
		if (lines[i].num == 0)
			lines.erase(i);
	int cur_line = 0;

	//HuiResourceNew c;
	res_load_rec(lines, cur_line, *this);
}

};

