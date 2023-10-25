//
// Created by DEV on 10/25/23.
//

#ifndef CRYPTO_DEMO_FLOWMANAGER_H
#define CRYPTO_DEMO_FLOWMANAGER_H

#include <string>

using namespace std;

/**
 * 流程管理
 */
class FlowManager {

public:

    FlowManager();

    /**
     * step 1  : compress
     */
    void CompressDex();

    /**
     *
     */
    void DecompressDex();

    /**
     * 处理加密
     */
    void handleEncryptZip();


    void moveEncryptFileToAssert();

    /**
     * 处理解密
     */
    void handleDecipher();


public:

    unsigned char key[16] = {0xB6, 0x2C, 0x0D, 0x8D,
                             0xE8, 0x22, 0x81, 0x4C,
                             0x2D, 0x13, 0xA9, 0xE0,
                             0x92, 0xB0, 0xCF, 0x0E};

    unsigned char iv[16] = {0xBD, 0x55, 0xC7, 0xC8,
                            0x40, 0xFE, 0xD6, 0xD1,
                            0x01, 0x4D, 0x18, 0xF3,
                            0xD3, 0x32, 0x0C, 0xFA,};


    const char *ROOT_DIR             = "/Users/dev/Documents/Andorid_Work/Work_1/TestAssetManager/";
    const char *WORK_DIR             = "lib_asset/work_dir/";
    // dex 文件名称
    const char *SINOHYDROSAURUS_NAME = "sinohydrosaurus.dex";
    string     sinohydrosaurusDexPath;

    // 压缩包路径
    const char *SINOHYDROSAURUS_COMPRESS_NAME = "result/sinohydrosaurus_compress";
    string sinohydrosaurus_Dex_CompressPath;

    // 解压缩包路径
    const char *SINOHYDROSAURUS_DECOMPRESS_NAME = "result/sinohydrosaurus_decompress.dex";
    string     sinohydrosaurus_Dex_DecompressPath;

    // 加密压缩包路径
    const char *SINOHYDROSAURUS_ENCRYPT_NAME = "result/sinohydrosaurus_encrypt";
    string     sinohydrosaurus_Dex_EncryptPath;


    // 解密压缩包路径
    const char *SINOHYDROSAURUS_DECIPHER_COMPRESS_NAME = "result/sinohydrosaurus_Decipher_compress";
    string sinohydrosaurus_Dex_DecipherPath;


};


#endif //CRYPTO_DEMO_FLOWMANAGER_H
