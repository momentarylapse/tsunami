/*
 * CLIParser.cpp
 *
 *  Created on: 02.02.2020
 *      Author: michi
 */

#include "CLIParser.h"
#include "../lib/file/msg.h"


string CLIParser::Mode::str(const string &cmd) const {
	string s = cmd;
	if (name != "")
		s += " " + name;
	for (auto &p: parameter)
		s += " " + p;
	return s;
}

string CLIParser::Mode::stri(const string &cmd) const {
	return format("%-36s # %s", str(cmd), info);
}

bool CLIParser::Mode::match(const string &arg) const {
	return sa_contains(name.explode("/"), arg);
}

int CLIParser::Mode::grab_parameters(const Array<string> &arg) const {
	int min_params = 0;
	int max_params = 0;
	for (auto &p: parameter) {
		if (p == "...") {
			max_params = 1000;
		} else if (p[0] == '[') {
			max_params ++;
		} else {
			min_params ++;
			max_params ++;
		}
	}

	if (arg.num < min_params)
		return -1;

	return min(arg.num, max_params);
}

string CLIParser::Option::str() const {
	if (parameter.num > 0)
		return name + " " + parameter;
	else
		return name;
}
string CLIParser::Option::stri() const {
	return format("%-36s # %s", str(), info);
}

void CLIParser::flag(const string &name, const string &info, const std::function<void()> &cb) {
	options.add({name, "", info, cb, nullptr});
}
void CLIParser::option(const string &name, const string &p, const string &info, const std::function<void(const string&)> &cb) {
	options.add({name, p, info, nullptr, cb});
}
void CLIParser::mode(const string &name, const Array<string> &p, const string &info, const std::function<void(const Array<string>&)> &cb) {
	modes.add({name, p, info, cb});
}

void CLIParser::info(const string &cmd, const string &i) {
	command = cmd;
	_info = i;
}

void CLIParser::show_info() {
	msg_write(_info);
	msg_write("");
	msg_write("run:");
	for (auto &m: modes)
		msg_write("  " + m.stri(command));
	if (options.num > 0) {
		msg_write("");
		msg_write("options:");
		for (auto &o: options)
			msg_write("  " + o.stri());
	}
}

void CLIParser::die(const string &msg) {
	msg_error(msg);
	exit(1);
}

bool CLIParser::try_run_mode(const Mode &m, const Array<string> &arg) {
	int i0 = 0;

	if (m.name == "") {
		// always match
	} else {
		if (arg.num == 0)
			return false;
		if (!m.match(arg[0]))
			return false;
		i0 = 1;
	}

	int n = m.grab_parameters(arg.sub_ref(i0));
	if (n < 0)
		die("missing parameter for command: " + m.str());
	if (n < arg.num-i0)
		die("too many parameters for command: " + m.str());
	m.callback(arg.sub_ref(i0, i0 + n));
	return true;
}

Array<string> CLIParser::eval_all_options(const Array<string> &arg) {

	Array<string> rest;

	// pick out all options
	for (int i=1; i<arg.num; i++) {
		if (arg[i].head(1) != "-") {
			rest.add(arg[i]);
			continue;
		}

		bool found = false;

		// is a mode?
		for (auto &m: modes) {
			if (m.name == "")
				continue;
			if (m.match(arg[i])) {
				rest.add(arg[i]);
				found = true;
			}
		}
		if (found)
			continue;

		// is an option?
		for (auto &o: options) {
			if (sa_contains(o.name.explode("/"), arg[i])) {
				if (o.parameter.num > 0) {
					if (arg.num <= i + 1)
						die("parameter '" + o.parameter + "' expected after " + o.name);
					o.callback_param(arg[i+1]);
					i ++;
				} else {
					o.callback();
				}
				found = true;
			}
		}


		if (!found)
			die("unknown option: " + arg[i]);
	}
	return rest;
}

void CLIParser::parse(const Array<string> &arg) {

	auto rest = eval_all_options(arg);

	// first try named modes
	for (auto &m: modes) {
		if (m.name != "")
			if (try_run_mode(m, rest))
				return;
	}
	// then the default mode
	for (auto &m: modes) {
		if (m.name == "")
			if (try_run_mode(m, rest))
				return;
	}


	die("no matching default mode?!?");
}
