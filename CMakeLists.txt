
cmake_minimum_required(VERSION 3.10)
project(UQAC_OPTI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  OpenXLSX
  GIT_REPOSITORY https://github.com/troldal/OpenXLSX.git
  GIT_TAG      master #ou autre tag
)
set(OPENXLSX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(OPENXLSX_BUILD_TESTS OFF CACHE INTERNAL "")
set(OPENXLSX_BUILD_DOCS OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(OpenXLSX)

add_executable(test_opti PROJET1/test_opti.cpp)

target_link_libraries(test_opti PRIVATE OpenXLSX::OpenXLSX)
target_compile_features(test_opti PRIVATE cxx_std_17)

# Commande pour copier le dossier data vers le dossier de build
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/PROJET1/data" 
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")