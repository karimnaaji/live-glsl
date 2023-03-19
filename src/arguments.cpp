#include "arguments.h"

#include <getopt/getopt.h>

enum Option {
    OPTION_INPUT = 1,
    OPTION_OUTPUT,
    OPTION_WIDTH,
    OPTION_HEIGHT,
    OPTION_NO_INI
};

static const getopt_option_t option_list[] = {
    { "input",  'i', GETOPT_OPTION_TYPE_REQUIRED, 0, OPTION_INPUT,  "input source file", "GLSL file" },
    { "output", 'o', GETOPT_OPTION_TYPE_REQUIRED, 0, OPTION_OUTPUT, "output source file", "PNG file" },
    { "width",  'w', GETOPT_OPTION_TYPE_REQUIRED, 0, OPTION_WIDTH,  "viewport width, in pixels (default 800)" },
    { "height", 'h', GETOPT_OPTION_TYPE_REQUIRED, 0, OPTION_HEIGHT, "viewport height, in pixels (default 600)" },
    { "ini",      0, GETOPT_OPTION_TYPE_REQUIRED, 0, OPTION_NO_INI, "Whether to enable ini shader file to save GUI presets (default true)" },
    GETOPT_OPTIONS_END
};

bool ArgumentsParse(int argc, const char** argv, Arguments& args) {
    getopt_context_t ctx;
    if (getopt_create_context(&ctx, argc, argv, option_list) < 0) {
        printf( "error while creating getopt ctx, bad options-list?" );
        return false;
    }

    int opt = 0;
    while ((opt = getopt_next(&ctx)) != -1) {
        switch (opt) {
            case '+':
                printf("live-glsl: got argument without flag: %s\n", ctx.current_opt_arg);
                return false;
            case '?':
                printf("live-glsl: unknown flag %s\n", ctx.current_opt_arg);
                return false;
            case '!':
                printf("live-glsl: invalid use of flag %s\n", ctx.current_opt_arg);
                return false;
            case OPTION_INPUT:
                args.Input = ctx.current_opt_arg;
                break;
            case OPTION_OUTPUT:
                args.Output = ctx.current_opt_arg;
                break;
            case OPTION_WIDTH:
                args.Width = atoi(ctx.current_opt_arg);
                break;
            case OPTION_HEIGHT:
                args.Height = atoi(ctx.current_opt_arg);
                break;
            case OPTION_NO_INI:
                args.EnableIni = (bool)atoi(ctx.current_opt_arg);
                break;
            default:
                break;
        }
    }
    if (args.Input.empty()) {
        printf("live-glsl: no input file (--input [path])\n");
        return false;
    }
    return true;
}
