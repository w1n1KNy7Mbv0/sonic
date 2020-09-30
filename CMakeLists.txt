project(Sonic)
cmake_minimum_required(VERSION 3.10)
include(ExternalProject)
set(WCOJ_SRC_DIR ${PROJECT_SOURCE_DIR}/src)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif(NOT CMAKE_BUILD_TYPE)


############################# Custom build modes ###############################

set(CMAKE_CXX_FLAGS_SANITIZE "-fno-omit-frame-pointer -fsanitize=address,null -O0 -g"
  CACHE STRING
  "Flags used by the C++ compiler during Sanitize builds."
  FORCE)
set(CMAKE_C_FLAGS_SANITIZE "-fno-omit-frame-pointer -fsanitize=address,null -O0 -g"
  CACHE STRING
  "Flags used by the C compiler during Sanitize builds."
  FORCE)
set(CMAKE_EXE_LINKER_FLAGS_SANITIZE
  ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING
  "Flags used for linking binaries during Sanitize builds."
  FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_SANITIZE
  ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} CACHE STRING
  "Flags used by the shared libraries linker during Sanitize builds."
  FORCE)
mark_as_advanced(
  CMAKE_CXX_FLAGS_SANITIZE		  CMAKE_EXE_LINKER_FLAGS_SANITIZE
  CMAKE_C_FLAGS_SANITIZE		  CMAKE_SHARED_LINKER_FLAGS_SANITIZE
  )

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Sanitize."
  FORCE)

############################ External Dependencies #############################

ExternalProject_Add(catch2
	URL https://github.com/catchorg/Catch2/archive/v2.9.2.tar.gz
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${Sonic_BINARY_DIR}/deps -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

ExternalProject_Add(googlebenchmark
	URL "https://github.com/google/benchmark/archive/v1.5.0.tar.gz"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${Sonic_BINARY_DIR}/deps -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )

ExternalProject_Add(robin-map
	URL "https://github.com/Tessil/robin-map/archive/v0.6.1.tar.gz"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${Sonic_BINARY_DIR}/deps -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )

ExternalProject_Add(taocpp-sequences
  URL https://github.com/taocpp/sequences/archive/2.0.1.tar.gz
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
  )

ExternalProject_Add(hat-trie
  URL https://github.com/Tessil/hat-trie/archive/v0.6.0.tar.gz
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
  )

ExternalProject_Add(abseil
  URL https://github.com/abseil/abseil-cpp/archive/b69c7d880caddfc25bf348dbcfe9d45fdd8bc6e6.zip
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${Sonic_BINARY_DIR}/deps -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )

ExternalProject_Add(surf
  URL https://github.com/efficient/SuRF/archive/be41f2f55970f27149ed85b1c26d1afc369e9225.zip
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
  )

ExternalProject_Add(tlx
  URL https://github.com/tlx/tlx/archive/v0.5.20191212.tar.gz
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
  )

set(VTune_DIR "" CACHE PATH "Where to look for VTune installation")
find_path(ITT_NOTIFY_INCLUDE_DIR "ittnotify.h" HINTS ${VTune_DIR}/include)

################################### Targets ####################################

add_executable(WCOJ 
	"${WCOJ_SRC_DIR}/join_benchmark.cpp"
	)
add_executable(SONIC
	"${WCOJ_SRC_DIR}/sonic_benchmark.cpp"
	)

set_property(TARGET WCOJ PROPERTY CXX_STANDARD 17)
set_property(TARGET SONIC PROPERTY CXX_STANDARD 17)

target_compile_options(WCOJ PUBLIC -msse4.2)
target_compile_options(SONIC PUBLIC -msse4.2)


add_dependencies(WCOJ googlebenchmark)
add_dependencies(SONIC googlebenchmark)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_dependencies(WCOJ robin-map)
add_dependencies(SONIC robin-map)

add_dependencies(WCOJ abseil)
add_dependencies(SONIC abseil)

add_dependencies(WCOJ hat-trie)
add_dependencies(SONIC hat-trie)

add_dependencies(WCOJ surf)
add_dependencies(SONIC surf)

add_dependencies(WCOJ tlx)
add_dependencies(SONIC tlx)

target_link_libraries(WCOJ Threads::Threads)

target_link_libraries(WCOJ -lpthread)

target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}benchmark.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}benchmark_main.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_hash.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_bad_variant_access.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_city.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_raw_hash_set.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_bad_optional_access.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_hashtablez_sampler.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_exponential_biased.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_synchronization.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_graphcycles_internal.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_stacktrace.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_symbolize.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_malloc_internal.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_debugging_internal.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_demangle_internal.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_time.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_strings.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_int128.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_throw_delegate.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_strings_internal.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_base.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_dynamic_annotations.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_spinlock_wait.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_raw_logging_internal.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_log_severity.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_civil_time.a)
target_link_libraries(WCOJ ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_time_zone.a)
if(APPLE)
  target_link_libraries(WCOJ "-framework CoreFoundation")  
endif(APPLE)

target_include_directories(WCOJ SYSTEM PUBLIC ${Sonic_BINARY_DIR}/deps/include)
target_include_directories(WCOJ SYSTEM PUBLIC ${Sonic_BINARY_DIR}/surf-prefix/src)
target_include_directories(WCOJ SYSTEM PUBLIC ${Sonic_BINARY_DIR}/tlx-prefix/src/tlx)
target_include_directories(WCOJ SYSTEM PUBLIC ${Sonic_BINARY_DIR}/hat-trie-prefix/src/hat-trie/include)
target_include_directories(WCOJ SYSTEM PUBLIC ${Sonic_BINARY_DIR}/abseil-prefix/src/abseil)

if(ITT_NOTIFY_INCLUDE_DIR)
  message(STATUS "found itt notify header in ${ITT_NOTIFY_INCLUDE_DIR}")
  target_include_directories(WCOJ SYSTEM PUBLIC ${ITT_NOTIFY_INCLUDE_DIR})
  target_link_libraries(WCOJ ${ITT_NOTIFY_INCLUDE_DIR}/../lib64/libittnotify.a)
  target_link_libraries(WCOJ dl)
  target_compile_options(WCOJ PUBLIC -DWITH_ITT_NOTIFY)
endif(ITT_NOTIFY_INCLUDE_DIR)

target_include_directories(WCOJ SYSTEM PUBLIC ${Sonic_BINARY_DIR}/abseil-prefix/src/abseil/absl/container/internal)

add_dependencies(WCOJ taocpp-sequences)
target_include_directories(WCOJ SYSTEM PUBLIC ${Sonic_BINARY_DIR}/taocpp-sequences-prefix/src/taocpp-sequences/include)


target_link_libraries(SONIC Threads::Threads)

target_link_libraries(SONIC -lpthread)

target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}benchmark.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}benchmark_main.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_hash.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_bad_variant_access.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_city.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_raw_hash_set.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_bad_optional_access.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_hashtablez_sampler.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_exponential_biased.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_synchronization.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_graphcycles_internal.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_stacktrace.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_symbolize.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_malloc_internal.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_debugging_internal.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_demangle_internal.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_time.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_strings.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_int128.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_throw_delegate.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_strings_internal.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_base.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_dynamic_annotations.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_spinlock_wait.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_raw_logging_internal.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_log_severity.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_civil_time.a)
target_link_libraries(SONIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}absl_time_zone.a)
if(APPLE)
  target_link_libraries(SONIC "-framework CoreFoundation")  
endif(APPLE)

target_include_directories(SONIC SYSTEM PUBLIC ${Sonic_BINARY_DIR}/deps/include)
target_include_directories(SONIC SYSTEM PUBLIC ${Sonic_BINARY_DIR}/surf-prefix/src)
target_include_directories(SONIC SYSTEM PUBLIC ${Sonic_BINARY_DIR}/tlx-prefix/src/tlx)
target_include_directories(SONIC SYSTEM PUBLIC ${Sonic_BINARY_DIR}/hat-trie-prefix/src/hat-trie/include)
target_include_directories(SONIC SYSTEM PUBLIC ${Sonic_BINARY_DIR}/abseil-prefix/src/abseil)

if(ITT_NOTIFY_PATH)
  target_include_directories(SONIC SYSTEM PUBLIC ${ITT_NOTIFY_PATH}/include)
  target_compile_definitions(SONIC PUBLIC WITH_ITT_NOTIFY)
  target_link_libraries(SONIC ${ITT_NOTIFY_PATH}/lib64/${CMAKE_SHARED_LIBRARY_PREFIX}ittnotify.a)
  target_link_libraries(SONIC -ldl)
endif(ITT_NOTIFY_PATH)

target_include_directories(SONIC SYSTEM PUBLIC ${Sonic_BINARY_DIR}/abseil-prefix/src/abseil/absl/container/internal)


#################################### Tests #####################################

add_executable(WCOJTests
	"${WCOJ_SRC_DIR}/test/sonic_test.cpp"
	"${WCOJ_SRC_DIR}/test/generic_join_test.cpp"
	)
set_property(TARGET WCOJTests PROPERTY CXX_STANDARD 17)

target_compile_options(WCOJTests PUBLIC -msse4.2)

add_dependencies(WCOJTests googlebenchmark)
add_dependencies(WCOJTests catch2)

target_include_directories(WCOJTests PUBLIC PUBLIC ${Sonic_BINARY_DIR}/deps/include)

add_dependencies(WCOJTests taocpp-sequences)
target_include_directories(WCOJTests SYSTEM PUBLIC ${Sonic_BINARY_DIR}/taocpp-sequences-prefix/src/taocpp-sequences/include)

target_link_libraries(WCOJTests PUBLIC Threads::Threads)

target_link_libraries(WCOJTests PUBLIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}benchmark.a)
target_link_libraries(WCOJTests PUBLIC ${Sonic_BINARY_DIR}/deps/lib/${CMAKE_SHARED_LIBRARY_PREFIX}benchmark_main.a)

if(APPLE)
  target_link_libraries(WCOJTests PUBLIC "-framework CoreFoundation")  
endif(APPLE)
