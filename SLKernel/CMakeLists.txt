cmake_minimum_required(VERSION 3.12)
set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")
project(SLKernel)
include(VitaDevelopmentSuite)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
enable_language(EMD)

add_compile_options(
    -D_SCE_TARGET_OS_PSP2=1 -Xdiag=0 -Xquit=2
)

add_link_options(
  --prx-no-runtime-support
)

add_library(${PROJECT_NAME} MODULE
    src/main.c
    ${PROJECT_NAME}.emd
)

target_link_libraries(${PROJECT_NAME}
    SceThreadMgrForDriver_stub
    SceDebugForDriver_stub
    SceProcessmgrForKernel_stub
    SceSysclibForDriver_stub
    SceSysmemForDriver_stub
    taihenForKernel_stub
    SceAppMgrForDriver_stub
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${VDSUITE_KERNEL_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
  $ENV{SCE_PSP2_SDK_DIR}/target/include
  $ENV{SCE_PSP2_SDK_DIR}/target/include_common
  include
)

target_link_directories(${PROJECT_NAME} PRIVATE
  ${VDSUITE_LIBRARY_DIRECTORIES}
)


VDSuiteSignElf(${PROJECT_NAME}.skprx ${PROJECT_NAME} KERNEL)

VDSuiteCreateStubs(stubs ${PROJECT_NAME}.emd
  LIBRARIES
    SLKernel_stub
    SLKernel_stub_weak
)