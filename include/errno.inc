/* errno.inc - POSIX errno constants (stub).
 *
 * This file is intentionally empty of %replace constants. At CMake
 * configure time it is replaced by a copy of include/<platform>/errno.inc
 * (darwin/linux/windows) into the build tree under include/errno.inc;
 * source/sock.pli and GenDistInc resolve to that platform copy via CMake's
 * include ordering. Do not %include this stub directly for constants.
 */
