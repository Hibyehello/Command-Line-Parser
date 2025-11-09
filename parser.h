#pragma once

#include <cstdio>
#include <cstring>
#include <map>
#include <vector>

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

  void printUsage() const;
  void parseArg(const char *arg);
};

struct Arg {
  Option *in_command;
  std::vector<Option *> in_flags;
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

  bool canParse() { return arg_index < argc; };

private:
  const char *m_name;
  bool has_commands = false;
  bool command_found = false;
  int arg_index = 1;

  std::vector<Option *> commands;
  std::vector<Option *> flags;

  Arg in_args;

  int argc;
  const char **argv;
};
