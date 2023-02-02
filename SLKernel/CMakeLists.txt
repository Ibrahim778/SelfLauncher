cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(SLKernel)
include("${VITASDK}/share/vita.cmake" REQUIRED)
include_directories(
    $ENV{VITASDK}/arm-vita-eabi/include/   
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(${PROJECT_NAME}
  src/main.c
)
set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS
  -nostdlib
)
target_link_libraries(${PROJECT_NAME}
  SceModulemgrForDriver_stub
  SceSysclibForDriver_stub
  SceAppMgrForDriver_stub
  SceThreadmgrForDriver_stub
  SceModulemgrForKernel_stub
  SceDebugForDriver_stub
  taihenModuleUtils_stub
  SceSysmemForDriver_stub 
)
vita_create_self(${PROJECT_NAME}.skprx ${PROJECT_NAME} CONFIG ${PROJECT_NAME}.yml UNSAFE)
vita_create_stubs(stubs ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.yml KERNEL)

install(DIRECTORY ${CMAKE_BINARY_DIR}/stubs/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.a"
)