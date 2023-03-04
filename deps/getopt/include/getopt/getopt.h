/* a getopt.
   version 0.1, march, 2012

   Copyright (C) 2012- Fredrik Kihlander

   https://github.com/wc-duck/getopt

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
      claim that you wrote the original software. If you use this software
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.

   Fredrik Kihlander
*/

#ifndef GETOPT_GETOPT_H_INCLUDED
#define GETOPT_GETOPT_H_INCLUDED

#include <stddef.h>

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * @file getopt.h
 *
 * Provides functionality to parse standard argc/argv in an easy manner.
 *
 * @example of simple parse
 *
 *   // ... first we need to define our options, these need to be valid during the entire commandline-parse ...
 *   const getopt_option_t option_list[] =
 *   {
 *       { "help",    'h', GETOPT_OPTION_TYPE_NO_ARG,   0x0,      'h', "print this help text",       0x0 },
 *       { "verbose", 'v', GETOPT_OPTION_TYPE_FLAG_SET, &verbose,  1,  "verbose logging enabled",    0x0 },
 *       { "input",   'i', GETOPT_OPTION_TYPE_REQUIRED, 0x0,      'i', "an input file",           "FILE" },
 *       GETOPT_OPTIONS_END
 *   };
 *
 *   // ... next we need to initialize our parse-context ...
 * 	 getopt_context_t ctx;
 *   if( getopt_create_context( &ctx, argc, argv, option_list ) < 0 )
 *   {
 *       printf( "error while creating getopt ctx, bad options-list?" );
 *       return 1;
 *   }
 *
 *   // ... time to get parsin! ...
 *   int opt;
 *
 *   while( ( opt = getopt_next( &ctx ) ) != -1 )
 *   {
 *       switch( opt )
 *       {
 *           case '+': printf( "got argument without flag: %s\n",   ctx.current_opt_arg ); break;
 *           case '?': printf( "unknown flag %s\n",                 ctx.current_opt_arg ); break;
 *           case '!': printf( "invalid use of flag %s\n",          ctx.current_opt_arg ); break;
 *           case 'i': printf( "got -i or --input with value %s\n", ctx.current_opt_arg ); break;
 *           case   0: printf( "flag was set!\n"); break;
 *           case 'h': print_help_string( ctx ); break;
 *           default: break;
 *       }
 *   }
 */

/**
 * option types supported by system.
 */
typedef enum getopt_option_type
{
	GETOPT_OPTION_TYPE_NO_ARG,         ///< The option can have no argument
	GETOPT_OPTION_TYPE_REQUIRED,       ///< The option requires an argument (--option=arg, -o arg)
	GETOPT_OPTION_TYPE_OPTIONAL,       ///< The option-argument is optional
	GETOPT_OPTION_TYPE_REQUIRED_INT32, ///< The option requires an argument and this argument has to be parseable as an int (--option=arg, -o arg)
	GETOPT_OPTION_TYPE_OPTIONAL_INT32, ///< The option-argument is optional, but if it is there it has to be parseable as int.
	GETOPT_OPTION_TYPE_REQUIRED_FP32,  ///< The option requires an argument and this argument has to be parseable as an float (--option=arg, -o arg)
	GETOPT_OPTION_TYPE_OPTIONAL_FP32,  ///< The option-argument is optional, but if it is there it has to be parseable as float.
	GETOPT_OPTION_TYPE_FLAG_SET,       ///< The option is a flag and value will be set to flag
	GETOPT_OPTION_TYPE_FLAG_AND,       ///< The option is a flag and value will be and:ed with flag
	GETOPT_OPTION_TYPE_FLAG_OR         ///< The option is a flag and value will be or:ed with flag
} getopt_option_type_t;

/**
 * Helper-macro to define end-element in options-array.
 * Mostly helpful on higher warning-level where compiler would complain for { 0 }
 */
#define GETOPT_OPTIONS_END { 0, 0, GETOPT_OPTION_TYPE_NO_ARG, 0, 0, 0, 0 }

/**
 * Option definition in system.
 */
typedef struct getopt_option
{
	const char*          name;       ///< Long name of argument, set to NULL if only short name is valid.
	int                  name_short; ///< Short name of argument, set to 0 if only long name is valid.
	getopt_option_type_t type;       ///< Type of option, see <getopt_option_type>.
	int*                 flag;       ///< Pointer to flag to set if option is of flag-type, set to null NULL if option is not of flag-type.
	int                  value;      ///< If option is of flag-type, this value will be set/and:ed/or:ed to the flag, else it will be returned from getopt_next when option is found.
	const char*          desc;       ///< Description of option, used when generating help-text.
	const char*          value_desc; ///< Short description of valid values to the option, will only be used when generating help-text. example: "--my_option=<value_desc>"
} getopt_option_t;

/**
 * Context used while parsing options.
 * Need to be initialized by <getopt_create_context> before usage. If reused a re-initialization by <getopt_create_context> is needed.
 *
 * @note: Do not modify data in this struct manually!
 */
typedef struct getopt_context
{
	int                    argc;            ///< Internal variable
	const char**           argv;            ///< Internal variable
	const getopt_option_t* opts;            ///< pointer to 'opts' passed in getopt_create_context().
	int                    num_opts;        ///< number of valid options in 'opts'
	int                    current_index;   ///< Internal variable

	/**
	 * Used to return values. Will point to a string that is the argument to the currently parsed option.
	 * I.e. when parsing '--my-flag whoppa_doppa", this will point to "whoppa doppa".
	 * 
	 * If the option is of type GETOPT_OPTION_TYPE_OPTIONAL this will be set to NULL if there was no argument passed.
	 */
	const char*            current_opt_arg;

	/**
	 * Union storeing parsed values if that is requested by the option-type.
	 * @note on parse-errors '!' will be returned from getopt_next() and current_opt_arg will be set to the name
	 *       of the option that failed to parse.
	 * @note if the option is 'optional' and there was no arg, current_opt_arg will be 0x0.
	 */
	union
	{
		/**
		 * if the option is on type GETOPT_OPTION_TYPE_OPTIONAL_INT or GETOPT_OPTION_TYPE_REQUIRED_INT and it parsed
		 * successfully the value will be stored here.
		 * supported int formats are, decimal, hex and octal (123, 0x123, 0123)
		 */
		int   i32;

		/**
		 * if the option is on type GETOPT_OPTION_TYPE_OPTIONAL_FP32 or GETOPT_OPTION_TYPE_REQUIRED_FP32 and it parsed
		 * successfully the value will be stored here.
		 */
		float fp32;
	} current_value;

} getopt_context_t;

/**
 * Initializes an getopt_context_t-struct to be used by <getopt_next>
 *
 * @param ctx  Pointer to context to initialize.
 * @param argc argc from "int main(int argc, char** argv)" or equal.
 * @param argv argv from "int main(int argc, char** argv)" or equal. Data need to be valid during option-parsing and usage of data.
 * @param opts Pointer to array with options that should be looked for. Should end with an option that is all zeroed!
 *
 * @return 0 on success, otherwise error-code.
 */
int getopt_create_context( getopt_context_t* ctx, int argc, const char** argv, const getopt_option_t* opts );

/**
 * Used to parse argc/argv with the help of a getopt_context_t.
 * Tries to parse the next token in ctx and return id depending on status.
 *
 * @param ctx Pointer to a initialized <getopt_context_t>
 *
 * @return '!' on error. ctx->current_opt_arg will be set to flag-name! Errors that can occur,
 *             Argument missing if argument is required or Argument found when there should be none.
 *         '?' if item was an unrecognized option, ctx->current_opt_arg will be set to item!
 *         '+' if item was no option, ctx->current_opt_arg will be set to item!
 *         '0' if the opt was a flag and it was set. ctx->current_opt_arg will be set to flag-name!
 *             the value stored is value in the found option.
 *         -1 no more options to parse!
*/
int getopt_next( getopt_context_t* ctx );

/**
 * Builds a string that describes all options for use with the --help-flag etc.
 *
 * @param ctx         Pointer to a initialized <getopt_context_t>
 * @param buffer      Pointer to buffer to build string in.
 * @param buffer_size Size of buffer.
 *
 * @return buffer filled with a help-string.
 */
const char* getopt_create_help_string( getopt_context_t* ctx, char* buffer, size_t buffer_size );

#if defined (__cplusplus)
}
#endif

#endif
