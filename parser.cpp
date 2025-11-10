#include "parser.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void Option::printUsage() const {
  printf("%s%s", ((type == COMMAND) ? "  " : "  --"), verbose_name);

  if (strcmp(short_name, "") != 0) {
    printf("%s%s", ((type == COMMAND) ? ", " : ", -"), short_name);
  }
  if (strcmp(arg_name, "") != 0) {
    printf(" <%s>", arg_name);
  }

  if (strcmp(desc, "") != 0)
    printf("%-25s%s", "\n", desc);

  printf("\n");
}

void Option::parseArg(const char *arg) {
  if (arg_type == NUM) {
    char *endptr;
    arg_num = strtol(arg, &endptr, 0);
    if (endptr == arg) {
      fprintf(stderr, "Argument '%s' expected a number, but got %s\n",
              verbose_name, arg);
      exit(EXIT_FAILURE);
    }
    if (errno == ERANGE) {
      fprintf(stderr, "%s error for argument '%s'\n",
              arg_num < 0 ? "underflow" : "overflow", verbose_name);
      exit(EXIT_FAILURE);
    }
    return;
  }
  arg_str = arg;
}

ArgParser::ArgParser(const char *app_name, int argc, const char *argv[])
    : m_name(app_name), argc(argc), argv(argv), in_args(Arg{}) {}

void ArgParser::parseArgs() {
  if (argc < 2) {
    fprintf(stderr, "No arguments given!\n");
    return;
  }
  if ((strcmp(argv[1] + 2, "help") == 0 || strcmp(argv[1] + 1, "h") == 0)) {
    printUsage();
    return;
  }

  while (arg_index < argc) {
    parseCommand(false);

    while (canParse())
      parseFlags();
  }
}

Option *ArgParser::parseCommand(bool reset_command_found) {
  if (argc < 2) {
    fprintf(stderr, "No arguments given!\n");
    return nullptr;
  }
  if ((strcmp(argv[arg_index] + 2, "help") == 0 ||
       strcmp(argv[arg_index] + 1, "h") == 0)) {
    return nullptr;
  }

  if (reset_command_found) {
    command_found = false;
  }

  // auto it = std::find_if(commands.begin(), commands.end(), [&](Option *opt) {
  //   return (strcmp(opt->verbose_name, argv[arg_index]) == 0) ||
  //          (strcmp(opt->short_name, argv[arg_index]) == 0);
  // });

  commands.cur_loc = 0;
  while (commands.cur_loc < commands.next_free_loc) {
    Option *opt = commands.getNext();
    if (strcmp(opt->verbose_name, argv[arg_index]) == 0 ||
        (strcmp(opt->short_name, argv[arg_index]) == 0)) {
      if (command_found) {
        fprintf(stderr, "A command has already been specified!\n");
        exit(EXIT_FAILURE);
      }

      in_args.in_command.addNext(*opt);
      command_found = true;
      arg_index++;

      if (opt->callback)
        opt->callback();

      return opt;
    }
  }

  if (!command_found) {
    fprintf(stderr, "Unknown command found '%s'\n", argv[arg_index]);
    exit(EXIT_FAILURE);
  }

  return nullptr;
}

Option *ArgParser::parseFlags() {
  if (argc < 2) {
    fprintf(stderr, "No arguments given!\n");
    return nullptr;
  }
  if ((strcmp(argv[1] + 2, "help") == 0 || strcmp(argv[1] + 1, "h") == 0)) {
    printUsage();
    exit(EXIT_SUCCESS);
  }

  if (has_commands && !command_found) {
    fprintf(stderr, "Unknown command, did you input a command?\n");
    exit(EXIT_FAILURE);
  }

  flags.cur_loc = 0;
  while (flags.cur_loc < flags.next_free_loc) {
    Option *opt = flags.getNext();
    if (strcmp(opt->verbose_name, argv[arg_index] + 2) == 0 ||
        (strcmp(opt->short_name, argv[arg_index] + 1) == 0)) {
      if (opt->arg_type != NONE) {
        opt->parseArg(argv[++arg_index]);
      }
      in_args.in_flags.addNext(*opt);
      arg_index++;

      if (opt->callback)
        opt->callback();

      return opt;
    }
  }

  fprintf(stderr, "Unknown flag found '%s'\n", argv[arg_index]);
  exit(EXIT_FAILURE);
}

void ArgParser::printUsage() {
  printf("Usage: %s", m_name);
  if (commands.next_free_loc > 0) {
    printf(" [COMMAND]");
  }
  if (flags.next_free_loc > 0) {
    printf(" [OPTIONS] ...");
  }

  printf("\n");

  if (commands.next_free_loc > 0) {
    printf("\nCommands:\n");
    commands.cur_loc = 0;
    while (commands.cur_loc < commands.next_free_loc) {
      Option *opt = commands.getNext();
      opt->printUsage();
    }
  }

  if (flags.next_free_loc > 0) {
    printf("\nOptions:\n");
    flags.cur_loc = 0;
    while (flags.cur_loc < flags.next_free_loc) {
      Option *opt = flags.getNext();
      opt->printUsage();
    }
  }
}

void ArgParser::addOption(OptionType type, const char *verbose_name,
                          const char *short_name, ArgType arg_type,
                          const char *arg_name) {
  if (type == COMMAND) {
    commands.addNext(Option{type, verbose_name, short_name, NONE, ""});
    has_commands = true;
  } else {
    flags.addNext(Option{type, verbose_name, short_name, arg_type, arg_name});
  }
}

void ArgParser::addOptionDesc(const char *opt_name, const char *desc) {
  commands.cur_loc = 0;
  while (commands.cur_loc < commands.next_free_loc) {
    Option *opt = commands.getNext();
    if (strcmp(opt->verbose_name, opt_name) == 0) {
      opt->desc = desc;
      return;
    }
  }

  flags.cur_loc = 0;
  while (flags.cur_loc < flags.next_free_loc) {
    Option *opt = flags.getNext();
    if (strcmp(opt->verbose_name, opt_name) == 0) {
      opt->desc = desc;
      return;
    }
  }

  fprintf(stderr, "No Option with specified name exists\n");
  exit(EXIT_FAILURE);
}

void ArgParser::addCallback(const char *opt_name, void (*func)()) {
  commands.cur_loc = 0;
  while (commands.cur_loc < commands.next_free_loc) {
    Option *opt = commands.getNext();
    if (strcmp(opt->verbose_name, opt_name) == 0) {
      opt->callback = func;
      return;
    }
  }

  flags.cur_loc = 0;
  while (flags.cur_loc < flags.next_free_loc) {
    Option *opt = flags.getNext();
    if (strcmp(opt->verbose_name, opt_name) == 0) {
      opt->callback = func;
      return;
    }
  }

  fprintf(stderr, "No Option with specified name exists\n");
  exit(EXIT_FAILURE);
}
