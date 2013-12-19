#include <iostream>
#include <png.h>
#include <zlib.h>
#include <errno.h>
#include <getopt.h>

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

static void process_png(png_struct *png, png_info_struct *info) {
    cout << " Got a png file into memory" << endl;
    string text_key;
    string text_value;
    int text_chunks;
    int num_text;
    png_text *text;

    text_chunks = png_get_text(png, info, &text, &num_text);
    if (text_chunks > 0) {
        cout << "This file has " << text_chunks << " text chunks in it." << endl;
        for (int i=0; i<text_chunks; ++i) {
            cout << " Text keyword is " << text[i].key << " text value is " << text[i].text << endl;
        } /* endfor */
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

int main(int argc, char* argv[]) {
    unsigned char file_signature[8];
    png_struct *png;
    png_info_struct *png_info; /* png info for chunks before the image */
    png_info_struct *end_info; /* png info for chunks after the image */
    FILE *fp;
    int rc =0;

    static struct option long_options[] = {
        { "insert-spec", required_argument, 0, 'w'},
        { 0, 0, 0, 0 },
    }; /* long_options */
    int option_index = 0;

    while (true) {
        rc=getopt_long(argc, argv, "-w", long_options, &option_index);
        if (rc == -1) {
            break;
        } else {
            switch(rc) {
            case 'w':
                /* add the option to the list of insertion points */
                break;
            default:
                usage();
                return -1;
            }
        } /* endif */
    } /* endwhile */
    cout << argv[0] << " Starting" << endl;
    if (argc < 4) {
        usage();
        return 1;
    } /* endif */
    if ((fp= fopen(argv[1], "rb")) == NULL) {
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
                        process_png(png, png_info);
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

