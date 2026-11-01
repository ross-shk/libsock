/* sock_common.inc - shared struct layouts for the sock_* platform variants.
 *
 * Included by both source/posix/sock.pli and source/windows/sock.pli so the
 * two variants stay in sync.  Contains only the struct layouts that are
 * identical on POSIX and Windows (64-bit pointer model); the platform-specific
 * pollfd layout and fd->SOCKET table live in each variant.
 */

/* struct addrinfo (48 bytes on 64-bit platforms):
 *   ai_flags:    int32  offset 0
 *   ai_family:   int32  offset 4
 *   ai_socktype: int32  offset 8
 *   ai_protocol: int32  offset 12
 *   ai_addrlen:  int64  offset 16
 *   ai_canonname: char* offset 24
 *   ai_addr:     char*  offset 32
 *   ai_next:     char*  offset 40 */
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
 *   byte 0: sin_len    = SOCKADDR_IN_SIZE (16)
 *   byte 1: sin_family = AF_INET (2)
 *   bytes 2-3: sin_port (network/big-endian) */
dcl phdr pointer;
dcl 1 sockaddr_hdr based(phdr) unaligned,
    2 w0 fixed bin(31);
