#include <iostream>
#include <png.h>
#include <zlib.h>
#include <errno.h>
#include <getopt.h>
#include <list>
#include <regex.h>
#include <algorithm>

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

/* Put inserts into png text chunks. inserts is assumed sorted and unique */
static int process_png(png_struct *read_png, png_info_struct *read_png_info, string out_file_name,
                        list<string> inserts) {
    FILE *write_fp;
    png_struct *write_png;
    string text_key;
    string text_value;
    int text_chunks;
    int num_text;
    png_text *text;
    png_text insert_text[1];
    list<string> current_inserts;
    int rc;

    if ((write_fp= fopen(out_file_name.c_str(), "wb")) == NULL) {
        cout << " Couldn't open output file " << out_file_name << endl;
        rc = -EIO;
    } else {
        write_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (write_png == NULL) {
            rc = -ENOMEM;
        } else {
            png_init_io(write_png, write_fp);
            text_chunks = png_get_text(read_png, read_png_info, &text, &num_text);
            for (int i=0; i<text_chunks; ++i) {
                current_inserts.push_back(text[i].text);
            } /* endfor */
            current_inserts.sort();
            current_inserts.unique();

            for (list<string>::iterator it=inserts.begin(); it!=inserts.end(); ++it) {
                list<string>::iterator ci;
                ci = find(current_inserts.begin(), current_inserts.end(), *it);
                if (ci == current_inserts.end()) {
                    char *cstr = new char[it->length()+1];
                    strncpy(cstr, it->c_str(), it->length());
                    insert_text[0].compression = PNG_TEXT_COMPRESSION_NONE;
                    insert_text[0].key = (char *)"insert_loc";
                    insert_text[0].text = cstr;
                    png_set_text(write_png, read_png_info, insert_text, 1);
                } /* endif */
            } /* endfor */
           png_write_png(write_png, read_png_info, PNG_TRANSFORM_IDENTITY, NULL);
           png_destroy_write_struct(&write_png, png_infopp_NULL);
        } /* endif */
        fclose(write_fp);
    } /* endif */
    return rc;
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

int process_png_inserts(string in_file, string out_file, list<string>insert_list) {
    png_struct *png;
    png_info_struct *png_info; /* png info for chunks before the image */
    png_info_struct *end_info; /* png info for chunks after the image */
    FILE *fp;
    int rc =0;

    if ((fp= fopen(in_file.c_str(), "rb")) == NULL) {
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
                        process_png(png, png_info, out_file, insert_list);
                        png_destroy_info_struct(png, &end_info);
                    } /* endif */
                    png_destroy_info_struct(png, &png_info);
                } /* endif */
                png_destroy_read_struct(&png, png_infopp_NULL, png_infopp_NULL);
            } /* endif */
        } /* endif */
        fclose(fp);
    } /* endif */
    return rc;
} /* process_png_inserts */

/* Display insertion points in the input file */
static void display_png_texts(png_struct *read_png, png_info_struct *read_png_info) {
    string text_key;
    string text_value;
    int text_chunks;
    int num_text;
    png_text *text;
    list<string> current_inserts;
    text_chunks = png_get_text(read_png, read_png_info, &text, &num_text);
    cout << "This file has " << text_chunks << " text chunks in it." << endl;
    for (int i=0; i<text_chunks; ++i) {
        string ct = text[i].text;
        cout << " Text keyword is " << text[i].key << " text value is " << ct << endl;
    } /* endfor */
} /* display_png_texts */

static int display_png_inserts(string in_file) {
    png_struct *png;
    png_info_struct *png_info; /* png info for chunks before the image */
    png_info_struct *end_info; /* png info for chunks after the image */
    FILE *fp;
    int rc =0;

    if ((fp= fopen(in_file.c_str(), "rb")) == NULL) {
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
                        display_png_texts(png, png_info);
                        png_destroy_info_struct(png, &end_info);
                    } /* endif */
                    png_destroy_info_struct(png, &png_info);
                } /* endif */
                png_destroy_read_struct(&png, png_infopp_NULL, png_infopp_NULL);
            } /* endif */
        } /* endif */
        fclose(fp);
    } /* endif */
    return rc;
} /* process_png_inserts */

int main(int argc, char* argv[]) {
    int rc =0;
    int opt =0;
    string insert_regex("[0-9]+\\x[0-9]+\\+[0-9]+\\+[0-9]+($|/[0-9]+$)");
    regex_t re;
    regmatch_t rm;
    list<string> insert_list;
    enum process_options {display_inserts, add_inserts, do_nothing};
    enum process_options p_opt = do_nothing;

    static struct option long_options[] = {
        { "insert-spec", required_argument, 0, 'w'},
        { "display-inserts", 0, 0, 'd'},
        { 0, 0, 0, 0 },
    }; /* long_options */
    int option_index = 1;

    if (regcomp(&re, insert_regex.c_str(), REG_EXTENDED) != 0) {
        cout << " Regular expression compile failed. Bad pattern." << endl;
        exit(-EINVAL);
    } /* endif */

    while ((opt = getopt_long(argc, argv, "w:d", long_options, &option_index)) != -1) {
        list<string>::iterator ci;
        switch(opt) {
        case 'd':
                /* display the list of insertion points from the file */
                p_opt = display_inserts;
            break;
            case 'w':
                /* add the option to the list of insertion points */
                if (regexec(&re, optarg, 1, &rm, 0) != 0) {
                    cout << " This insert item (" << optarg << ") is not Ok, should be {number}x{number}+{x offset}+{y offset}/{rotation angle}. " << endl;
                    p_opt = do_nothing;
                    exit(-EINVAL);
                }
                ci = find(insert_list.begin(), insert_list.end(), string(optarg));
                if (ci != insert_list.end()) {
                    cout << " This insert item \"" << optarg << "\" is a duplicate, check your list of -w arguments.." << endl;
                    p_opt = do_nothing;
                    exit(-EINVAL);
                }
                insert_list.push_back(string(optarg));
                p_opt = add_inserts;
                break;
        default:
            usage();
            return -1;
        } /* endswitch */
    } /* endwhile */
    insert_list.sort();
    insert_list.unique();

    if (((argc-optind) == 2) && (p_opt == add_inserts)) {
        rc =process_png_inserts(argv[optind], argv[optind+1], insert_list);
    } else {
        if (((argc-optind) < 2) && (p_opt == display_inserts)) {
            rc = display_png_inserts(argv[optind]);
        } else {
            usage();
            rc = 1;
        }
    } /* endif */
    return rc;
}

