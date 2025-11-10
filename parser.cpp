#include "parser.h"

#include <algorithm>
#include <cerrno>
#include <iomanip>
#include <sstream>

void Option::printUsage() const {
  std::stringstream out;

  out << ((type == COMMAND) ? "  " : "  --") << verbose_name;

  if (strcmp(short_name, "") != 0) {
    out << ((type == COMMAND) ? ", " : ", -") << short_name;
  }
  if (strcmp(arg_name, "") != 0) {
    out << " <" << std::right << arg_name << ">";
  }

  if (strcmp(desc, "") != 0)
    out << std::setw(25) << std::left << "\n" << desc;

  printf("%s\n", out.str().c_str());
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

  auto it = std::find_if(commands.begin(), commands.end(), [&](Option *opt) {
    return (strcmp(opt->verbose_name, argv[arg_index]) == 0) ||
           (strcmp(opt->short_name, argv[arg_index]) == 0);
  });

  if (it != commands.end()) {
    if (command_found) {
      fprintf(stderr, "A command has already been specified!\n");
      exit(EXIT_FAILURE);
    }

    in_args.in_command = *it;
    command_found = true;
    arg_index++;

    if ((*it)->callback)
      (*it)->callback();

    return *it;
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

  auto it = std::find_if(flags.begin(), flags.end(), [&](Option *opt) {
    return (strcmp(opt->verbose_name, argv[arg_index] + 2) == 0) ||
           (strcmp(opt->short_name, argv[arg_index] + 1) == 0);
  });

  if (it != flags.end()) {
    if ((*it)->arg_type != NONE) {
      (*it)->parseArg(argv[++arg_index]);
    }
    in_args.in_flags.push_back((*it));
    arg_index++;

    if ((*it)->callback)
      (*it)->callback();

    return *it;
  }

  fprintf(stderr, "Unknown flag found '%s'\n", argv[arg_index]);
  exit(EXIT_FAILURE);
}

void ArgParser::printUsage() {
  std::stringstream out;

  out << "Usage: " << m_name << (commands.size() > 0 ? " [COMMAND]" : "")
      << (flags.size() > 0 ? " [OPTIONS] ..." : "");

  printf("%s\n", out.str().c_str());

  if (commands.size() > 0) {
    printf("\nCommands:\n");
    for (auto cmd : commands) {
      cmd->printUsage();
    }
  }

  if (flags.size() > 0) {
    printf("\nOptions:\n");
    for (auto flag : flags) {
      flag->printUsage();
    }
  }
}

void ArgParser::addOption(OptionType type, const char *verbose_name,
                          const char *short_name, ArgType arg_type,
                          const char *arg_name) {
  if (type == COMMAND) {
    commands.push_back(new Option{type, verbose_name, short_name, NONE, ""});
    has_commands = true;
  } else {
    flags.push_back(
        new Option{type, verbose_name, short_name, arg_type, arg_name});
  }
}

void ArgParser::addOptionDesc(const char *opt_name, const char *desc) {
  auto it = std::find_if(commands.begin(), commands.end(), [&](Option *opt) {
    return strcmp(opt->verbose_name, opt_name) == 0;
  });
  if (it != commands.end()) {
    (*it)->desc = desc;
    return;
  }

  it = std::find_if(flags.begin(), flags.end(), [&](Option *opt) {
    return strcmp(opt->verbose_name, opt_name) == 0;
  });
  if (it != flags.end()) {
    (*it)->desc = desc;
    return;
  }

  fprintf(stderr, "No Option with specified name exists\n");
  exit(EXIT_FAILURE);
}

void ArgParser::addCallback(const char *opt_name, std::function<void()> func) {
  auto it = std::find_if(commands.begin(), commands.end(), [&](Option *opt) {
    return strcmp(opt->verbose_name, opt_name) == 0;
  });
  if (it != commands.end()) {
    (*it)->callback = func;
    return;
  }

  it = std::find_if(flags.begin(), flags.end(), [&](Option *opt) {
    return strcmp(opt->verbose_name, opt_name) == 0;
  });
  if (it != flags.end()) {
    (*it)->callback = func;
    return;
  }
}
