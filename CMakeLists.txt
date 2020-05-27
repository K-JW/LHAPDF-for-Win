cmake_minimum_required(VERSION 3.0)
project(LHAPDF CXX)

# Fail if cmake is called in the source directory
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "You don't want to configure in the source directory!")
endif()

set(CMAKE_CXX_STANDARD 11)
add_compile_options("-O3")
add_compile_options("-fPIC")

include_directories("${CMAKE_SOURCE_DIR}/LHAPDF/include")
include_directories("${CMAKE_SOURCE_DIR}/LHAPDF/src/yamlcpp")

# Add yamlcpp library
set(YAML_DIR ${CMAKE_SOURCE_DIR}/LHAPDF/src/yamlcpp)
set(YAML_SRC ${YAML_DIR}/aliasmanager.cpp ${YAML_DIR}/binary.cpp 
    ${YAML_DIR}/conversion.cpp ${YAML_DIR}/directives.cpp ${YAML_DIR}/emitfromevents.cpp 
    ${YAML_DIR}/emitter.cpp ${YAML_DIR}/emitterstate.cpp ${YAML_DIR}/emitterutils.cpp 
    ${YAML_DIR}/exp.cpp ${YAML_DIR}/iterator.cpp ${YAML_DIR}/node.cpp ${YAML_DIR}/nodebuilder.cpp 
    ${YAML_DIR}/nodeownership.cpp ${YAML_DIR}/null.cpp ${YAML_DIR}/ostream.cpp ${YAML_DIR}/parser.cpp 
    ${YAML_DIR}/regex.cpp ${YAML_DIR}/scanner.cpp ${YAML_DIR}/scanscalar.cpp ${YAML_DIR}/scantag.cpp 
    ${YAML_DIR}/scantoken.cpp ${YAML_DIR}/simplekey.cpp ${YAML_DIR}/singledocparser.cpp 
    ${YAML_DIR}/stream.cpp ${YAML_DIR}/tag.cpp
    )
add_library(yamlcpp STATIC ${YAML_SRC})

if (MSVC)
    include_directories(${CMAKE_SOURCE_DIR}/include)
    # MSVC doesn't define M_PI
    # https://blog.csdn.net/liu_feng_zi_/article/details/84816763?
    add_definitions(-D_USE_MATH_DEFINES)
    # Ref. https://stackoverflow.com/questions/42070450/
    set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
endif()

set(LHAPDF_DIR ${CMAKE_SOURCE_DIR}/LHAPDF/src)
set(LHAPDF_SRC ${LHAPDF_DIR}/AlphaS_Analytic.cc ${LHAPDF_DIR}/AlphaS_Ipol.cc 
    ${LHAPDF_DIR}/AlphaS_ODE.cc ${LHAPDF_DIR}/AlphaS.cc ${LHAPDF_DIR}/BicubicInterpolator.cc 
    ${LHAPDF_DIR}/BilinearInterpolator.cc ${LHAPDF_DIR}/Config.cc 
    ${LHAPDF_DIR}/ContinuationExtrapolator.cc ${LHAPDF_DIR}/ErrExtrapolator.cc 
    ${LHAPDF_DIR}/Factories.cc ${LHAPDF_DIR}/FileIO.cc ${LHAPDF_DIR}/GridPDF.cc 
    ${LHAPDF_DIR}/Info.cc ${LHAPDF_DIR}/Interpolator.cc ${LHAPDF_DIR}/LHAGlue.cc 
    ${LHAPDF_DIR}/LogBicubicInterpolator.cc ${LHAPDF_DIR}/LogBilinearInterpolator.cc 
    ${LHAPDF_DIR}/NearestPointExtrapolator.cc 
    ${LHAPDF_DIR}/PDF.cc ${LHAPDF_DIR}/PDFIndex.cc ${LHAPDF_DIR}/PDFSet.cc 
    ${LHAPDF_DIR}/PDFInfo.cc ${LHAPDF_DIR}/Utils.cc)

add_library(LHAPDF-static STATIC ${LHAPDF_SRC} ${CMAKE_SOURCE_DIR}/patch/Paths.cc)
target_compile_definitions(LHAPDF-static PRIVATE YAMLCPP_API=3 YAML_NAMESPACE=LHAPDF_YAML 
    LHAPDF_DATA_PREFIX="./")

add_library(LHAPDF SHARED ${LHAPDF_SRC} ${CMAKE_SOURCE_DIR}/patch/Paths.cc)
target_compile_definitions(LHAPDF PRIVATE YAMLCPP_API=3 YAML_NAMESPACE=LHAPDF_YAML 
    LHAPDF_DATA_PREFIX="./")
target_link_libraries(LHAPDF yamlcpp)

#install
if (MSVC)
    install (FILES ${CMAKE_BINARY_DIR}/Release/LHAPDF-static.lib 
        ${CMAKE_BINARY_DIR}/Release/LHAPDF.dll 
        ${CMAKE_BINARY_DIR}/Release/yamlcpp.lib 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
else()
    install (FILES ${CMAKE_BINARY_DIR}/libLHAPDF-static.a 
        ${CMAKE_BINARY_DIR}/libLHAPDF.dll 
        ${CMAKE_BINARY_DIR}/libLHAPDF.dll.a 
        ${CMAKE_BINARY_DIR}/libyamlcpp.a 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif()
install (DIRECTORY ${CMAKE_SOURCE_DIR}/LHAPDF/include/LHAPDF
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include 
    FILES_MATCHING PATTERN "*.h")
install (FILES ${CMAKE_SOURCE_DIR}/LHAPDF/lhapdf.conf 
    ${CMAKE_SOURCE_DIR}/LHAPDF/pdfsets.index 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/LHAPDF)
install (FILES ${CMAKE_SOURCE_DIR}/LHAPDF/bin/lhapdf 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Set Env
execute_process(
    COMMAND powershell "[environment]::SetEnvironmentVariable(\"LHAPDF_DATA_PATH\", \"${CMAKE_INSTALL_PREFIX}/share\",\"User\")" 
    OUTPUT_QUIET)