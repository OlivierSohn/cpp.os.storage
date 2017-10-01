//
//  genRandBytes.cpp
//  os.storage
//
//  Created by Olivier on 01/10/2017.
//  Copyright © 2017 Olivier. All rights reserved.
//

#include "../include/public.h"

int main(int argc, const char * argv[]) {
    using namespace imajuscule;
    constexpr auto countBytes = 2224 * 1668; // the number of pixels of 10.5 inch ipad pro
    
    auto writer = std::make_unique<bsonparser::BSonWriter>(DirectoryPath{"./"}, FileName{"rndBytes.bson"});
    writer->Initialize();
    writer->writeBinary(countBytes, []() -> uint8_t {
        return std::uniform_int_distribution<>{0,255}(mersenne<SEEDED::No>());
    });
    
    return 0;
}
