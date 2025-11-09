#include "math.h"
#include "parser.h"

int main(int argc, const char *argv[]) {
  ArgParser parser("parser", argc, argv);
  parser.addOption(COMMAND, "test");
  parser.addOptionDesc("test", "Test command");

  parser.parseCommand();

  parser.addOption(FLAG, "number", "n", NUM, "frame_count");
  parser.addOption(FLAG, "text", "t", STR, "ghost.rkg");
  parser.addOption(FLAG, "arg");
  parser.addOptionDesc("framecount", "Specify a number of frames");
  parser.addOptionDesc("ghost", "Specify a ghost");

  while (parser.canParse()) {
    Option *flag = parser.parseFlags();
    if (flag->arg_type == NUM) {
      printf("Flag: '%s' found with numeric arg %li\n", flag->verbose_name,
             flag->arg_num);
    } else if (flag->arg_type == STR) {
      printf("Flag: '%s' found with string arg %s\n", flag->verbose_name,
             flag->arg_str);
    } else {
      printf("Flag: '%s' found\n", flag->verbose_name);
    }
  }

  return 0;
}
