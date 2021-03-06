/*
 * ============================================================================
 *
 *       Filename:  axe_main.c
 *    Description:  Main loop for axe
 *      Copyright:  2014-2015 Kevin Murray <spam@kdmurray.id.au>
 *        License:  GNU GPL v3+
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 */


#include "axe.h"

#include <getopt.h>

static void
print_version(void)
{
    fprintf(stderr, "AXE Version %s\n", AXE_VERSION);
}

static void
print_help(void)
{
    fprintf(stderr, "All mandatory short options are mandatory in their\n");
    fprintf(stderr, "long option form. Likewise, all short options that take\n");
    fprintf(stderr, "an argument must be given an argument in their long form\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "If a forward read input is given, a forward read output\n");
    fprintf(stderr, "must be. Likewise for a reverse/interleaved input. If either\n");
    fprintf(stderr, "forward and/or reverse reads are given, interleaved input\n");
    fprintf(stderr, "cannot be. However, one can input interleaved paired reads\n");
    fprintf(stderr, "and output separate forwards and reverse reads, and vice versa.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The barcode file is a tab-separated tabular file with an\n");
    fprintf(stderr, "optional header, and has two alternative formats. The standard\n");
    fprintf(stderr, "form (see below) is expected unless --combinatorial is given.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The standard format is:\n");
    fprintf(stderr, "Barcode\tID\n");
    fprintf(stderr, "ACTA\tA1\n");
    fprintf(stderr, "CCTC\tA2\n");
    fprintf(stderr, "...\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The combinatorial format is:\n");
    fprintf(stderr, "Barcode1\tBarcode2\tID\n");
    fprintf(stderr, "ACTA\tACGC\tA1\n");
    fprintf(stderr, "CCTC\tTCTA\tA2\n");
    fprintf(stderr, "...\n");
    fprintf(stderr, "\n");
}

static void
print_usage(void)
{
    print_version();
    fprintf(stderr, "\nUSAGE:\n");
    fprintf(stderr, "axe-demux [-mzc2pt] -b (-f [-r] | -i) (-F [-R] | -I)\n");
    fprintf(stderr, "axe-demux -h\n");
    fprintf(stderr, "axe-demux -v\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "    -m, --mismatch\tMaximum hamming distance mismatch. [int, default 1]\n");
    fprintf(stderr, "    -z, --ziplevel\tGzip compression level, or 0 for plain text [int, default 0]\n");
    fprintf(stderr, "    -c, --combinatorial\tUse combinatorial barcode matching. [flag, default OFF]\n");
    fprintf(stderr, "    -p, --permissive\tDon't error on barcode mismatch confict, matching only\n");
    fprintf(stderr, "                    \texactly for conficting barcodes. [flag, default OFF]\n");
    fprintf(stderr, "    -2, --trim-r2\tTrim barcode from R2 read as well as R1. [flag, default OFF]\n");
    fprintf(stderr, "    -b, --barcodes\tBarcode file. See --help for example. [file]\n");
    fprintf(stderr, "    -f, --fwd-in\tInput forward read. [file]\n");
    fprintf(stderr, "    -F, --fwd-out\tOutput forward read prefix. [file]\n");
    fprintf(stderr, "    -r, --rev-in\tInput reverse read. [file]\n");
    fprintf(stderr, "    -R, --rev-out\tOutput reverse read prefix. [file]\n");
    fprintf(stderr, "    -i, --ilfq-in\tInput interleaved paired reads. [file]\n");
    fprintf(stderr, "    -I, --ilfq-out\tOutput interleaved paired reads prefix. [file]\n");
    fprintf(stderr, "    -t, --table-file\tOutput a summary table of demultiplexing statistics to file. [file]\n");
    fprintf(stderr, "    -h, --help\t\tPrint this usage plus additional help.\n");
    fprintf(stderr, "    -V, --version\tPrint version string.\n");
    fprintf(stderr, "    -v, --verbose\tBe more verbose. Additive, -vv is more vebose than -v.\n");
    fprintf(stderr, "    -q, --quiet\t\tBe very quiet.\n");
    fprintf(stderr, "\n");
}

static const char *axe_opts = "m:z:c2pb:f:F:r:R:i:I:t:hVvqd";
static const struct option axe_longopts[] = {
    { "mismatch",   optional_argument,  NULL,   'm' },
    { "ziplevel",   required_argument,  NULL,   'z' },
    { "combinatorial", no_argument,     NULL,   'c' },
    { "trim-r2",    no_argument,        NULL,   '2' },
    { "permissive", no_argument,        NULL,   'p' },
    { "barcodes",   required_argument,  NULL,   'b' },
    { "fwd-in",     required_argument,  NULL,   'f' },
    { "fwd-out",    required_argument,  NULL,   'F' },
    { "rev-in",     required_argument,  NULL,   'r' },
    { "rev-out",    required_argument,  NULL,   'R' },
    { "ilfq-in",    required_argument,  NULL,   'i' },
    { "ilfq-out",   required_argument,  NULL,   'I' },
    { "table-file", required_argument,  NULL,   't' },
    { "help",       no_argument,        NULL,   'h' },
    { "version",    no_argument,        NULL,   'V' },
    { "verbose",    no_argument,        NULL,   'v' },
    { "debug",      no_argument,        NULL,   'd' },
    { NULL,         0,                  NULL,    0  }
};

static int
parse_args(struct axe_config *config, int argc, char * const *argv)
{
    int c = 0;
    int optind = 0;

    if (argc < 2 ) {
        return 1;
    }
    if (!axe_config_ok(config) || argc < 1 || argv == NULL) {
        goto error;
    }
    /* Set some sane defaults */
    /* Most things will default to 0 as we `calloc` the config struct, so we
     * don't need to explicity set them. */
    config->mismatches = 1;
    config->verbosity = 0;
    config->out_compress_level = 0;
    /* Parse argv using getopt */
    while ((c = getopt_long(argc, argv, axe_opts, axe_longopts, &optind)) > 0){
        switch (c) {
            case 'm':
                config->mismatches = atol(optarg);
                break;
            case 'z':
                config->out_compress_level = atoi(optarg);
                break;
            case 'c':
                config->match_combo |= 1;
                break;
            case 'p':
                config->permissive |= 1;
                break;
            case '2':
                config->trim_rev |= 1;
                break;
            case 'b':
                config->barcode_file = strdup(optarg);
                break;
            case 'f':
                if (config->in_mode == READS_INTERLEAVED) {
                    goto error;
                    break;
                }
                config->infiles[0] = strdup(optarg);
                if (config->in_mode == READS_UNKNOWN) {
                    config->in_mode = READS_SINGLE;
                }
                break;
            case 'F':
                config->out_prefixes[0] = strdup(optarg);
                config->out_mode = READS_SINGLE;
                break;
            case 'r':
                if (config->in_mode == READS_INTERLEAVED) {
                    goto error;
                    break;
                }
                config->infiles[1] = strdup(optarg);
                config->in_mode = READS_PAIRED;
                break;
            case 'R':
                config->out_prefixes[1] = strdup(optarg);
                config->out_mode = READS_PAIRED;
                break;
            case 'i':
                config->infiles[0] = strdup(optarg);
                config->in_mode = READS_INTERLEAVED;
                break;
            case 'I':
                config->out_prefixes[0] = strdup(optarg);
                config->out_mode = READS_INTERLEAVED;
                break;
            case 't':
                config->table_file = strdup(optarg);
                break;
            case 'h':
                goto help;
            case 'V':
                goto version;
            case 'v':
                config->verbosity += 1;
                break;
            case 'q':
                config->verbosity -= 1;
                break;
            case 'd':
                config->debug = 1;
                break;
            case '?':
            default:
                /* Getopt long prints its own error msg */
                goto error;
        }
    }
    /* Check options are sane */
    if (config->barcode_file == NULL) {
        fprintf(stderr, "ERROR: Barcode file must be provided\n");
        goto error;
    }
    if (config->mismatches > 4) {
        fprintf(stderr, "ERROR: Silly mismatch level %zu\n",
                config->mismatches);
        goto error;
    }
    if (config->in_mode == READS_UNKNOWN) {
        fprintf(stderr, "ERROR: Input file(s) must be provided\n");
        goto error;
    }
    if (config->infiles[0] == NULL) {
        switch (config->in_mode) {
            case READS_SINGLE:
                fprintf(stderr, "ERROR: Setting forward read input file failed.\n");
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Forward read file must be provided.\n");
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Setting interleaved input file failed.\n");
                break;
            case READS_UNKNOWN:
            default:
                break;
        }
        goto error;
    }
    if (config->infiles[1] == NULL) {
        switch (config->in_mode) {
            case READS_SINGLE:
            case READS_INTERLEAVED:
                /* Not an error */
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Setting revese read input file failed.\n");
                goto error;
                break;
            case READS_UNKNOWN:
            default:
                goto error;
                break;
        }
    }
    if (config->infiles[1] != NULL) {
        switch (config->in_mode) {
            case READS_PAIRED:
                /* Not an error */
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Revese read input file set in interleaved mode.\n");
                goto error;
                break;
            case READS_SINGLE:
                fprintf(stderr, "ERROR: Revese read input file set in single-end mode.\n");
                goto error;
                break;
            case READS_UNKNOWN:
            default:
                /* Misc weirdness */
                goto error;
                break;
        }
    }
    if (config->out_prefixes[0] == NULL) {
        switch (config->out_mode) {
            case READS_SINGLE:
                fprintf(stderr, "ERROR: Setting forward read output prefix failed.\n");
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Forward read prefix must be provided.\n");
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Setting interleaved output prefix failed.\n");
                break;
            case READS_UNKNOWN:
            default:
                break;
        }
        goto error;
    }
    if (config->out_prefixes[1] == NULL) {
        switch (config->out_mode) {
            case READS_SINGLE:
            case READS_INTERLEAVED:
                /* Not an error */
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Setting revese read output prefix failed.\n");
                goto error;
                break;
            case READS_UNKNOWN:
            default:
                goto error;
                break;
        }
    }
    if (config->out_prefixes[1] != NULL) {
        switch (config->out_mode) {
            case READS_PAIRED:
                /* Not an error */
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Revese read output prefix set in interleaved mode.\n");
                goto error;
                break;
            case READS_SINGLE:
            case READS_UNKNOWN:
            default:
                /* Misc weirdness */
                goto error;
                break;
        }
    }
    config->have_cli_opts = 1;
    format_call_number = 0;
    qes_logger_init(config->logger, "[axe] ", QES_LOG_DEBUG);
    qes_logger_add_destination_formatted(config->logger, stderr, QES_LOG_DEBUG,
                                         &axe_formatter);
    return 0;
error:
    fprintf(stderr,
            "Axe failed due to bad CLI flags. Consult the usage below please!\n\n");
    config->have_cli_opts = 0;
    return 1;
help:
    config->have_cli_opts = 0;
    return 2;
version:
    print_version();
    axe_config_destroy(config);
    exit(0);
}

int
main (int argc, char * const *argv)
{
    int ret = 0;
    struct axe_config *config = axe_config_create();

    if (config == NULL) {
        ret = EXIT_FAILURE;
        goto end;
    }
    ret = parse_args(config, argc, argv);
    if (ret != 0) {
        print_usage();
        if (ret == 2) {
            print_help();
        }
        goto end;
    }
    ret = axe_read_barcodes(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_read_barcodes returned %i\n", ret);
        fprintf(stderr, "\tThis indicates that the barcode file is invalid.\n");
        fprintf(stderr, "\tPlease check that it conforms to the layout described in the help message\n");
        goto end;
    }
    ret = axe_setup_barcode_lookup(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_setup_barcode_lookup returned %i\n",
                ret);
        goto end;
    }
    ret = axe_make_tries(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_make_tries returned %i\n", ret);
        goto end;
    }
    ret = axe_load_tries(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_load_tries returned %i\n", ret);
        goto end;
    }
    ret = axe_make_outputs(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_make_outputs returned %i\n", ret);
        goto end;
    }
    ret = axe_process_file(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_process_file returned %i\n", ret);
        goto end;
    }
    ret = axe_print_summary(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_print_summary returned %i\n", ret);
        goto end;
    }
    ret = axe_write_table(config);
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: axe_write_table returned %i\n", ret);
        goto end;
    }
end:
    axe_config_destroy(config);
    return ret;
}
