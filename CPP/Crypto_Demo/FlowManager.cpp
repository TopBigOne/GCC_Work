//
// Created by DEV on 10/25/23.
//

#include "FlowManager.h"
#include "TestZlib.h"
#include "TestEvp.h"

FlowManager::FlowManager() {
    sinohydrosaurusDexPath.append(ROOT_DIR);
    sinohydrosaurusDexPath.append(WORK_DIR);
    sinohydrosaurusDexPath.append(SINOHYDROSAURUS_NAME);


    sinohydrosaurus_Dex_CompressPath.append(ROOT_DIR);
    sinohydrosaurus_Dex_CompressPath.append(WORK_DIR);
    sinohydrosaurus_Dex_CompressPath.append(SINOHYDROSAURUS_COMPRESS_NAME);


    sinohydrosaurus_Dex_DecompressPath.append(ROOT_DIR);
    sinohydrosaurus_Dex_DecompressPath.append(WORK_DIR);
    sinohydrosaurus_Dex_DecompressPath.append(SINOHYDROSAURUS_DECOMPRESS_NAME);

    // 加密路径
    sinohydrosaurus_Dex_EncryptPath.append(ROOT_DIR);
    sinohydrosaurus_Dex_EncryptPath.append(WORK_DIR);
    sinohydrosaurus_Dex_EncryptPath.append(SINOHYDROSAURUS_ENCRYPT_NAME);


    // 解密路径
    sinohydrosaurus_Dex_DecipherPath.append(ROOT_DIR);
    sinohydrosaurus_Dex_DecipherPath.append(WORK_DIR);
    sinohydrosaurus_Dex_DecipherPath.append(SINOHYDROSAURUS_DECIPHER_COMPRESS_NAME);


}

void FlowManager::CompressDex() {
    puts("step 1");
    char     *p1 = const_cast<char * >(sinohydrosaurus_Dex_CompressPath.c_str());
    char     *p2 = const_cast<char *>(sinohydrosaurus_Dex_CompressPath.c_str());
    TestZlib testZlib;
    testZlib.compress_one_file(p1, p2);

}


/**
 * 加密
 */
void FlowManager::handleEncryptZip() {
    puts("step 2");
    char    *p1 = const_cast<char * >(sinohydrosaurus_Dex_CompressPath.c_str());
    char    *p2 = const_cast<char *>(sinohydrosaurus_Dex_EncryptPath.c_str());
    TestEvp testEvp;
    testEvp.encryptFile(key, iv, p1, p2);

}

/**
 * 解密
 */
void FlowManager::handleDecipher() {
    puts("step 4 ");
    char    *p1 = const_cast<char * >(sinohydrosaurus_Dex_EncryptPath.c_str());
    char    *p2 = const_cast<char *>(sinohydrosaurus_Dex_DecipherPath.c_str());
    TestEvp testEvp;
    testEvp.decryptFile(key, iv, p1, p2);


}


/**
 * 解压缩
 */
void FlowManager::DecompressDex() {
    puts("step 5");
    char     *p1 = const_cast<char * >(sinohydrosaurusDexPath.c_str());
    char     *p2 = const_cast<char *>(sinohydrosaurus_Dex_DecompressPath.c_str());
    TestZlib testZlib;
    testZlib.decompress_one_file(p1, p2);
}

void FlowManager::moveEncryptFileToAssert() {
    puts("step  3: moveEncryptFileToAssert");

    string asset_dex_Encrypt_Path;
    asset_dex_Encrypt_Path.append(
            "/Users/dev/Documents/Andorid_Work/Work_1/TestAssetManager/app/src/main/assets/sinohydrosaurus_encrypt");


    std::ifstream sourceFile(sinohydrosaurus_Dex_EncryptPath, std::ios::binary);
    if (!sourceFile) {
        std::cerr << "Failed to open source file: " << sinohydrosaurus_Dex_EncryptPath << std::endl;

    }

    std::ofstream destinationFile(asset_dex_Encrypt_Path, std::ios::binary);
    if (!destinationFile) {
        std::cerr << "Failed to open destination file: " << asset_dex_Encrypt_Path << std::endl;
        sourceFile.close();

    }

    destinationFile << sourceFile.rdbuf();

    sourceFile.close();
    destinationFile.close();


}

