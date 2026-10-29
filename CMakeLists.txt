# libsock — CMake build (portable: Linux, macOS, Windows).
#
# Layout:
#   lib sock        <- source/c_bridge.c [+ compiled source/sock.pli if the PL/I
#                      compiler is available]  (static lib, installed with headers)
#   dist/sock.inc   <- include/{sock,c_bridge,errno,type_defs}.inc minus %include lines
#
# The PL/I module is optional: pli-llvm's `pli` compiler is usually absent on
# Windows, so when it is not found the C bridge still builds. Point CMake at the
# PL/I compiler with -DPLI_EXECUTABLE=... or -DPLI_LLVM=...
cmake_minimum_required(VERSION 3.16)
project(libsock LANGUAGES C VERSION 0.1.0)

include(GNUInstallDirs)

option(LIBSOCK_ENABLE_PLI "Build the compiled PL/I module when the PL/I compiler is found" ON)

set(LIBSOCK_PLIFLAGS "" CACHE STRING "Extra flags forwarded to the PL/I compiler")
set(PLI_LLVM "$ENV{PLI_LLVM}" CACHE PATH "pli-llvm install prefix (bin/pli)")

# ---------------------------------------------------------------- C bridge
# On POSIX (Linux/macOS) no C source is compiled: c_* symbols resolve
# to libc directly via pli-llvm's external() bindings. On Windows,
# source/c_bridge.c provides c_* implementations wrapping Winsock2.
if(WIN32)
  add_library(sock STATIC source/c_bridge.c)
else()
  add_library(sock STATIC)
endif()
add_library(libsock::sock ALIAS sock)
set_target_properties(sock PROPERTIES LINKER_LANGUAGE C)
set_target_properties(sock PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
  OUTPUT_NAME sock)
target_compile_options(sock PRIVATE
  $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra>
  $<$<C_COMPILER_ID:MSVC>:/W4>)
if(WIN32)
  # Winsock2 (WSAStartup/WSAPoll/getaddrinfo/...).
  target_link_libraries(sock PUBLIC ws2_32)
  target_compile_definitions(sock PRIVATE _WIN32_WINNT=0x0600 WIN32_LEAN_AND_MEAN)
endif()

# ------------------------------------------------------- PL/I module (opt)
find_program(PLI_EXECUTABLE NAMES pli
  HINTS "${PLI_LLVM}/bin" /usr/local/bin /opt/pli-llvm/bin
  DOC "pli-llvm compiler driver")
mark_as_advanced(PLI_EXECUTABLE)

if(LIBSOCK_ENABLE_PLI AND PLI_EXECUTABLE)
  set(LIBSOCK_HAVE_PLI TRUE)
else()
  set(LIBSOCK_HAVE_PLI FALSE)
endif()

set(LIBSOCK_PLI_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/c_bridge.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/sock.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/type_defs.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/errno.inc")

if(LIBSOCK_HAVE_PLI)
  # Compile the PL/I module; the object joins the static lib alongside
  # the C bridge object (on Windows) or stands alone (on POSIX).
  set(LIBSOCK_PLI_OBJ_SOCK "${CMAKE_CURRENT_BINARY_DIR}/sock${CMAKE_C_OUTPUT_EXTENSION}")
  if(LIBSOCK_PLIFLAGS)
    set(_pli_flags ${LIBSOCK_PLIFLAGS})
  endif()
  add_custom_command(
    OUTPUT "${LIBSOCK_PLI_OBJ_SOCK}"
    COMMAND "${PLI_EXECUTABLE}" ${_pli_flags}
      -c "${CMAKE_CURRENT_SOURCE_DIR}/source/sock.pli"
      -I "${CMAKE_CURRENT_SOURCE_DIR}/include"
      -o "${LIBSOCK_PLI_OBJ_SOCK}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/sock.pli" ${LIBSOCK_PLI_INCLUDES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "PL/I compiler: compiling source/sock.pli"
    VERBATIM)
  target_sources(sock PRIVATE "${LIBSOCK_PLI_OBJ_SOCK}")
  message(STATUS "libsock: PL/I compiler found (${PLI_EXECUTABLE}), PL/I module enabled")
else()
  message(STATUS "libsock: PL/I compiler not found, building C bridge only (PL/I module skipped)")
endif()

# ------------------------------------------------------------ dist/sock.inc
set(LIBSOCK_DIST_INC "${CMAKE_CURRENT_BINARY_DIR}/dist/sock.inc")
add_custom_command(
  OUTPUT "${LIBSOCK_DIST_INC}"
  COMMAND "${CMAKE_COMMAND}"
    "-DINC_FILES=${CMAKE_CURRENT_SOURCE_DIR}/include/sock.inc$<SEMICOLON>${CMAKE_CURRENT_SOURCE_DIR}/include/c_bridge.inc$<SEMICOLON>${CMAKE_CURRENT_SOURCE_DIR}/include/errno.inc$<SEMICOLON>${CMAKE_CURRENT_SOURCE_DIR}/include/type_defs.inc"
    "-DOUT_FILE=${LIBSOCK_DIST_INC}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenDistInc.cmake"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/sock.inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/c_bridge.inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/errno.inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/type_defs.inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenDistInc.cmake"
  COMMENT "Generating dist/sock.inc"
  VERBATIM)
add_custom_target(libsock_dist_inc ALL DEPENDS "${LIBSOCK_DIST_INC}")

# ---------------------------------------------------------------- install
install(TARGETS sock EXPORT libsock-targets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/sock.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/c_bridge.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/errno.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/type_defs.inc"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${LIBSOCK_DIST_INC}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RENAME sock.inc)

if(WIN32)
  set(LIBSOCK_PC_PRIVATE "-lws2_32")
endif()
configure_file(cmake/libsock.pc.in libsock.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsock.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# --------------------------------------------------------------- uninstall
configure_file(
  "cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY)
add_custom_target(libsock_uninstall
  COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
