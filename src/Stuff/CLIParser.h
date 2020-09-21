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
		string info;
		std::function<void()> callback;
		std::function<void(const string&)> callback_param;
		string str() const;
		string stri() const;
		bool match(const string &arg) const;
	};
	Array<Option> options;
	struct Mode {
		string name;
		Array<string> parameter;
		string info;
		std::function<void(const Array<string>&)> callback;
		string str(const string &cmd = "") const;
		string stri(const string &cmd) const;
		bool match(const string &arg) const;
		int grab_parameters(const Array<string> &arg) const;
	};
	Array<Mode> modes;
	void flag(const string &name, const string &info, const std::function<void()> &cb);
	void option(const string &name, const string &p, const string &info, const std::function<void(const string&)> &cb);
	void mode(const string &name, const Array<string> &p, const string &info, const std::function<void(const Array<string> &)> &cb);
	string command;
	string _info;
	void info(const string &cmd, const string &i);
	void show_info();
	void parse(const Array<string> &arg);

private:
	Array<string> eval_all_options(const Array<string> &arg);
	bool try_run_mode(const Mode &m, const Array<string> &arg);
	void die(const string &msg);
};

#endif /* SRC_STUFF_CLIPARSER_H_ */
