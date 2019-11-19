# ---- Include directories ----

#include_directories(${O2_ROOT}/include/Framework)

# ---- Library ----

add_library(QcTPC)

target_sources(QcTPC PRIVATE src/PID.cxx)

target_include_directories(
  QcTPC
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
          ${O2_ROOT}/include/GPU)

target_link_libraries(QcTPC 
                      PUBLIC QualityControl 
			     O2::TPCQC
                             AliceO2::Common)

install(TARGETS QcTPC
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_root_dictionary(QcTPC
                    HEADERS include/TPC/PID.h
                    LINKDEF include/TPC/LinkDef.h
                    BASENAME QcTPC)

# ---- Test(s) ----

set(TEST_SRCS test/testQcTPC.cxx)

foreach(test ${TEST_SRCS})
  get_filename_component(test_name ${test} NAME)
  string(REGEX REPLACE ".cxx" "" test_name ${test_name})

  add_executable(${test_name} ${test})
  target_link_libraries(${test_name}
                        PUBLIC QcTPC
                        PRIVATE Boost::unit_test_framework)
  add_test(NAME ${test_name} COMMAND ${test_name})
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 60)
endforeach()

# ---- Executables ----

#set(EXE_SRCS
#    run/runTPCQCPID.cxx)

#set(EXE_NAMES
#    o2-qc-run-tpcpid)

# These were the original names before the convention changed. We will get rid
# of them but for the time being we want to create symlinks to avoid confusion.
#set(EXE_OLD_NAMES
#    qcRunTPCPID)

add_executable(o2-qc-run-tpcpid run/runTPCQCPID.cxx)
# As per https://stackoverflow.com/questions/35765106/symbolic-links-cmake
#macro(install_symlink filepath sympath)
#  install(DIRECTORY DESTINATION bin) # just in case it is not there yet
#  install(
#    CODE
#      "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})"
#    )
#  install(CODE "message(\"-- Created symlink: ${sympath} -> ${filepath}\")")
#endmacro(install_symlink)

#list(LENGTH EXE_SRCS count)
#math(EXPR count "${count}-1")
#foreach(i RANGE ${count})
#  list(GET EXE_SRCS ${i} src)
#  list(GET EXE_NAMES ${i} name)
#  list(GET EXE_OLD_NAMES ${i} oldname)
#  add_executable(${name} ${src})
#  target_link_libraries(${name} PRIVATE QualityControl CURL::libcurl ROOT::Tree)
#  install_symlink(${name} ${CMAKE_INSTALL_FULL_BINDIR}/${oldname})
#endforeach()

# ---- Install ----

# Build targets with install rpath on Mac to dramatically speed up installation
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
#          "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
#if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#  if("${isSystemDir}" STREQUAL "-1")
#    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
#  endif()
#  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#endif()
#unset(isSystemDir)

# Install library and binaries
#install(TARGETS QualityControl QualityControlTypes ${EXE_NAMES}
#        EXPORT QualityControlTargets
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install headers
#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/QualityControl
#        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# Create version file
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#  "${CMAKE_CURRENT_BINARY_DIR}/cmake/QualityControlConfigVersion.cmake"
#  VERSION ${PACKAGE_VERSION}
#  COMPATIBILITY AnyNewerVersion)

# Export targets
#install(EXPORT QualityControlTargets
#        FILE QualityControlTargets.cmake
#        NAMESPACE QualityControl::
#        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QualityControl)

# Configure and install Config files
#configure_package_config_file(
#  ../cmake/QualityControlConfig.cmake.in
#  ${CMAKE_CURRENT_BINARY_DIR}/cmake/QualityControlConfig.cmake
#  INSTALL_DESTINATION
#  "${CMAKE_INSTALL_LIBDIR}/cmake/QualityControl"
#  PATH_VARS
#  CMAKE_INSTALL_PREFIX)

#install(
#  FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/QualityControlConfig.cmake"
#        "${CMAKE_CURRENT_BINARY_DIR}/cmake/QualityControlConfigVersion.cmake"
#  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QualityControl)

# ---- Install config files ----

install(FILES run/tpcQCPID.json
        DESTINATION etc)
