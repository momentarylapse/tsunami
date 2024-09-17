/*
 * CommandLineParser.h
 *
 *  Created on: Feb 12, 2021
 *      Author: michi
 */

#pragma once

#include "../base/base.h"
#include <functional>


class CommandLineParser {
public:
	using Callback = std::function<void()>;
	using CallbackString = std::function<void(const string&)>;
	using CallbackStringList = std::function<void(const Array<string>&)>;

	void option(const string &name, const string& comment, Callback cb);
	void option(const string &name, const string &p, const string& comment, CallbackString cb);
	struct Option {
		Array<string> names;
		string parameter;
		string comment;
		Callback callback;
		CallbackString callback_param;

		string sig() const;
	};
	Array<Option> options;

	// "example", "<P1> <P2> [OPT] ...", "comment", lambda(string[] a) ...
	// name = "" -> default
	void cmd(const string &name, const string &params, const string &comment, CallbackStringList cb);
	struct Command {
		Array<Array<string>> names;
		Array<string> params;
		string comment;
		CallbackStringList callback;
		bool is_hidden = false;

		bool match(const Array<string> &arg, int &offset) const;
		int min_params() const;
		string sig() const;
		bool is_default() const;
	};
	Array<Command> commands;

	void info(const string &p, const string &i);
	string _info;
	string _program;
	bool error = false;

	void show();
	//Array<string> arg;
	void parse(const Array<string> &arg);

private:
	bool parse_commands(const Array<string> &arg);
	Array<string> parse_options(const Array<string> &arg);
	int do_option(const string &name, const Array<string> &args);

	int non_default_commands() const;
	Command *default_command() const;
};


