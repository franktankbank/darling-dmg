#ifndef DMG_EXTRACTOR_WRAPPER_H
#define DMG_EXTRACTOR_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#define DMG_EXTRACTOR_WRAPPER_SUCCESS           0
#define DMG_EXTRACTOR_WRAPPER_RUMTIME_ERROR    -1

int extract_dmg(const char* imagePath, const char* destination);

#ifdef __cplusplus
}
#endif

#endif /* DMG_EXTRACTOR_WRAPPER_H */
