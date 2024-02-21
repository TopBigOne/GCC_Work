//
// Created by DEV on 10/24/23.
//

#ifndef CRYPTO_DEMO_TESTZLIB_H
#define CRYPTO_DEMO_TESTZLIB_H

#include <zlib.h>
#include <stdio.h>


#include <string.h>

class TestZlib {

public:

    int decompress_one_file(char *infilename, char *outfilename);

    unsigned long file_size(char *filename);


    int compress_one_file(char *infilename, char *outfilename);


};


#endif //CRYPTO_DEMO_TESTZLIB_H
