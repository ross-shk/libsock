/* type_defs-windows.inc - libnet shared types and constants (Windows reference).
 *
 * Values match Winsock2 (ws2tcpip.h / winsock2.h) where they differ
 * from macOS/Linux. This file is copied over include/type_defs.inc at
 * CMake configure time when LIBSOCK_PLATFORM=windows; see CMakeLists.txt.
 */

 /* --- address family / socket type / protocol (POSIX) --------- */
 /* IPv4 only: address strings are 16-byte dotted quads. AF_INET6 */
 /* is reserved for a future revision with wider buffers.         */
%replace AF_INET      by  2;
%replace AF_INET6     by 23;  /* Windows: 23 (AF_INET6); macOS: 30, Linux: 10 */
%replace SOCK_STREAM  by  1;
%replace SOCK_DGRAM   by  2;
%replace IPPROTO_TCP  by  6;
%replace IPPROTO_UDP  by 17;

 /* --- C error number: defined in source/windows/sock.pli (PL/I storage,
  * no C dependency).  Declared external for callers via sock.inc. */

/* --- shutdown(2) directions ---------------------------------- */
%replace SHUT_RD      by 0;
%replace SHUT_WR      by 1;
%replace SHUT_RDWR    by 2;

/* --- poll(2) event bits: libnet mask, mapped onto POLLIN/    */
/* --- POLLOUT by the C bridge (platform POLLOUT is 4, not 2) --- */
%replace POLLIN       by 1;
%replace POLLOUT      by 2;

/* --- condition oncode sentinels (set via SIGNAL ... SET ONCODE)
 * -------------------------------------------------------------- */
/* These are the oncode values carried by the net_timeout / net_eof
 * / net_overflow conditions. POSIX errno (always positive) is
 * carried by the net_error condition. They are NOT return codes:
 * libnet reports all failures through conditions. */
%replace NET_ERR_EOF       by -2;   /* peer closed / end of stream  */
%replace NET_ERR_TIMEOUT   by -3;   /* read/write timeout or EAGAIN */
%replace NET_ERR_OVERFLOW  by -4;   /* caller buffer exhausted      */

/* --- raw socket option levels (platform-specific; Windows shown) ---
 * On Linux, SOL_SOCKET = 1; on macOS, 65535. The PL/I layer passes
 * these directly to the raw sock_setsockopt / sock_getsockopt wrappers.
 * ---------------------------------------------------------------- */
%replace SOL_SOCKET     by 65535;  /* Windows; Linux: 1, macOS: 65535 */
%replace SO_REUSEADDR   by     4;  /* Windows; Linux: 2, macOS: 4     */
%replace SO_KEEPALIVE   by     8;  /* Windows; Linux: 9, macOS: 8     */
%replace SO_LINGER      by   128;  /* Windows; Linux: 13, macOS: 128  */
%replace SO_RCVTIMEO    by  4102;  /* Windows; Linux: 20, macOS: 4102 */
%replace SO_SNDTIMEO    by  4101;  /* Windows; Linux: 21, macOS: 4101 */
%replace SO_ERROR       by  4103;  /* Windows; Linux: 4, macOS: 4103  */

%replace TCP_NODELAY    by     1;  /* portable                 */

/* --- struct sizes (for raw buffer operations) ------------------- */
%replace SOCKADDR_IN_SIZE  by 16;   /* sizeof(struct sockaddr_in) */
/* Winsock sockaddr_in starts with sin_family; there is no sin_len. */
%replace SOCKADDR_HAS_SIN_LEN by 0;
/* Winsock struct addrinfo orders ai_canonname before ai_addr. */
%declare addrinfo_addr_before_canon value(0);
%activate addrinfo_addr_before_canon;
%replace LINGER_SIZE       by  8;   /* sizeof(struct linger)       */
%replace TIMEVAL_SIZE      by 16;   /* sizeof(struct timeval)      */

/* --- legacy net_setopt codes (still used by the public API) ---
 * PL/I maps NETOPT_* to (SOL_SOCKET, SO_*) via map_opt.
 * ---------------------------------------------------------------- */
%replace NETOPT_REUSEADDR by 0;   /* SO_REUSEADDR */
%replace NETOPT_KEEPALIVE  by 1;  /* SO_KEEPALIVE */
%replace NETOPT_NODELAY    by 2;  /* TCP_NODELAY  */

/* --- send/recv flag: nonblocking ------------------------------ */
/* Winsock has no MSG_DONTWAIT equivalent; nonblocking is set via
 * ioctlsocket(FIONBIO) in c_bridge.c. Passing 0 to Winsock
 * send/recv is a safe no-op flag. */
%replace MSG_DONTWAIT  by 0;      /* Windows no-op; Linux: 64, macOS: 128 */

/* --- default buffer capacities -------------------------------- */
%replace NET_BUF_CAP   by 4096;
%replace NET_HOST_MAX  by 256;

/* --- server connection pool ------------------------------------ */
%replace NET_MAX_CLIENTS by 1024;
