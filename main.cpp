#include <iostream>
#include <png.h>
#include <zlib.h>
#include <errno.h>
#include <getopt.h>
#include <list>
#include <regex.h>

using namespace std;

static void png_add_inserts_version_info() {
    cout << "   Compiled with libpng " << PNG_LIBPNG_VER_STRING <<
            " using libpng " << png_libpng_ver << "." << endl;
    cout << "   Compiled with zlib " << ZLIB_VERSION <<
            " using zlib " << zlib_version << "." << endl;
} /* png_add_inserts_version_info */

static void usage(void) {
    png_add_inserts_version_info();
    cout << " Usage is png_add_inserts input_file output_file -w geometry_spec -w geometry_spec ..." << endl;
} /* usage */

static void process_png(png_struct *read_png, png_info_struct *read_png_info, char *out_file_name) {
    FILE *write_fp;
    png_struct *write_png;
    png_info_struct *write_png_info; /* png info for chunks before the image */
    png_info_struct *write_end_info; /* png info for chunks after the image */
    string text_key;
    string text_value;
    int text_chunks;
    int num_text;
    png_text *text;
    int rc;

    cout << " Got a png file into memory" << endl;

    if ((write_fp= fopen(out_file_name, "wb")) == NULL) {
        cout << " Couldn't open output file " << out_file_name << endl;
        rc = -EIO;
    } else {
        write_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (write_png == NULL) {
            rc = -ENOMEM;
        } else {
            write_png_info = png_create_info_struct(write_png);
            if (write_png_info == NULL) {
                rc = -ENOMEM;
            } else {
                write_end_info = png_create_info_struct(write_png);
                if (write_end_info == NULL) {
                    rc = -ENOMEM;
                } else {
                    png_init_io(write_png, write_fp);
                    text_chunks = png_get_text(read_png, read_png_info, &text, &num_text);
                    if (text_chunks > 0) {
                        cout << "This file has " << text_chunks << " text chunks in it." << endl;
                        for (int i=0; i<text_chunks; ++i) {
                            cout << " Text keyword is " << text[i].key << " text value is " << text[i].text << endl;
                        } /* endfor */
                    } /* endif */
                    png_destroy_info_struct(write_png, &write_end_info);
                } /* endif */
                png_destroy_info_struct(write_png, &write_png_info);
            } /* endif */
            png_destroy_read_struct(&write_png, png_infopp_NULL, png_infopp_NULL);
        } /* endif */
        fclose(write_fp);
    } /* endif */
} /* process_png */

static bool is_png_file(FILE *fp, int *bytes_read) {
    unsigned char sig_bytes[8];
    int rc;
    if (fread(sig_bytes, 1, 8, fp) != 8) {
        rc = false;
    } else {
        if (png_sig_cmp(sig_bytes, 0, 8) == 0) {
            rc = true;
        } else {
            rc = false;
        } /* endif */
        fseek(fp, 0, SEEK_SET); /* put the file back at the beginning */
    } /* endif */
    return rc;
} /* is_png_file */

static list<string> insert_list;

int main(int argc, char* argv[]) {
    unsigned char file_signature[8];
    png_struct *png;
    png_info_struct *png_info; /* png info for chunks before the image */
    png_info_struct *end_info; /* png info for chunks after the image */
    FILE *fp;
    int rc =0;
    int opt =0;
    string insert_regex("[0-9]+\\x[0-9]+\\+[0-9]+\\+[0-9]+($|/[0-9]+$)");
    regex_t re;
    regmatch_t rm;

    static struct option long_options[] = {
        { "insert-spec", required_argument, 0, 0},
        { 0, 0, 0, 0 },
    }; /* long_options */
    int option_index = 1;

    cout << argv[0] << " Starting" << endl;

    if (regcomp(&re, insert_regex.c_str(), REG_EXTENDED) != 0) {
        cout << " Regular expression compile failed. Bad pattern." << endl;
        exit(-EINVAL);
    } /* endif */

    while ((opt = getopt_long(argc, argv, "w:", long_options, &option_index)) != -1) {
        switch(opt) {
        case 'w':
            /* add the option to the list of insertion points */
            if (regexec(&re, optarg, 1, &rm, 0) != 0) {
                cout << " This item is not Ok " << optarg << endl;
                cout << "match " <<  string(optarg+rm.rm_so, optarg+rm.rm_eo) << endl;
            }

            insert_list.push_back(string(optarg));
            break;
        default:
            usage();
            return -1;
        } /* endswitch */
    } /* endwhile */

    if ((argc-optind) < 2) {
        usage();
        return 1;
    } /* endif */
    if ((fp= fopen(argv[optind], "rb")) == NULL) {
        rc = -EIO;
    } else {
        int bytes_read;
        if (!is_png_file(fp, &bytes_read)) {
            rc = -EIO;
        } else {
            png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (png == NULL) {
                rc = -ENOMEM;
            } else {
                png_info = png_create_info_struct(png);
                if (png_info == NULL) {
                    rc = -ENOMEM;
                } else {
                    end_info = png_create_info_struct(png);
                    if (end_info == NULL) {
                        rc = -ENOMEM;
                    } else {
                        png_init_io(png, fp);
                        png_read_png(png, png_info, PNG_TRANSFORM_IDENTITY, png_voidp_NULL);
                        process_png(png, png_info, argv[optind+1]);
                        png_destroy_info_struct(png, &end_info);
                    } /* endif */
                    png_destroy_info_struct(png, &png_info);
                } /* endif */
                png_destroy_read_struct(&png, png_infopp_NULL, png_infopp_NULL);
            } /* endif */
        } /* endif */
        fclose(fp);
    } /* endif */
    cout << argv[0] << " Ending return code =" << rc << endl;
    return rc;
}

