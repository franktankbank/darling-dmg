cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0167 NEW)
project(darling-dmg CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WITH_STATIC "Build libdmg as a static library" OFF)
option(WITH_STRIP "Strip executables on install" ON)
option(WITH_LTO "Build project with lto" ON)
option(WITH_TESTS "Enable tests" OFF)
option(BUILD_DARLING_DMG "Build darling-dmg executable" ON)
option(BUILD_DARLING_DMG_EXTRACT "Build darling-dmg-extract executable" ON)
option(INSTALL_C_HEADERS "Install c wrapper headers" OFF)

if (WITH_STRIP)
	set(CMAKE_INSTALL_DO_STRIP ON)
endif (WITH_STRIP)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported)
if (supported)
	if (WITH_LTO)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif (WITH_LTO)
endif (supported)

set(
	CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/"
)

set(
	SOURCES
	"adc.cpp"
	"AppleDisk.cpp"
	"CacheZone.cpp"
	"CachedReader.cpp"
	"DMGDecompressor.cpp"
	"DMGDisk.cpp"
	"DMGPartition.cpp"
	"FileReader.cpp"
	"GPTDisk.cpp"
	"HFSAttributeBTree.cpp"
	"HFSBTree.cpp"
	"HFSCatalogBTree.cpp"
	"HFSExtentsOverflowBTree.cpp"
	"HFSFork.cpp"
	"HFSHighLevelVolume.cpp"
	"HFSVolume.cpp"
	"HFSZlibReader.cpp"
	"MacBinary.cpp"
	"MemoryReader.cpp"
	"Reader.cpp"
	"ResourceFork.cpp"
	"unichar.cpp"
	"SubReader.cpp"
	"DMGExtractor.cpp"
	"DMGExtractorWrapper.cpp"
)

list(TRANSFORM SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# This part of CMakeLists.txt is for when building darling-dmg
# as a standalone FUSE module outside of Darling.
if (NOT DARLING)
	if (BUILD_DARLING_DMG)
		add_definitions(-D_FILE_OFFSET_BITS=64)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")
	endif (BUILD_DARLING_DMG)

	include(FindLibXml2)

	find_package(LibXml2 REQUIRED)
	include_directories(${LIBXML2_INCLUDE_DIR})

	set(ICU_GENCNVAL_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_ICUINFO_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENBRK_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_ICU-CONFIG_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENRB_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENCCODE_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENCFU_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENCMN_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENDICT_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENNORM2_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_GENSPREP_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_ICUPKG_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_UCONV_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_MAKECONV_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_PKGDATA_EXECUTABLE "" CACHE INTERNAL FILEPATH "")
	set(ICU_DERB_EXECUTABLE "" CACHE INTERNAL FILEPATH "")

	find_package(ICU COMPONENTS uc data REQUIRED)

	find_package(ZLIB REQUIRED)

	find_package(BZip2 REQUIRED)

	if (WITH_TESTS)
		enable_testing()
		find_package(Boost COMPONENTS unit_test_framework REQUIRED)

		add_definitions(-DBOOST_TEST_DYN_LINK ) # -DDEBUG

		set(CacheTest_SRC
			test/CacheTest.cpp
			src/CacheZone.cpp
			src/CachedReader.cpp
			src/Reader.cpp
			src/MemoryReader.cpp
		)

		add_executable(CacheTest ${CacheTest_SRC})
		target_link_libraries(CacheTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
		add_test(NAME CacheTest COMMAND CacheTest)
	endif (WITH_TESTS)

	if (WITH_STATIC)
		add_library(dmg STATIC)
	else (WITH_STATIC)
		add_library(dmg SHARED)
	endif (WITH_STATIC)

	target_sources(dmg PRIVATE ${SOURCES})
	target_link_libraries(dmg ICU::uc ICU::data ZLIB::ZLIB BZip2::BZip2 ${LIBXML2_LIBRARY})
	install(TARGETS dmg DESTINATION lib)

	if (BUILD_DARLING_DMG)
		add_executable(darling-dmg
			src/main-fuse.cpp
		)

		target_link_libraries(darling-dmg -lfuse3 dmg)
		install(TARGETS darling-dmg RUNTIME DESTINATION bin)
	endif (BUILD_DARLING_DMG)

	set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	if (BUILD_DARLING_DMG_EXTRACT)
		add_executable(darling-dmg-extract src/main-extract.cpp)
		target_compile_features(darling-dmg-extract PRIVATE cxx_std_17)
		target_link_libraries(darling-dmg-extract dmg)
		install(TARGETS darling-dmg-extract RUNTIME DESTINATION bin)
	endif (BUILD_DARLING_DMG_EXTRACT)

	if (INSTALL_C_HEADERS)
		target_sources(dmg PUBLIC
			FILE_SET public_headers
			TYPE HEADERS
			BASE_DIRS src
			FILES src/dmg_extractor_wrapper.h
		)

		install(TARGETS dmg FILE_SET public_headers)
	endif (INSTALL_C_HEADERS)

# This is used in Darling build.
else (NOT DARLING)
	include(darling_exe)
	include(FindPkgConfig)

	add_definitions(-D_FILE_OFFSET_BITS=64 -DCOMPILE_WITH_LZFSE=1)

	option(DARLING_DMG_ENABLE_FUSE "Build guest hdiutil with FUSE mounting support" OFF)
	if (DARLING_DMG_ENABLE_FUSE)
		include(wrap_elf)
		pkg_check_modules(FUSE REQUIRED fuse)
		include_directories(${FUSE_INCLUDE_DIRS})
		wrap_elf(fuse libfuse.so)
	endif()

	decltype(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc")

	wrap_elf(fuse libfuse.so)

	add_darling_executable(hdiutil
		${SOURCES}
		src/main-hdiutil.cpp
	)

	if (DARLING_DMG_ENABLE_FUSE)
		target_link_libraries(hdiutil fuse)
	else()
		target_compile_definitions(hdiutil PRIVATE DARLING_DMG_NO_FUSE)
	endif()
	target_link_libraries(hdiutil icucore z bz2 crypto44 xml2 iconv lzfse)
	install(TARGETS hdiutil DESTINATION libexec/darling/usr/bin)

endif (NOT DARLING)
