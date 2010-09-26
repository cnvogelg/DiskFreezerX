#include <stdio.h>
#include <getopt.h>

#include "config.h"

config_t config = {
    .begin_track = 0,
    .end_track = 79,
    .sides = SIDES_ALL,

    .device = "/dev/spidev4.0",
    .speed = 6000000,
    .wait_blocks = 100 * 5,
    .max_blocks = 100 * 5,
    .spi_mode = 1,
    .cmd_timeout = 2000,

    .verbose = 0
};

typedef struct {
    const char *name;
    int id;
    const char *desc_opts;
    int min_opts;
    int max_opts;
    const char *desc;
} cmd_tab;

static cmd_tab commands[] = {
    { "exec",       CMD_EXEC,       "<cmd_seq>",1,1,    "execute commands directly on sampler" },
    { "read_trk",   CMD_READ_TRK,   "<raw_file> [debug_file]",1,2, "read a single track and store in file" },
    { "read_dsk",   CMD_READ_DSK,   "<raw_file>",1,1,   "read a disk and store in file" },
    { NULL, -1, NULL }
};

static void print_usage(const char *prog)
{
    printf("Usage: %s [switches] <command> [options]\n", prog);
    puts("\nSwitches:\n\n"
         "  -b <track>             begin track (0)\n"
         "  -e <track>             end track (79)\n"
         "  -s <sides>             t=top b=bottom a=all\n"
         "\n"
         "  -D --devicee <string>  spidev device to use (default /dev/spidev4.0)\n"
         "  -S --speed   <int>     spi speed (Hz)\n"
         "  -w           <int>     wait for n blocks until start\n"
         "  -n           <int>     track capture buffers\n"
         "  -m           <spimode> 0..3\n"
         "  -r           <ms>      wait for a slave ready\n"
         "  -v                     be verbose"
         "\nCommands:\n"
         );
    cmd_tab *tab = commands;
    while(tab->name != NULL) {
        printf("  %-12s  %-12s  %s\n", tab->name, tab->desc_opts, tab->desc);
        tab++;
    }
}

#define MAX_TRACK 84

int config_parse_opts(int argc, char *argv[])
{
    const char *prog = argv[0];
    
    while (1) {
        static const struct option lopts[] = {
            { "device",  1, 0, 'D' },
            { "speed",   1, 0, 's' },
            { NULL, 0, 0, 0 },
        };
        int c;
        c = getopt_long(argc, argv, "b:e:s:D:S:w:n:m:r:v", lopts, NULL);
        if (c == -1)
            break;

        switch (c) {
        case 'b':
            config.begin_track = atoi(optarg);
            if((config.begin_track<0)||(config.begin_track>MAX_TRACK)) {
                printf("ERROR: invalid begin track: %d\n", config.begin_track);
                print_usage(prog);
                return -5;
            }
            break;
        case 'e':
            config.end_track = atoi(optarg);
            if((config.begin_track<0)||(config.end_track>MAX_TRACK)) {
                printf("ERROR: invalid end track: %d\n", config.end_track);
                print_usage(prog);
                return -6;
            }
            break;
        case 's':
            config.sides = optarg[0];
            if((config.sides!=SIDES_ALL)&&
               (config.sides!=SIDES_TOP)&&
               (config.sides!=SIDES_BOTTOM)) {
                printf("ERROR: invalid sides given: %c", config.sides);
                print_usage(prog);
                return -7;                   
            }
            break;
            
        case 'D':
            config.device = optarg;
            break;
        case 'S':
            config.speed = atoi(optarg);
            break;
        case 'w':
            config.wait_blocks = atoi(optarg);
            break;
        case 'n':
            config.max_blocks = atoi(optarg);
            break;
        case 'm':
            config.spi_mode = atoi(optarg);
            break;
        case 'r':
            config.cmd_timeout = atoi(optarg);
            break;
        case 'v':
            config.verbose ++;
            break;
        default:
            printf("ERROR: invalid option: '%c'\n", c);
            print_usage(prog);
            return -1;
        }
    }
    
    argc -= optind;
    argv += optind;

    // checks
    if(config.begin_track > config.end_track) {
        printf("ERROR: track range invalid: [%d:%d]\n",config.begin_track, config.end_track);
        print_usage(prog);
        return -8;
    }

    // check for command
    if(argc == 0) {
        printf("ERROR: no command given!\n");
        print_usage(prog);
        return -2;
    }

    // store args    
    config.args = &argv[1];
    config.num_args = argc-1;

    const char *cmd_str = argv[0];

    // parse command
    cmd_tab *tab = commands;
    while(tab->name != NULL) {
        if(strcmp(tab->name,cmd_str) == 0) {
            if((config.num_args < tab->min_opts)||(config.num_args > tab->max_opts)) {
                printf("ERROR: invalid number of options for command!\n");
                print_usage(prog);
                return -3;
            } else {
                // command found!
                config.cmd = tab->id;
                return 0;
            }
        }
        tab++;
    }
    
    // unknown command
    printf("ERROR: unknown command '%s'\n", cmd_str);
    print_usage(prog);
    return -4;
}
