#enable_testing()
file(GLOB LB_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test*.cc")
file(GLOB LB_BENCH_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*bench*.cc")

#list(APPEND LB_TEST_BENCH_SOURCES ${LB_TEST_SOURCES} ${LB_BENCH_SOURCES})

############################################
# User can determine whether to build all tests when build target all
# e.g. cmake --build */lb/build [--target all -j 2]
############################################
set(BUILD_ALL_TESTS OFF CACHE BOOL "Don't exclude tests from target all")

message(STATUS "lb test source files: ${LB_TEST_SOURCES}")
message(STATUS "lb bench source files: ${LB_BENCH_SOURCES}")

if (NOT ${BUILD_ALL_TESTS})
  message(STATUS "[The lb test and bench files are excluded from the build of target all]")
endif (NOT ${BUILD_ALL_TESTS})

# find_package(absl REQUIRED)
############################################
# generate test and bench program
############################################
macro (GenTest SRC)
  foreach (lb_test_source ${${SRC}})
    get_filename_component(lb_test_filename ${lb_test_source} NAME_WE)

    if (${BUILD_ALL_TESTS})
      add_executable(${lb_test_filename} ${lb_test_source})
    else ()
      message(STATUS "${lb_test_filename} is excluded from target all")
      add_executable(${lb_test_filename} EXCLUDE_FROM_ALL ${lb_test_source})
    endif (${BUILD_ALL_TESTS})
  
    # target_link_libraries(${lb_test_filename} ${ARGN})
    target_link_libraries(${lb_test_filename} ${ARGN} absl_strings)
    set_target_properties(${lb_test_filename}
      PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
      COMMAND ${lb_test_filename}
    )

    add_test(NAME ${lb_test_filename} COMMAND ${lb_test_filename})
  endforeach (lb_test_source ${LB_TEST_SOURCES})
endmacro()

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  set(LB_LIB lb)
else ()
  set(LB_LIB lb_debug)
endif()
GenTest(LB_TEST_SOURCES gtest gtest_main ${LB_LIB} kanon_net kanon_base)
GenTest(LB_BENCH_SOURCES benchmark benchmark_main ${LB_LIB} kanon_net kanon_base)
