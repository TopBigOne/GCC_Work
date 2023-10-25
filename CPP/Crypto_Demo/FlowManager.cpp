//
// Created by DEV on 10/25/23.
//

#include "FlowManager.h"
#include "TestZlib.h"

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

}

void FlowManager::CompressDex() {
    char *p1 = const_cast<char * >(sinohydrosaurus_Dex_CompressPath.c_str());
    char *p2 = const_cast<char *>(sinohydrosaurus_Dex_CompressPath.c_str());
    TestZlib testZlib;
    testZlib.compress_one_file(p1, p2);

}


void FlowManager::DecompressDex() {
    char *p1 = const_cast<char * >(sinohydrosaurusDexPath.c_str());
    char *p2 = const_cast<char *>(sinohydrosaurus_Dex_DecompressPath.c_str());
    TestZlib testZlib;
    testZlib.decompress_one_file(p1, p2);

}


void FlowManager::handleEncryptZip() {

}

void FlowManager::moveEncryptFileToAssert() {

}

void FlowManager::handleDecipher() {

}
