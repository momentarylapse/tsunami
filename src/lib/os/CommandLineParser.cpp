/*
 * CLIParser.cpp
 *
 *  Created on: Feb 12, 2021
 *      Author: michi
 */

#include "CommandLineParser.h"
#include "../base/algo.h"
#include "../base/sort.h"
#include "../os/msg.h"


void CommandLineParser::option(const string &name, const string &comment, Callback cb) {
	options.add({name.explode("/"), "", comment, cb, nullptr});
}

void CommandLineParser::option(const string &name, const string &p, const string &comment, CallbackString cb) {
	options.add({name.explode("/"), p, comment, nullptr, cb});
}

// "A B/C D"
void CommandLineParser::cmd(const string &name, const string &params, const string &comment, CallbackStringList cb) {
	Array<Array<string>> names;
	bool hidden = (name.head(8) == "@hidden ");
	for (auto &n: name.replace("@hidden ", "").replace(",", "/").explode("/"))
		names.add(n.explode(" "));
	if (names.num == 0)
		names.add({});
	commands.add({names, params.explode(" "), comment, cb, hidden});
}

void CommandLineParser::info(const string &p, const string &i) {
	_program = p;
	_info = i;
}

static string format_title_with_block(const string &title, const string& block, int n = 28) {
	string s = "  " + title;
	if (s.num + 2 <= n)
		s += string(" ").repeat(n - s.num);
	else
		s += "\n" + string(" ").repeat(n);
	return s + block;
}

int CommandLineParser::non_default_commands() const {
	return base::count_if(commands, [] (const Command &c) { return !c.is_default() and !c.is_hidden; });
}

CommandLineParser::Command *CommandLineParser::default_command() const {
	return base::find_if(commands, [] (const Command &c) { return c.is_default(); });
}

void CommandLineParser::show() {
	if (default_command()) {
		for (auto &c: commands)
			if (c.is_default())
				msg_write(format("usage:    %s%s", _program, c.sig()));
		if (non_default_commands() > 0)
			msg_write(format("          %s  [COMMANDS]", _program));
	} else {
		msg_write(format("usage:    %s  [COMMANDS]", _program));
	}
	msg_write(_info);

	if (non_default_commands() > 0) {
		msg_write("\ncommands:");
		for (auto &c: commands)
			if (!c.is_default() and !c.is_hidden)
				msg_write(format_title_with_block(c.sig(), c.comment));
	}

	if (options.num > 0) {
		msg_write("\noptions:");
		for (auto &o: options)
			msg_write(format_title_with_block(o.sig(), o.comment));
	}
}


Array<string> CommandLineParser::parse_options(const Array<string> &arg) {
	Array<string> arg2;
	for (int i=0; i<arg.num; i++) {
		bool is_command = false;
		int _offset;
		for (auto &c: commands)
			if (c.match(arg.sub_ref(i), _offset))
				if (_offset > 0)
					is_command = true;

		if (!is_command and arg[i].head(1) == "-") {
			i += do_option(arg[i], arg.sub_ref(i+1));
		} else if (auto d = default_command()) {
			// FIXME (just a hack for now)
			if ((d->params.num > 0) and (d->params.back() == "..."))
				return arg.sub_ref(i);
			arg2.add(arg[i]);
		} else {
			arg2.add(arg[i]);
		}
	}
	return arg2;
}

int CommandLineParser::do_option(const string &name, const Array<string> &arg_rest) {
	for (auto &o: options) {
		if (sa_contains(o.names, name)) {
			if (o.parameter == "") {
				o.callback();
				return 0;
			} else {
				if (arg_rest.num == 0)
					throw Exception("missing parameter: " + o.sig());
				o.callback_param(arg_rest[0]);
				return 1;
			}
		}
	}
	throw Exception("unknown option " + name);
}

bool CommandLineParser::parse_commands(const Array<string> &arg) {
	//msg_write("ARG: " + sa2s(arg));
	auto sorted_commands = base::sorted(commands, [] (const Command &a, const Command &b) {
		return a.names[0].num >= b.names[0].num;
	});
	for (auto &c: sorted_commands) {
		int offset;
		if (c.match(arg, offset)) {
			//msg_write(" -->> " + c.sig() + format("   %d %d %d", offset, arg.num, c.min_params()));
			if (arg.num - offset < c.min_params())
				throw Exception("missing parameters: " + c.sig());
			c.callback(arg.sub_ref(offset));
			return true;
		}// else
			//msg_write(" :(");
	}
	return false;
}

void CommandLineParser::parse(const Array<string> &_arg) {
	auto arg = _arg.sub_ref(1); // ignore progname

	try {
		auto arg2 = parse_options(arg);
		if (parse_commands(arg2))
			return;
		if (arg2.num > 0) {
			msg_error("unhandled command");
			error = true;
		}
		show();
	} catch (Exception &e) {
		error = true;
		msg_error(e.message());
	}
/*
	for (int i=1; i<_arg.num; i++) {
		if (_arg[i].head(1) == "-") {
			bool found = false;
			for (auto &o: options) {
				if (sa_contains(o.names.explode("/"), _arg[i])) {
					if (o.parameter.num > 0) {
						if (_arg.num <= i + 1) {
							msg_error("parameter '" + o.parameter + "' expected after " + o.name);
							exit(1);
						}
						o.callback_param(_arg[i+1]);
						i ++;
					} else {
						o.callback();
					}
					found = true;
				}
			}
			if (!found) {
				msg_error("unknown option " + _arg[i]);
				exit(1);
			}
		} else {
			// rest
			arg = _arg.sub_ref(i);
			break;
		}
	}
*/
}

string CommandLineParser::Option::sig() const {
	string s = implode(names, ", ");
	if (parameter != "")
		s += "  " + parameter;
	return s;
}

bool CommandLineParser::Command::match(const Array<string> &arg, int &offset) const {
	if (names.num == 0) {
		offset = 0;
		return true;
	}
	for (auto &nn: names) {
		//msg_write(sa2s(nn) + sa2s(arg));
		if (arg.num < nn.num)
			continue;
		bool ok = true;
		for (int i=0; i<nn.num; i++)
			if (arg[i] != nn[i])
				if ((i == 0 and arg[i] != "--" + nn[i]) or (i > 0))
					ok = false;
		//msg_write(ok);
		if (ok) {
			offset = nn.num;
			return true;
		}
	}
	return false;
}

int CommandLineParser::Command::min_params() const {
	int m = 0;
	for (auto &p: params) {
		if (p == "..." or p == "...!" or p.head(1) == "[")
			continue;
		m ++;
	}
	return m;
}

string CommandLineParser::Command::sig() const {
	Array<string> s;
	for (auto &nn: names)
		s.add(implode(nn, " "));
	return implode(s, ", ") + "  " + implode(params, "  ");
}

bool CommandLineParser::Command::is_default() const {
	return (names.num == 1) and (names[0].num == 0);
}
