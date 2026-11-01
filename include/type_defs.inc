 /* type_defs.inc - libnet shared types and constants (stub).
 *
 * This file is intentionally empty of %replace constants and the errno
 * declaration. At CMake configure time it is replaced by a copy of
 * include/<platform>/type_defs.inc (darwin/linux/windows) into the
 * build tree under include/type_defs.inc; source/sock.pli and
 * GenDistInc resolve to that platform copy via CMake's include
 * ordering. Do not %include this stub directly for constants.
 */
