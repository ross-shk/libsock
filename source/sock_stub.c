/* sock_stub.c - minimal translation unit for the sock static library.
 *
 * model B compiles no C bridge, so this file guarantees `sock` has at least
 * one source when the PL/I module is not built.  On Windows it additionally
 * defines the errno storage referenced by sock.pli and callers via type_defs
 * (Win32 has no global errno symbol; errno is a macro to *errno()).
 */

#ifdef _WIN32
int errno = 0;
#endif
