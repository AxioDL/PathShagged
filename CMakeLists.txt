cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15...3.20)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
      "Build type options: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif ()

# obtain revision info from git
find_package(Git)
if (GIT_FOUND)
  # make sure version information gets re-run when the current Git HEAD changes
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --git-path HEAD
          OUTPUT_VARIABLE metaforce_git_head_filename
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${metaforce_git_head_filename}")

  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --symbolic-full-name HEAD
          OUTPUT_VARIABLE metaforce_git_head_symbolic
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          COMMAND ${GIT_EXECUTABLE} rev-parse --git-path ${metaforce_git_head_symbolic}
          OUTPUT_VARIABLE metaforce_git_head_symbolic_filename
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${metaforce_git_head_symbolic_filename}")

  # defines METAFORCE_WC_REVISION
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
          OUTPUT_VARIABLE METAFORCE_WC_REVISION
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  # defines METAFORCE_WC_DESCRIBE
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} describe --tag --long --dirty
          OUTPUT_VARIABLE METAFORCE_WC_DESCRIBE
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  # remove hash (and trailing "-0" if needed) from description
  string(REGEX REPLACE "(-0)?-[^-]+((-dirty)?)$" "\\2" METAFORCE_WC_DESCRIBE "${METAFORCE_WC_DESCRIBE}")

  # defines METAFORCE_WC_BRANCH
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
          OUTPUT_VARIABLE METAFORCE_WC_BRANCH
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  # defines METAFORCE_WC_DATE
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} log -1 --format=%ad
          OUTPUT_VARIABLE METAFORCE_WC_DATE
          OUTPUT_STRIP_TRAILING_WHITESPACE)
else ()
  message(STATUS "Unable to find git, commit information will not be available")
endif ()

if (METAFORCE_WC_DESCRIBE)
  string(REGEX REPLACE "v([0-9]+)\.([0-9]+)\.([0-9]+)\-([0-9]+).*" "\\1.\\2.\\3.\\4" METAFORCE_VERSION_STRING "${METAFORCE_WC_DESCRIBE}")
  string(REGEX REPLACE "v([0-9]+)\.([0-9]+)\.([0-9]+).*" "\\1.\\2.\\3" METAFORCE_VERSION_STRING "${METAFORCE_VERSION_STRING}")
else ()
  set(METAFORCE_WC_DESCRIBE "UNKNOWN-VERSION")
  set(METAFORCE_VERSION_STRING "0.0.0")
endif ()

string(TIMESTAMP CURRENT_YEAR "%Y")

# Add version information to CI environment variables
if(DEFINED ENV{GITHUB_ENV})
  file(APPEND "$ENV{GITHUB_ENV}" "METAFORCE_VERSION=${METAFORCE_WC_DESCRIBE}\n")
endif()
message(STATUS "Metaforce version set to ${METAFORCE_WC_DESCRIBE}")
project(metaforce LANGUAGES C CXX ASM VERSION ${METAFORCE_VERSION_STRING})

if (APPLE)
  set(PLATFORM_NAME macos)
elseif (WIN32)
  set(PLATFORM_NAME win32)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(PLATFORM_NAME linux)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)

if(APPLE AND NOT CMAKE_OSX_SYSROOT)
  # If the Xcode SDK is lagging behind system version, CMake needs this done first
  execute_process(COMMAND xcrun --sdk macosx --show-sdk-path
                  OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

set(METAFORCE_TARGET_GAME "PRIME1" CACHE STRING "The game Metaforce will replicate")
set_property(CACHE METAFORCE_TARGET_GAME PROPERTY STRINGS PRIME1 PRIME2)

option(METAFORCE_CROSSCOMPILING "Don't build tools; attempt package import" OFF)
if (METAFORCE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING On)
endif()

if(CMAKE_CROSSCOMPILING)
  set(HAVE_WORDS_BIGENDIAN_EXITCODE 0 CACHE INTEGER "Makes soxr happy" FORCE)
endif()

# MSVC has a "latest" flag, which always uses the newest standard
# when available. GCC and Clang posess no such flag, and must be
# manually enforced. CMake, curiously, also doesn't have a "latest"
# standard flag either.
if (NOT MSVC)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force shared libs off" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "Force static libs on" FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/extern/sanitizers-cmake/cmake")
find_package(Sanitizers)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL AMD64)
  set(METAFORCE_VECTOR_ISA "sse41" CACHE STRING "Vector ISA to build for (sse2, sse3, sse41, avx, avx2)")
endif ()

if(MSVC)
  if(${METAFORCE_VECTOR_ISA} STREQUAL "avx2")
    add_compile_options(/arch:AVX2)
    add_compile_definitions(__SSE4_1__=1)
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "avx")
    add_compile_options(/arch:AVX)
    add_compile_definitions(__SSE4_1__=1)
    message(STATUS "Building with AVX Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse41")
    add_compile_definitions(__SSE4_1__=1)
    # clang-cl 10 requires -msse4.1, may be fixed in newer versions?
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL Clang)
      add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-msse4.1>)
    endif()
    message(STATUS "Building with SSE4.1 Vector ISA")
  else()
    message(STATUS "Building with SSE2 Vector ISA")
  endif()

  if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set(VS_OPTIONS "/MP")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT metaforce)
  endif()

  # Shaddup MSVC
  add_compile_definitions(UNICODE=1 _UNICODE=1 __SSE__=1
      _CRT_SECURE_NO_WARNINGS=1 D_SCL_SECURE_NO_WARNINGS=1
      _SCL_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
      _ENABLE_EXTENDED_ALIGNED_STORAGE=1 NOMINMAX=1)
  add_compile_options(/IGNORE:4221
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4018>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4800>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4005>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4311>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4068>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4267>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4244>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4200>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4305>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4067>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4146>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4309>
          $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4805>
          ${VS_OPTIONS})

  string(REPLACE "/GR " "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REPLACE " /EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options(
    # Disable exceptions
    $<$<COMPILE_LANGUAGE:CXX>:/EHsc->

    # Disable RTTI
    $<$<COMPILE_LANGUAGE:CXX>:/GR->

    # Enforce various standards compliant behavior.
    $<$<COMPILE_LANGUAGE:CXX>:/permissive->

    # Enable standard volatile semantics.
    $<$<COMPILE_LANGUAGE:CXX>:/volatile:iso>

    # Reports the proper value for the __cplusplus preprocessor macro.
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>

    # Use latest C++ standard.
    $<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>
  )

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # Flags for MSVC (not clang-cl)
    add_compile_options(
      # Allow constexpr variables to have explicit external linkage.
      $<$<COMPILE_LANGUAGE:CXX>:/Zc:externConstexpr>

      # Assume that new throws exceptions, allowing better code generation.
      $<$<COMPILE_LANGUAGE:CXX>:/Zc:throwingNew>

      # Link-time Code Generation for Release builds
      $<$<CONFIG:Release>:/GL>
    )

    # Link-time Code Generation for Release builds
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /RELEASE /OPT:REF /OPT:ICF /INCREMENTAL:NO /DEBUGTYPE:cv,fixup")
  endif()

else()
  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
  if(${METAFORCE_VECTOR_ISA} STREQUAL "native")
    add_compile_options(-march=native)
    message(STATUS "Building with native ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "avx2")
    add_compile_options(-mavx2)
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "avx")
    add_compile_options(-mavx)
    message(STATUS "Building with AVX Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse41")
    add_compile_options(-msse4.1)
    message(STATUS "Building with SSE4.1 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse3")
    add_compile_options(-msse3)
    message(STATUS "Building with SSE3 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse2")
    add_compile_options(-msse2)
    message(STATUS "Building with SSE2 Vector ISA")
  else()
    message(STATUS "Building with x87 Vector ISA")
  endif()
  endif()

  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fno-plt HAS_NO_PLT)
  if (HAS_NO_PLT)
    add_compile_options(-fno-plt)
  endif()
  check_cxx_compiler_flag(-fno-asynchronous-unwind-tables HAS_NO_ASYNC_UNWIND_TABLES)
  if (HAS_NO_ASYNC_UNWIND_TABLES AND ${CMAKE_BUILD_TYPE} STREQUAL Release)
    # Binary size reduction
    add_compile_options(-fno-asynchronous-unwind-tables)
  endif()

  if(METAFORCE_MSAN)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++> -fsanitize=memory
                        -fsanitize-memory-track-origins -fsanitize-recover=all)
  endif()
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
                      $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
                      -Wall -Wno-multichar -Werror=implicit-fallthrough
                      -Wno-unused-variable -Wno-unused-result
                      -Wno-unused-function -Wno-sign-compare -Wno-unknown-pragmas -Werror)
  # doesn't work with generator expression in add_compile_options?
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_compile_options(-Wno-unknown-warning-option -Wno-unused-private-field)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wno-lto-type-mismatch -Wno-maybe-uninitialized)
  endif()

  if(APPLE)
    add_compile_options(-Wno-error=deprecated-declarations
                        $<$<CONFIG:Release>:-flto=thin>)
    if (METAFORCE_ASAN)
      add_compile_options(-fsanitize=address -fsanitize-address-use-after-scope)
      add_link_options(-fsanitize=address -fsanitize-address-use-after-scope)
    endif ()
  endif()

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
      # This is required to summarize std::string
      add_compile_options(-fno-limit-debug-info -fno-omit-frame-pointer)
    endif()
    option(USE_LD_LLD "Link with LLD" ON)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    option(USE_LD_GOLD "Link with GNU Gold" ON)
  endif()

  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED)
  if(LTO_SUPPORTED AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
    option(USE_LTO "Enable LTO" ON)
  else()
    option(USE_LTO "Enable LTO" OFF)
  endif()
  # FIXME GCC 11.1 -flto is completely broken
  if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 11.1.0)
    message(NOTICE "Working around GCC 11.1 bug; disabling LTO")
    set(USE_LTO OFF)
  endif()
else()
  option(USE_LD_LLD "Link with LLD" OFF)
  option(USE_LD_GOLD "Link with GNU Gold" OFF)
  option(USE_LTO "Enable LTO" OFF)
endif()
if(USE_LD_LLD)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=lld -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
  if("${LD_VERSION}" MATCHES "LLD")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -Wl,--build-id=uuid")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
    if(USE_LTO)
      add_compile_options(-flto=thin)
      add_link_options(-flto=thin)
      message(STATUS "LLD linker enabled with LTO.")
    else()
      message(STATUS "LLD linker enabled.")
    endif()
    set(USE_LD_GOLD OFF)
  else()
    message(WARNING "LLD linker isn't available, using the default system linker.")
    set(USE_LD_LLD OFF)
  endif()
endif()
if(USE_LD_GOLD)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
  if("${LD_VERSION}" MATCHES "GNU gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    if (USE_SPLIT_DWARF)
      add_compile_options(-gsplit-dwarf -Wl,--gdb-index)
      add_link_options(-gsplit-dwarf -Wl,--gdb-index)
      message(STATUS "GNU gold linker enabled with split DWARF.")
    elseif (USE_LTO)
      add_compile_options(-flto)
      add_link_options(-flto)
      message(STATUS "GNU gold linker enabled with LTO.")
    else()
      message(STATUS "GNU gold linker enabled.")
    endif()
    set(USE_LD_LLD OFF)
  else()
    message(WARNING "GNU gold linker isn't available, using the default system linker.")
    set(USE_LD_GOLD OFF)
  endif()
endif()

# Add discord-rpc here
if(NOT GEKKO AND NOT NX)
  set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/discord-rpc)
  if (NOT CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/fake-prefix)
  endif()
  add_subdirectory(extern/discord-rpc/src EXCLUDE_FROM_ALL)
  target_include_directories(discord-rpc PRIVATE extern/rapidjson/include PUBLIC extern/discord-rpc/include)
  if (APPLE)
    # remove their nasty hack
    get_target_property(DISCORD_LINK_LIBRARIES discord-rpc INTERFACE_LINK_LIBRARIES)
    list(REMOVE_ITEM DISCORD_LINK_LIBRARIES "-framework AppKit, -mmacosx-version-min=10.10")
    set_target_properties(discord-rpc PROPERTIES INTERFACE_LINK_LIBRARIES "${DISCORD_LINK_LIBRARIES}")
  endif ()
  if (UNIX)
    # remove another nasty hack
    get_target_property(DISCORD_COMPILE_OPTIONS discord-rpc COMPILE_OPTIONS)
    list(REMOVE_ITEM DISCORD_COMPILE_OPTIONS "-g")
    set_target_properties(discord-rpc PROPERTIES COMPILE_OPTIONS "${DISCORD_COMPILE_OPTIONS}")
  endif ()
endif()

if (NOT WIN32)
  find_package(ZLIB REQUIRED)
  set(ZLIB_LIBRARIES ZLIB::ZLIB CACHE STRING "zlib libraries" FORCE)
endif()

if("${METAFORCE_TARGET_GAME}" STREQUAL "PRIME1")
    add_definitions(-DPRIME1)
elseif("${METAFORCE_TARGET_GAME}" STREQUAL "PRIME2")
    add_definitions(-DPRIME2)
endif()

add_subdirectory(extern/nod)

set(DATA_SPEC_LIBS RetroDataSpec AssetNameMap)
set(HECL_DATASPEC_DECLS
"/* RetroCommon specs */
namespace DataSpec
{
    extern hecl::Database::DataSpecEntry SpecEntMP1;
    extern hecl::Database::DataSpecEntry SpecEntMP1PC;
    extern hecl::Database::DataSpecEntry SpecEntMP1ORIG;
    extern hecl::Database::DataSpecEntry SpecEntMP2;
    extern hecl::Database::DataSpecEntry SpecEntMP2PC;
    extern hecl::Database::DataSpecEntry SpecEntMP2ORIG;
    extern hecl::Database::DataSpecEntry SpecEntMP3;
    extern hecl::Database::DataSpecEntry SpecEntMP3PC;
    extern hecl::Database::DataSpecEntry SpecEntMP3ORIG;
}")
set(HECL_DATASPEC_PUSHES
"    /* RetroCommon */
    hecl::Database::DATA_SPEC_REGISTRY.reserve(9);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP1);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP1PC);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP1ORIG);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP2);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP2PC);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP2ORIG);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP3);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP3PC);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP3ORIG);")

add_subdirectory(extern/athena)
add_subdirectory(extern/boo)
add_subdirectory(hecl/shaderc)
include(hecl/ApplicationTools.cmake)
add_subdirectory(Shaders)
add_subdirectory(hecl/bintoc)
add_subdirectory(imgui)
add_subdirectory(extern/libSquish)
add_subdirectory(extern/libpng)
add_subdirectory(extern/libjpeg-turbo)
add_subdirectory(hecl)
add_subdirectory(extern/zeus)
target_include_directories(hecl-full PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(hecl-light PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hecl-full PRIVATE zeus nod)
target_link_libraries(hecl-light PRIVATE zeus nod)

bintoc(CModelShaders.common.glsl.cpp Shaders/CModelShaders.common.glsl CMODELSHADERS_COMMON_GLSL)
bintoc(CModelShaders.vert.glsl.cpp Shaders/CModelShaders.vert.glsl CMODELSHADERS_VERT_GLSL)
bintoc(CModelShaders.frag.glsl.cpp Shaders/CModelShaders.frag.glsl CMODELSHADERS_FRAG_GLSL)
bintoc(CModelShaders.common.hlsl.cpp Shaders/CModelShaders.common.hlsl CMODELSHADERS_COMMON_HLSL)
bintoc(CModelShaders.vert.hlsl.cpp Shaders/CModelShaders.vert.hlsl CMODELSHADERS_VERT_HLSL)
bintoc(CModelShaders.frag.hlsl.cpp Shaders/CModelShaders.frag.hlsl CMODELSHADERS_FRAG_HLSL)
bintoc(CModelShaders.common.metal.cpp Shaders/CModelShaders.common.metal CMODELSHADERS_COMMON_METAL)
bintoc(CModelShaders.vert.metal.cpp Shaders/CModelShaders.vert.metal CMODELSHADERS_VERT_METAL)
bintoc(CModelShaders.frag.metal.cpp Shaders/CModelShaders.frag.metal CMODELSHADERS_FRAG_METAL)
add_library(CModelShaders
    CModelShaders.common.glsl.cpp CModelShaders.vert.glsl.cpp CModelShaders.frag.glsl.cpp
    CModelShaders.common.hlsl.cpp CModelShaders.vert.hlsl.cpp CModelShaders.frag.hlsl.cpp
    CModelShaders.common.metal.cpp CModelShaders.vert.metal.cpp CModelShaders.frag.metal.cpp)
target_link_libraries(CModelShaders PUBLIC zeus)
target_link_libraries(shader_CModelShaders PUBLIC CModelShaders)

if(NOT TARGET atdna)
  # Import native atdna if cross-compiling
  find_package(atdna REQUIRED)
  if(NOT TARGET atdna)
    message(FATAL_ERROR "atdna required for building Metaforce; please verify LLVM installation")
  endif()
endif()

add_subdirectory(extern/amuse)
add_subdirectory(assetnameparser)
add_compile_definitions(URDE_ZIP_INPUT_STREAM=1) # Enable CZipInputStream now that zlib header is known
add_subdirectory(DataSpec)
add_subdirectory(extern/kabufuda)

add_subdirectory(extern/jbus)
set(JBUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/jbus/include)

add_subdirectory(NESEmulator)
add_subdirectory(Runtime)
add_subdirectory(mpcksum)
add_subdirectory(gbalink)
add_subdirectory(visigen)

add_dependencies(hecl visigen)

if (NOT WINDOWS_STORE AND NOT NX)
  if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
    set(QT_HOMEBREW_PATH /usr/local/opt/qt)
  elseif(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL arm64)
    set(QT_HOMEBREW_PATH /opt/homebrew/opt/qt)
  else()
    set(QT_HOMEBREW_PATH "")
  endif()

  find_package(Qt6Widgets QUIET PATHS ${QT_HOMEBREW_PATH})
  find_package(Qt5Widgets QUIET PATHS ${QT_HOMEBREW_PATH})
  if (Qt6Widgets_FOUND)
    message(STATUS "Qt6 found, metaforce-gui will be built")
    add_subdirectory(metaforce-gui)
  elseif(Qt5Widgets_FOUND)
    message(STATUS "Qt5 found, metaforce-gui will be built")
    add_subdirectory(metaforce-gui)
  else()
    message(STATUS "Qt5-6 not found, metaforce-gui will not be built")
  endif()
endif()

configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_BINARY_DIR}/version.h)
