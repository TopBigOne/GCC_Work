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


    const char *ROOT_DIR             = "/Users/dev/Documents/Andorid_Work/Work_1/TestAssetManager/";
    const char *WORK_DIR             = "lib_asset/work_dir/";
    // dex 文件名称
    const char *SINOHYDROSAURUS_NAME = "sinohydrosaurus.dex";
    string     sinohydrosaurusDexPath;

    const char *SINOHYDROSAURUS_COMPRESS_NAME   = "result/sinohydrosaurus_compress";
    string     sinohydrosaurus_Dex_CompressPath;

    const char *SINOHYDROSAURUS_DECOMPRESS_NAME = "result/sinohydrosaurus_decompress.dex";
    string     sinohydrosaurus_Dex_DecompressPath;

};


#endif //CRYPTO_DEMO_FLOWMANAGER_H
