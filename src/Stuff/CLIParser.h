/*
 * CLIParser.h
 *
 *  Created on: 02.02.2020
 *      Author: michi
 */

#ifndef SRC_STUFF_CLIPARSER_H_
#define SRC_STUFF_CLIPARSER_H_

#include "../lib/base/base.h"
#include <functional>


class CLIParser {
public:
	struct Option {
		string name;
		string parameter;
		std::function<void()> callback;
		std::function<void(const string&)> callback_param;
		string str();
		bool match(const string &arg);
	};
	Array<Option> options;
	struct Mode {
		string name;
		Array<string> parameter;
		std::function<void(const Array<string>&)> callback;
		string str(const string &cmd);
		bool match(const string &arg);
		int grab_parameters(const Array<string> &arg);
	};
	Array<Mode> modes;
	void option(const string &name, const std::function<void()> &cb);
	void option(const string &name, const string &p, const std::function<void(const string&)> &cb);
	void mode(const string &name, const Array<string> &p, const std::function<void(const Array<string> &)> &cb);
	string command;
	string _info;
	void info(const string &cmd, const string &i);
	void show();
	Array<string> arg;
	void parse(const Array<string> &_arg);
};

#endif /* SRC_STUFF_CLIPARSER_H_ */
