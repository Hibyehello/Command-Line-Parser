#pragma once

#include <limits>

enum OptionType { COMMAND, FLAG };
enum ArgType { NONE, STR, NUM };

struct Option {

  OptionType type;

  const char *verbose_name;

  const char *short_name;

  ArgType arg_type;

  const char *arg_name;

  long arg_num = 0;

  const char *arg_str = "";

  const char *desc = "";

  void (*callback)();

  void printUsage() const;

  void parseArg(const char *arg);
};

struct Options {
  Option opts[std::numeric_limits<char>::max()];
  int cur_loc = 0;
  int next_free_loc = 0;

  void addNext(Option opt) { opts[next_free_loc++] = opt; }
  Option *getNext() { return &opts[cur_loc++]; }
};

struct Arg {
  Options in_command;

  Options in_flags;
};

class ArgParser {
public:
  ArgParser(const char *app_name, int argc, const char *argv[]);

  void parseArgs();

  Option *parseCommand(bool reset_command_found = true);

  Option *parseFlags();

  Arg *getArgs() { return &in_args; }

  void printUsage();

  void addOption(OptionType opt, const char *verbose_name,

                 const char *short_name = "", ArgType arg_type = NONE,

                 const char *arg_name = "");

  void addOptionDesc(const char *opt_name, const char *desc);
  void addCallback(const char *opt_name, void (*func)());

  bool canParse() { return arg_index < argc; };

private:
  const char *m_name;

  bool has_commands = false;

  bool command_found = false;

  int arg_index = 1;

  Options commands;

  Options flags;

  Arg in_args;

  int argc;
  const char **argv;
};
