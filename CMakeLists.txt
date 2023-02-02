cmake_minimum_required(VERSION 3.12)
set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")
project(SelfLauncher)

include(VitaDevelopmentSuite)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(libScePafPreload)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    message("Debug build!")
    set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
    add_compile_options(
        -D_SCE_TARGET_OS_PSP2=1 -Xdiag=0 -Xquit=2 -Od -g -noex -DSCE_PAF_TOOL_PRX=1 -D_DEBUG=1
    )
else()
    add_compile_options(
        -D_SCE_TARGET_OS_PSP2=1 -Xdiag=0 -Xquit=2 -O3 -noex
    )
endif()

add_link_options(
    --no-standard-libraries  -Map=${CMAKE_BINARY_DIR}/mapfile -sn-full-map
)

# Generate the headers for the PAF plugin
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/sl_plugin.h
    COMMAND py ${CMAKE_SOURCE_DIR}/genhashheaders.py ${CMAKE_SOURCE_DIR}/resource/sl_plugin.xml ${CMAKE_SOURCE_DIR}/include/sl_plugin.h
)

# Generate the headers for the plugin locale
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/sl_locale.h
    COMMAND py ${CMAKE_SOURCE_DIR}/genhashheaders.py ${CMAKE_SOURCE_DIR}/resource/locale/en.xml ${CMAKE_SOURCE_DIR}/include/sl_locale.h
)

# Generate the headers for the plugin settings
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/sl_settings.h
    COMMAND py ${CMAKE_SOURCE_DIR}/genhashheaders.py ${CMAKE_SOURCE_DIR}/resource/file/sl_settings.xml ${CMAKE_SOURCE_DIR}/include/sl_settings.h
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/utils.cpp
    src/settings.cpp
    src/pages/page.cpp
    src/pages/file_browser.cpp

    include/common.h
    include/print.h
    include/utils.h
    include/pages/page.h
    include/pages/file_browser.h
    include/sl_plugin.h
    include/sl_locale.h
    include/sl_settings.h
)

target_link_directories(${PROJECT_NAME} PRIVATE
  ${VDSUITE_LIBRARY_DIRECTORIES}
  stubs
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
  $ENV{SCE_PSP2_SDK_DIR}/target/include
  $ENV{SCE_PSP2_SDK_DIR}/target/include_common
)

target_link_libraries(${PROJECT_NAME}
    SceLibKernel_stub
    SceThreadMgr_stub
    SceVshBridge_stub
    ScePafStdc_stub
    ScePafThread_stub
    ScePafTopLevel_stub
    ScePafWidget_stub
    ScePafCommon_stub
    ScePafGraphics_stub
    ScePafResource_stub
    ScePafMisc_stub
    LoaderCompanionKernel_stub_weak
    SLKernel_stub_weak
    SceAppUtil_stub
    SceAppSettings_stub
    taihenUnsafe_stub
    SceSysmodule_stub
)

# Create our list for all locale .xml files
file(GLOB localeXml "${CMAKE_SOURCE_DIR}/resource/locale/*.xml")
# Create our list for generated rcs files
set(rcsFiles)
# Loop through the xml files and generate an rcs file and add to the deps list
foreach(_xml ${localeXml})
    get_filename_component(localeName ${_xml} NAME_WE)
    set(rcsOutPath ${CMAKE_SOURCE_DIR}/resource/locale/${localeName}.rcs)
    message("Locale added - ${localeName}.rcs")
    add_custom_command(
        OUTPUT ${rcsOutPath}
        COMMAND python $ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cxml/appinfo/rcs_compiler.py -o ${rcsOutPath} ${_xml} 
    )
    list(APPEND rcsFiles ${rcsOutPath})
endforeach()
# Generate RCO file
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/sl_plugin.rco
    DEPENDS ${CMAKE_SOURCE_DIR}/resource/sl_plugin.xml ${rcsFiles}
    COMMAND python $ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cxml/appinfo/appinfo_compiler.py -o ${CMAKE_BINARY_DIR}/sl_plugin.rco ${CMAKE_SOURCE_DIR}/resource/sl_plugin.xml
)
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    # Generate the RCD file
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/sl_plugin.rcd
        DEPENDS ${CMAKE_SOURCE_DIR}/resource/sl_plugin.xml 
        COMMAND $ENV{SCE_PSP2_SDK_DIR}/host_tools/build/bin/psp2rcd.exe -i ${CMAKE_SOURCE_DIR}/resource/sl_plugin.xml -o ${CMAKE_BINARY_DIR}/sl_plugin.rcd
    )
endif()

VDSuiteCreateSfo(${PROJECT_NAME}.sfo param.sfx)
VDSuiteSignElf(${PROJECT_NAME}.self $<TARGET_FILE:${PROJECT_NAME}> BOOT_PARAM bootparam.yml)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf
        DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.self
        COMMAND "$ENV{SCE_ROOT_DIR}/PSP2/Tools/Publishing Tools/bin/ext/unfself.exe" ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.self ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf
    )
    VDSuitePackage(${PROJECT_NAME}.vpk
    FILES
        ${PROJECT_NAME}.self eboot.bin
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sfo sce_sys/param.sfo
        ${CMAKE_BINARY_DIR}/sl_plugin.rco resource/sl_plugin.rco
        ${CMAKE_BINARY_DIR}/sl_plugin.rcd resource/sl_plugin.rcd
        ${CMAKE_BINARY_DIR}/libScePafPreload/libScePafPreload.suprx module/libScePafPreload.suprx
        ${CMAKE_SOURCE_DIR}/SLKernel/build/SLKernel.skprx module/SLKernel.skprx  
    )  
else()
    VDSuitePackage(${PROJECT_NAME}.vpk
    FILES
        ${PROJECT_NAME}.self eboot.bin
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sfo sce_sys/param.sfo
        ${CMAKE_BINARY_DIR}/sl_plugin.rco resource/sl_plugin.rco
        ${CMAKE_BINARY_DIR}/libScePafPreload/libScePafPreload.suprx module/libScePafPreload.suprx
        ${CMAKE_SOURCE_DIR}/SLKernel/build/SLKernel.skprx module/SLKernel.skprx
    )
endif()