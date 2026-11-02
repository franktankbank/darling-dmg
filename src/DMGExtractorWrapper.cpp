#include "DMGExtractor.h"
#include "dmg_extractor_wrapper.h"
#include <iostream>

extern "C" {
    #define SUCCESS 0
    #define RUNTIME_ERROR -1

    int extract_dmg(const char* imagePath, const char* destination) {
        try {
            extractDMG(imagePath, destination);
        } catch (const std::exception& error) {
            std::cerr << "Extraction failed: " << error.what() << '\n';
            return RUNTIME_ERROR;
        }

        return SUCCESS;
    }
}
