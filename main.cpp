#include <iostream>
#include <png.h>
#include <zlib.h>

using namespace std;

void png_add_inserts_version_info() {
    cout << "   Compiled with libpng " << PNG_LIBPNG_VER_STRING <<
            " using libpng " << png_libpng_ver << "." << endl;
    cout << "   Compiled with zlib " << ZLIB_VERSION <<
            " using zlib " << zlib_version << "." << endl;
}

int main() {
    cout << "Hello World!" << endl;
    png_add_inserts_version_info();
    return 0;
}

