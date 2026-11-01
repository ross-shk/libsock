# libsock — CMake build (portable: Linux, macOS, Windows).
#
# Layout:
#   lib sock        <- compiled source/${LIBSOCK_PLI_SUBDIR}/sock.pli [if the PL/I
#                      compiler is available]  (static lib, installed with headers)
#   include/sock.inc<- cmake-formed self-contained installable interface
#                      (sock_* declarations + type_defs/errno inlined,
#                       no %include dependencies)
#
# Zero C dependency: the socket procedures are implemented entirely in PL/I
# calling libc (POSIX) or Winsock2 (Windows) directly via external() FFI.
# The library target only exists when the PL/I compiler is found.
# Point CMake at the PL/I compiler with -DPLI_EXECUTABLE=... or -DPLI_LLVM=...
cmake_minimum_required(VERSION 3.25)
project(libsock LANGUAGES NONE VERSION 0.1.0)

include(GNUInstallDirs)

set(LIBSOCK_PLIFLAGS "" CACHE STRING "Extra flags forwarded to the PL/I compiler")
set(PLI_LLVM "$ENV{PLI_LLVM}" CACHE PATH "pli-llvm install prefix (bin/pli)")

# Build products (static lib, etc.) land in ./lib at the project root, and
# ./lib ./include are created so consumers (e.g. libnet) can use the tree
# directly without running install.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
     "${CMAKE_CURRENT_SOURCE_DIR}/include")

# ---------------------------------------------------------------- PL/I build
# Zero C dependency: the socket procedures are implemented entirely in PL/I,
# calling libc (POSIX) directly via external() FFI.


# ------------------------------------------------------- PL/I module (opt)
find_program(PLI_EXECUTABLE NAMES pli
  HINTS "${PLI_LLVM}/bin" /usr/local/bin /opt/pli-llvm/bin
  REQUIRED
  DOC "pli-llvm compiler driver")
mark_as_advanced(PLI_EXECUTABLE)

if(PLI_EXECUTABLE)
  set(LIBSOCK_HAVE_PLI TRUE)
else()
  set(LIBSOCK_HAVE_PLI FALSE)
endif()

# ---------------------------------------------------------------- PL/I library
# Zero C dependency: the socket procedures are implemented entirely in PL/I,
# calling libc (POSIX) directly via external() FFI.
# The PL/I object and archive are built below, once LIBSOCK_PLI_SUBDIR and
# LIBSOCK_PLI_INCLUDES are resolved.

# ------------------------------------------------ platform constant selection
# include/errno.inc and include/type_defs.inc are stubs; the real platform
# constants live in include/<platform>/errno.inc and
# include/<platform>/type_defs.inc. CMake copies the selected variant into
# ${CMAKE_CURRENT_BINARY_DIR}/include/ at configure time (file(COPY), no
# build custom commands), so %include errno / %include type_defs in
# source/${LIBSOCK_PLI_SUBDIR}/sock.pli resolve to the platform copy via
# include ordering below.
if(APPLE)
  set(LIBSOCK_PLATFORM darwin)
elseif(WIN32)
  set(LIBSOCK_PLATFORM win)
else()
  set(LIBSOCK_PLATFORM linux)
endif()
set(LIBSOCK_PLATFORM "${LIBSOCK_PLATFORM}" CACHE STRING "Target platform for errno/type_defs constants (darwin/linux/windows)")

# Select the PL/I socket variant by platform: posix for darwin/linux,
# windows for windows.  Each variant calls its platform's socket calls
# directly (model B: no C bridge compiled).
if(LIBSOCK_PLATFORM STREQUAL "win")
  set(LIBSOCK_PLI_SUBDIR win)
else()
  set(LIBSOCK_PLI_SUBDIR posix)
endif()
set(LIBSOCK_PLI_SUBDIR "${LIBSOCK_PLI_SUBDIR}" CACHE STRING "PL/I socket variant subdirectory (posix/windows)")
set(LIBSOCK_TYPE_DEFS_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBSOCK_PLATFORM}/type_defs.inc")
set(LIBSOCK_ERRNO_INC     "${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBSOCK_PLATFORM}/errno.inc")
if(NOT EXISTS "${LIBSOCK_TYPE_DEFS_INC}")
  message(FATAL_ERROR "Platform type_defs file not found: ${LIBSOCK_TYPE_DEFS_INC}")
endif()
if(NOT EXISTS "${LIBSOCK_ERRNO_INC}")
  message(FATAL_ERROR "Platform errno file not found: ${LIBSOCK_ERRNO_INC}")
endif()

# Resolved platform copies used by the PL/I compile and GenDistInc.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
file(READ "${LIBSOCK_TYPE_DEFS_INC}" _LIBSOCK_TD_CONTENT)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include/type_defs.inc" "${_LIBSOCK_TD_CONTENT}")
file(READ "${LIBSOCK_ERRNO_INC}" _LIBSOCK_ERRNO_CONTENT)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include/errno.inc" "${_LIBSOCK_ERRNO_CONTENT}")

set(LIBSOCK_PLI_INCLUDES
  "${LIBSOCK_TYPE_DEFS_INC}"
  "${LIBSOCK_ERRNO_INC}"
  "${CMAKE_CURRENT_BINARY_DIR}/include/type_defs.inc"
  "${CMAKE_CURRENT_BINARY_DIR}/include/errno.inc")

if(LIBSOCK_HAVE_PLI)
  find_program(PLI_AR NAMES ar REQUIRED)
  set(LIBSOCK_PLI_SOCK "${CMAKE_CURRENT_SOURCE_DIR}/source/${LIBSOCK_PLI_SUBDIR}/sock.pli")
  set(LIBSOCK_SOCK_A "${CMAKE_CURRENT_SOURCE_DIR}/lib/libsock${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(LIBSOCK_PLI_OBJ "${CMAKE_CURRENT_BINARY_DIR}/sock.o")
  if(LIBSOCK_PLIFLAGS)
    set(_pli_flags ${LIBSOCK_PLIFLAGS})
  endif()
  add_custom_command(
    OUTPUT "${LIBSOCK_PLI_OBJ}"
    COMMAND "${PLI_EXECUTABLE}" ${_pli_flags}
      -c "${LIBSOCK_PLI_SOCK}"
      -I "${CMAKE_CURRENT_BINARY_DIR}/include"
      -I "${CMAKE_CURRENT_SOURCE_DIR}/include"
      -I "${CMAKE_CURRENT_SOURCE_DIR}/source"
      -o "${LIBSOCK_PLI_OBJ}"
    DEPENDS
      "${LIBSOCK_PLI_SOCK}"
      "${CMAKE_CURRENT_SOURCE_DIR}/source/sock_common.inc"
      ${LIBSOCK_PLI_INCLUDES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "PL/I compiler: compiling source/${LIBSOCK_PLI_SUBDIR}/sock.pli"
    VERBATIM)
  add_custom_command(
    OUTPUT "${LIBSOCK_SOCK_A}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    COMMAND "${PLI_AR}" "rcs" "${LIBSOCK_SOCK_A}" "${LIBSOCK_PLI_OBJ}"
    DEPENDS "${LIBSOCK_PLI_OBJ}"
    COMMENT "Archiving PL/I object into libsock.a"
    VERBATIM)
  add_custom_target(sock ALL DEPENDS "${LIBSOCK_SOCK_A}")
  add_library(libsock::sock INTERFACE IMPORTED)
  set_target_properties(libsock::sock PROPERTIES
    INTERFACE_LINK_LIBRARIES "${LIBSOCK_SOCK_A}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
  message(STATUS "libsock: PL/I compiler found (${PLI_EXECUTABLE}), PL/I module enabled")
else()
  message(STATUS "libsock: PL/I module skipped")
endif()

# ------------------------------------------------------------ include/sock.inc (cmake-formed)
# Self-contained external interface: sock_* declarations with the
# type_defs/errno contents inlined and %include lines stripped,
# so the include/sock.inc dropped into the project tree needs no sibling
# include files and can be installed to /usr/include standalone.
set(LIBSOCK_INC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/include/sock.inc.in")
set(LIBSOCK_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/sock.inc")
add_custom_command(
  OUTPUT "${LIBSOCK_INC}"
  COMMAND "${CMAKE_COMMAND}"
    "-DINC_FILES=${LIBSOCK_INC_TEMPLATE}$<SEMICOLON>${CMAKE_CURRENT_BINARY_DIR}/include/type_defs.inc$<SEMICOLON>${CMAKE_CURRENT_BINARY_DIR}/include/errno.inc"
    "-DOUT_FILE=${LIBSOCK_INC}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenDistInc.cmake"
  DEPENDS
    "${LIBSOCK_INC_TEMPLATE}"
    "${CMAKE_CURRENT_BINARY_DIR}/include/type_defs.inc"
    "${CMAKE_CURRENT_BINARY_DIR}/include/errno.inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenDistInc.cmake"
  COMMENT "Generating cmake-formed include/sock.inc"
  VERBATIM)
add_custom_target(libsock_inc ALL DEPENDS "${LIBSOCK_INC}")

# ---------------------------------------------------------------- install
if(LIBSOCK_HAVE_PLI)
  install(FILES "${LIBSOCK_SOCK_A}" DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/darwin/type_defs.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/linux/type_defs.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/win/type_defs.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/darwin/errno.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/linux/errno.inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/win/errno.inc"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${LIBSOCK_INC}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

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
