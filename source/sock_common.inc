 /* sock_common.inc - shared struct layouts for the sock_*
 * platform variants.
 *
 * Included by both source/posix/sock.pli and source/win/sock.pli.  The
 * platform-specific pollfd layout and fd->SOCKET table live in each
 * variant.
 */


 /* struct sockaddr_in header (bytes 0-3):
 *   Darwin: byte 0 sin_len = SOCKADDR_IN_SIZE (16), byte 1 sin_family
 *   Linux/Windows: bytes 0-1 sin_family = AF_INET (2)
 *   bytes 2-3: sin_port (network/big-endian) */
 dcl phdr pointer;
 dcl 1 sockaddr_hdr based(phdr) unaligned,
       2 w0 fixed bin(31);
