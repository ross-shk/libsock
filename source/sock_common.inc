 /* sock_common.inc - shared struct layouts for the sock_*
 * platform variants.
 *
 * Included by both source/posix/sock.pli and source/win/sock.pli.  The
 * addrinfo field order differs by platform and is selected with the
 * addrinfo_addr_before_canon flag from type_defs.inc; the
 * platform-specific pollfd layout and fd->SOCKET table live in each
 * variant.
 */

 /* struct addrinfo (48 bytes on 64-bit platforms):
 *   ai_flags:    int32  offset 0
 *   ai_family:   int32  offset 4
 *   ai_socktype: int32  offset 8
 *   ai_protocol: int32  offset 12
 *   ai_addrlen:  int64  offset 16
 *   then two pointers at offsets 24 and 32, in platform order:
 *     Darwin/Windows: ai_canonname, ai_addr
 *     Linux:          ai_addr, ai_canonname
 *   ai_next:     char*  offset 40
 * Requires type_defs.inc first (for addrinfo_addr_before_canon). */
 %if addrinfo_addr_before_canon = 1 %then
 dcl 1 addrinfo_t unaligned,
    2 ai_flags     fixed bin(31),
    2 ai_family    fixed bin(31),
    2 ai_socktype  fixed bin(31),
    2 ai_protocol  fixed bin(31),
    2 ai_addrlen   fixed bin(63),
    2 ai_addr      pointer,
    2 ai_canonname pointer,
    2 ai_next      pointer;
 %else
 dcl 1 addrinfo_t unaligned,
    2 ai_flags     fixed bin(31),
    2 ai_family    fixed bin(31),
    2 ai_socktype  fixed bin(31),
    2 ai_protocol  fixed bin(31),
    2 ai_addrlen   fixed bin(63),
    2 ai_canonname pointer,
    2 ai_addr      pointer,
    2 ai_next      pointer;

 /* struct sockaddr_in header (bytes 0-3):
 *   Darwin: byte 0 sin_len = SOCKADDR_IN_SIZE (16), byte 1 sin_family
 *   Linux/Windows: bytes 0-1 sin_family = AF_INET (2)
 *   bytes 2-3: sin_port (network/big-endian) */
 dcl phdr pointer;
 dcl 1 sockaddr_hdr based(phdr) unaligned,
    2 w0 fixed bin(31);
