# Hibyehello's Command-line Argument Parser

This is a super basic argument parser, built to be easy to use and relatively flexible.

## Defining Arguments

Defining a `COMMAND` option is as easy as

> `parser.addOption(COMMAND, "test", "t");`

`COMMAND` is used to determine that the argument is a `COMMAND` option

**NOTE** `COMMAND` options when defined are required before `FLAG` options when using `parseArgs`, as it is expected that `FLAG` options
are used to modify the command.

*For more info on any structs refer to the bottom of this README and parser.h.*

Defining a `FLAG` option is as easy as

> `parser.addOption(FLAG, "number", "n", NUM, "number");`

`NUM` is used to define that the argument is a number, while `STR` is used to specify that it's a string.
`"number"` is the name for use in the usage guide to represent the argument for the option.

> `parser.addOptionDesc("test", "Test Command");`

The first argument corresponds to an option that has already been added, and the second option is the description to print in the usage help prompt.

## Parsing Arguments

There are two options for parsing Options

1. 
  > `parser.parseArgs();`
  
  This will loop through all arguments and store them in the `Arg` struct, which has two members, `in_command` and `in_flags`. The `Arg` struct stores a single command and a vector of flags, that you can can then apply your own logic to.

2. 
  > `parser.parseCommand();`
  
  This returns the next `COMMAND` option in the arguments, and it also also takes an option boolean that will reset the parsers command check to allow for using multiple commands.
  
  This function increments the parsers argument index location.
  
  > `parser.parseArgs();`

  This returns the next `FLAG` option in the arguments that you can then fetch the arguments (if specified to hold an argument).
  
  This function increments the parsers argument index location.
  
  > `parser.canParse();`
  
  This returns true if there is still more arguments to parse.
  
## Usage help

`--help` and `-h` are built in commands that call printUsage, which is automatically handled by the parser.

**NOTE** `--help` and `-h` only work as the first argument, which allows for adding custom usage prompts per command.

### Example Usage output

```
Usage: parser [COMMAND] [OPTIONS] ...

Commands:
  test
                        Test command

Options:
  --number, -n <number>
                        Send a number to the program
  --text, -t <string>
                        Send text to the program
  --arg
                        Example Arg
```

## Structs

``` 
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
```

```
struct Arg {
  Option *in_command;
  std::vector<Option *> in_flags;
};
```
