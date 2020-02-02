/*
 * CLIParser.cpp
 *
 *  Created on: 02.02.2020
 *      Author: michi
 */

#include "CLIParser.h"
#include "../lib/file/msg.h"


string CLIParser::Mode::str(const string &cmd) {
	string s = cmd;
	if (name != "")
		s += " " + name;
	for (auto &p: parameter)
		s += " " + p;
	return s;
}

bool CLIParser::Mode::match(const string &arg) {
	return sa_contains(name.explode("/"), arg);
}

int CLIParser::Mode::grab_parameters(const Array<string> &arg) {
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

string CLIParser::Option::str() {
	if (parameter.num > 0)
		return name + " " + parameter;
	else
		return name;
}

void CLIParser::option(const string &name, const std::function<void()> &cb) {
	options.add({name, "", cb, nullptr});
}
void CLIParser::option(const string &name, const string &p, const std::function<void(const string&)> &cb) {
	options.add({name, p, nullptr, cb});
}
void CLIParser::mode(const string &name, const Array<string> &p, const std::function<void(const Array<string>&)> &cb) {
	modes.add({name, p, cb});
}

void CLIParser::info(const string &cmd, const string &i) {
	command = cmd;
	_info = i;
}

void CLIParser::show() {
	msg_write(_info);
	msg_write("");
	msg_write("run:");
	for (auto &m: modes)
		msg_write("  " + m.str(command));
	if (options.num > 0) {
		msg_write("");
		msg_write("options:");
		for (auto &o: options)
			msg_write("  " + o.str());
	}
}

void CLIParser::parse(const Array<string> &_arg) {
	bool had_mode = false;
	for (int i=1; i<_arg.num; i++) {
		if (_arg[i].head(1) == "-") {
			bool found = false;
			for (auto &o: options) {
				if (sa_contains(o.name.explode("/"), _arg[i])) {
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
			for (auto &m: modes) {
				if (m.name == "")
					continue;
				if (m.match(_arg[i])) {
					int n = m.grab_parameters(_arg.sub(i+1, -1));
					if (n < 0) {
						msg_error("missing parameter after " + m.name);
						msg_write(m.str(command));
						exit(1);
					}
					m.callback(_arg.sub(i+1, n));
					i += n;
					found = true;
					had_mode = true;
				}
			}
			if (!found) {
				msg_error("unknown option " + _arg[i]);
				exit(1);
			}
		} else {
			// rest
			arg.add(_arg[i]);
		}
	}

	if (had_mode) {
		if (arg.num > 0) {
			msg_error("unexpected parameter: " + arg[0]);
			exit(1);
		}

	} else {
	// default mode
	for (auto &m: modes) {
		if (m.name != "")
			continue;
		int n = m.grab_parameters(arg);
		if (n < 0) {
			msg_error("missing parameter");
			msg_write(m.str(command));
			exit(1);
		}
		if (n < arg.num) {
			msg_error("unexpected parameter: " + arg[n]);
			exit(1);
		}
		m.callback(arg);
		had_mode = true;
	}
	}

	if (!had_mode) {
		msg_error("no default mode?!?");
		exit(1);
	}

	/*if (!found) {
		msg_error("unexpected parameter: " + _arg[i]);
		exit(1);
	}*/

}
