//
// Created by DEV on 10/25/23.
//

#include "FlowManager.h"
#include "TestZlib.h"
#include "TestEvp.h"

FlowManager::FlowManager() {
    ori_zip_file_path.append(ROOT_DIR);
    ori_zip_file_path.append(WORK_DIR);
    ori_zip_file_path.append(ori_zip_file_name);


    sinohydrosaurus_Dex_CompressPath.append(ROOT_DIR);
    sinohydrosaurus_Dex_CompressPath.append(WORK_DIR);
    sinohydrosaurus_Dex_CompressPath.append(SINOHYDROSAURUS_COMPRESS_NAME);


    sinohydrosaurus_Dex_DecompressPath.append(ROOT_DIR);
    sinohydrosaurus_Dex_DecompressPath.append(WORK_DIR);
    sinohydrosaurus_Dex_DecompressPath.append(SINOHYDROSAURUS_DECOMPRESS_NAME);

    // 加密路径
    encrypt_file_path.append(ROOT_DIR);
    encrypt_file_path.append(WORK_DIR);
    encrypt_file_path.append(encrypt_file_name);


    // 解密路径
    decipher_file_path.append(ROOT_DIR);
    decipher_file_path.append(WORK_DIR);
    decipher_file_path.append(SINOHYDROSAURUS_DECIPHER_COMPRESS_NAME);

    cout << "ori_zip_file_path             : " << ori_zip_file_path << endl;
    cout << "sinohydrosaurus_Dex_DecompressPath : " << sinohydrosaurus_Dex_DecompressPath << endl;
    cout << "encrypt_file_path    : " << encrypt_file_path << endl;
    cout << "decipher_file_path   : " << decipher_file_path << endl;


}

/**
 * 压缩
 */
void FlowManager::CompressDex() {
    puts("step 1 ： 压缩");
    char *p1 = const_cast<char * >(ori_zip_file_path.c_str());
    char *p2 = const_cast<char *>( sinohydrosaurus_Dex_CompressPath.c_str());

    printf("    sinohydrosaurus_Dex_Path          : %s\n", p1);
    printf("    sinohydrosaurus_Dex_Compress_Path : %s\n", p2);
    TestZlib testZlib;
    testZlib.compress_one_file(p1, p2);

}


/**
 * 加密
 */
void FlowManager::handleEncryptZip() {
    puts("step 2 ： 加密");
    char *p1 = const_cast<char * >(ori_zip_file_path.c_str());
    char *p2 = const_cast<char *>(encrypt_file_path.c_str());

    TestEvp testEvp;
    testEvp.encryptFile(key, iv, p1, p2);

}

/**
 * 解密
 */
void FlowManager::handleDecipher() {
    puts("step 4 :解密");
    char *p1 = const_cast<char * >(encrypt_file_path.c_str());

    char    *p2 = const_cast<char *>(decipher_file_path.c_str());
    TestEvp testEvp;
    testEvp.decryptFile(key, iv, p1, p2);


}


/**
 * 解压缩
 */
void FlowManager::DecompressDex() {
    puts("step 5 : 解压缩");
    char *p1 = const_cast<char * >(decipher_file_path.c_str());
    char *p2 = const_cast<char *>(sinohydrosaurus_Dex_DecompressPath.c_str());
    printf("    ori_zip_file_path             : %s\n", p1);
    printf("    sinohydrosaurus_Dex_DecompressPath : %s\n", p2);
    TestZlib testZlib;
    testZlib.decompress_one_file(p1, p2);
}

/**
 * 移动到资源目录
 */
void FlowManager::moveEncryptFileToAssert() {
    puts("step  3: 移动到资源目录");

    string asset_dex_Encrypt_Path;


    asset_dex_Encrypt_Path.append(ROOT_DIR);
    // test env
//    asset_dex_Encrypt_Path.append("app/src/main/assets/");
// release end
    asset_dex_Encrypt_Path.append("app/assets/");
    asset_dex_Encrypt_Path.append("2_LRZUMyOA_yq");

// 2_sinohydrosaurus_encrypt
    std::ifstream sourceFile(encrypt_file_path, std::ios::binary);
    if (!sourceFile) {
        std::cerr << "  moveEncryptFileToAssert() Failed to open source file: " << encrypt_file_path
                  << std::endl;

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

