//
// Created by DEV on 10/24/23.
//

#include "TestZlib.h"


int TestZlib::decompress_one_file(char *infilename, char *outfilename) {
    int    num_read    = 0;
    char   buffer[128] = {0};
    gzFile infile      = gzopen(infilename, "rb");
    FILE   *outfile    = fopen(outfilename, "wb");
    if (!infile || !outfile) {
        return -1;
    }
    while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, num_read, outfile);
        memset(buffer, 0, 128);
    }
    gzclose(infile);
    fclose(outfile);
    return 0;
}

unsigned long TestZlib::file_size( char *filename) {
    FILE *pFile = fopen(filename, "rb");
    fseek(pFile, 0, SEEK_END); //偏移到文件尾
    unsigned long size = ftell(pFile); //读取文件指针到开头的字节数
    fclose(pFile);
    return size;

}

int TestZlib::compress_one_file(char *infilename, char *outfilename) {
    int           num_read      = 0; //保存读取到的字节数
    char          inbuffer[128] = {0}; //保存读取的文件数据
    unsigned long total_read    = 0; //保存源文件大小
    FILE          *infile       = fopen(infilename, "rb");
    gzFile        outfile       = gzopen(outfilename, "wb");
    if (!infile || !outfile) {
        return -1;
    }
    while ((num_read = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0) //读文件数据
    {
        total_read += num_read;
        gzwrite(outfile, inbuffer, num_read); //写到压缩文件中
        memset(inbuffer, 0, 128); //清空内存
    }
    fclose(infile); //关闭文件
    gzclose(outfile); //关闭压缩文件
    printf("原文件大小: %ld bytes\n 压缩后的文件大小: %ld bytes\n 压缩率 %4.2f%%\n", total_read, file_size(outfilename),
           (1.0 -
            file_size(outfilename) * 1.0 / total_read) * 100.0);

}
