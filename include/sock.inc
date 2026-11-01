 /* sock.inc - libnet socket compatibility layer interface.
  *
  * Module:  sock
 * Purpose: Declare the sock_* procedures (implemented in
 *          source/posix/sock.pli on POSIX and source/windows/sock.pli
 *          on Windows) that provide thin PL/I wrappers over the raw
 *          platform socket calls.  The PL/I interface is identical on
 *          both platforms; only the underlying calls differ.
 *
 * On POSIX (Linux/macOS) each sock_* procedure calls libc directly
 * via external('socket') etc.; no C source is compiled.  On Windows,
 * each sock_* procedure calls Winsock2 directly via external(), with
 * a private fd->SOCKET table held in source/windows/sock.pli.
 *
 * This include pulls in type_defs + errno so callers
 * (e.g. net.pli) get errno constants and the sock_errno procedure.
 */


 dcl sock_errno entry () returns (fixed bin(31)) external;

 /*--- Socket lifecycle ---*/

 /* Create a socket.  Wraps libc socket(2).
  * Returns a non-negative file descriptor on success, or -1 on failure
  * (check errno). */
 dcl new_socket entry (
     fixed bin(31),  /* address family (AF_INET, AF_INET6, ...)       */
     fixed bin(31),  /* socket type (SOCK_STREAM, SOCK_DGRAM, ...)    */
     fixed bin(31)   /* protocol (IPPROTO_TCP, IPPROTO_UDP, or 0)     */
 )  returns (fixed bin(31)) external;

 /* Close a file descriptor.  Wraps libc close(2).
  * Returns 0 on success, or -1 on failure (check errno). */
 dcl sock_close entry (
     fixed bin(31)   /* socket file descriptor to close */
 )  returns (fixed bin(31)) external;

 /* Shut down part of a socket.  Wraps libc shutdown(2).
  * Returns 0 on success, or -1 on failure (check errno). */
 dcl sock_shutdown entry (
     fixed bin(31),  /* file descriptor of the socket to shut down   */
     fixed bin(31)   /* shutdown direction: SHUT_RD, SHUT_WR etc     */
 )  returns (fixed bin(31)) external;

 /* Bind a socket to an address.  Wraps libc bind(2).
  * The PL/I length of addr_buf is passed as the sockaddr length arg.
  * Returns 0 on success, or -1 on failure (check errno). */
 dcl sock_bind entry (
     fixed bin(31),  /* socket file descriptor to bind          */
     char(*)         /* buffer containing a struct sockaddr_in  */
 )  returns (fixed bin(31)) external;

 /* Mark a socket as passive (ready to accept connections).
  * Wraps libc listen(2).  Returns 0 on success, or -1 on failure
  * (check errno). */
 dcl sock_listen entry (
     fixed bin(31),  /* listening socket file descriptor    */
     fixed bin(31)   /* max pending-connection queue length */
 )  returns (fixed bin(31)) external;

 /* Accept a pending connection on a listening socket.
  * Wraps libc accept(2) with NULL sockaddr and addrlen, so the peer
  * address is not returned to the caller.  Blocks until a connection
  * arrives.  Returns a new file descriptor on success, or -1 on failure
  * (check errno). */
 dcl sock_accept entry (
     fixed bin(31)   /* file descriptor of the listening socket */
 )  returns (fixed bin(31)) external;

 /*--- Connect / data transfer ---*/

 /* Initiate a connection on a socket.  Wraps libc connect(2).
  * The PL/I length of addr_buf is passed as the sockaddr length
  * argument.  Returns 0 on success, or for non-blocking sockets -1
  * with errno EINPROGRESS; on failure -1 (check errno). */
 dcl sock_connect entry (
     fixed bin(31),  /* socket file descriptor to connect     */
     char(*)         /* buffer containing sockaddr_in of peer */
 )  returns (fixed bin(31)) external;

 /* Send data on a connected socket.  Wraps libc send(2).
  * The PL/I length of buf is the byte count.  Returns the number of
  * bytes sent (>= 0), or -1 on failure (check errno); a short count
  * (< length(buf)) is possible. */
 dcl sock_send entry (
     fixed bin(31),  /* connected socket file descriptor          */
     char(*),        /* buffer holding data to send               */
     fixed bin(31)   /* send flags (e.g. MSG_DONTWAIT)            */
 )  returns (fixed bin(31)) external;

 /* Receive data on a connected socket.  Wraps libc recv(2).
  * The PL/I length of buf is the maximum byte count.  Returns the
  * number of bytes received (>= 0; 0 means peer closed), or -1 on
  * failure (check errno); a short count is possible. */
 dcl sock_recv entry (
     fixed bin(31),  /* connected socket file descriptor          */
     char(*),        /* buffer to receive data into               */
     fixed bin(31)   /* recv flags (e.g. MSG_DONTWAIT)            */
 )  returns (fixed bin(31)) external;

 /* Send data to a specific peer address.  Wraps libc sendto(2).
  * The PL/I lengths of buf and addr_buf are the byte and sockaddr
  * lengths respectively.  Returns bytes sent (>= 0), or -1 on failure
  * (check errno). */
 dcl sock_sendto entry (
     fixed bin(31),  /* socket fd (may be unconnected for dgrams)  */
     char(*),        /* buffer holding data to send               */
     fixed bin(31),  /* send flags (e.g. MSG_DONTWAIT)            */
     char(*)         /* buffer with sockaddr_in of the peer       */
 )  returns (fixed bin(31)) external;

 /* Receive data and store the sender's address.  Wraps libc recvfrom.
  * The PL/I length of buf is the max byte count; the PL/I length of
  * addr_buf is passed as the sockaddr length.  Returns bytes received
  * (>= 0; 0 = peer closed), or -1 on failure (check errno). */
 dcl sock_recvfrom entry (
     fixed bin(31),  /* receiving socket file descriptor          */
     char(*),        /* buffer to receive data into               */
     fixed bin(31),  /* recv flags (e.g. MSG_DONTWAIT)            */
     char(*),        /* buffer to receive sender's sockaddr_in    */
     pointer         /* ptr to fixed bin(31) receiving actual len */
 )  returns (fixed bin(31)) external;

 /*--- Socket options ---*/

 /* Set a socket option.  Wraps libc setsockopt(2).
  * The address of value_ptr's first byte and the PL/I length of
  * value_ptr are passed as the optval pointer and optlen.  Returns 0
  * on success, or -1 on failure (check errno). */
 dcl sock_setsockopt entry (
     fixed bin(31),  /* socket file descriptor                   */
     fixed bin(31),  /* option level (e.g. SOL_SOCKET, TCP)      */
     fixed bin(31),  /* option name                              */
     char(*)         /* buffer holding the option value          */
 )  returns (fixed bin(31)) external;

 /* Get a socket option.  Wraps libc getsockopt(2).
  * value_ptr receives the option value; optlen_ptr must point to a
  * fixed bin(31) giving the buffer size on input and receiving the
  * actual length on output.  Returns 0 on success, or -1 on failure
  * (check errno). */
 dcl sock_getsockopt entry (
     fixed bin(31),  /* socket file descriptor                      */
     fixed bin(31),  /* option level (e.g. SOL_SOCKET, TCP)         */
     fixed bin(31),  /* option name                                 */
     pointer,        /* ptr to buffer receiving the option value    */
     pointer         /* ptr to fixed bin(31) for size, len out      */
 )  returns (fixed bin(31)) external;

 /*--- Address queries ---*/

 /* Get the peer address of a connected socket.  Wraps libc
  * getpeername(2).  Returns 0 on success, or -1 on failure
  * (check errno). */
 dcl sock_getpeername entry (
     fixed bin(31),  /* socket file descriptor                      */
     char(*),        /* buffer to receive peer's sockaddr_in        */
     pointer         /* ptr to fixed bin(31) receiving actual len   */
 )  returns (fixed bin(31)) external;

 /* Get the local address of a socket.  Wraps libc getsockname(2).
  * Returns 0 on success, or -1 on failure (check errno). */
 dcl sock_getsockname entry (
     fixed bin(31),  /* socket file descriptor                      */
     char(*),        /* buffer to receive local sockaddr_in         */
     pointer         /* ptr to fixed bin(31) receiving actual len   */
 )  returns (fixed bin(31)) external;

 /*--- Nonblocking I/O ---*/

 /* Set or clear nonblocking mode on a file descriptor.
  * Uses fcntl(F_GETFL) to read current flags, bitwise OR/ANDs the
  * O_NONBLOCK bit, then fcntl(F_SETFL) to write them back.  Returns 0
  * on success, or -1 on failure (check errno). */
 dcl sock_setnonblock entry (
     fixed bin(31),  /* file descriptor to modify                   */
     fixed bin(31)   /* non-zero enables nonblocking, zero disables */
 )  returns (fixed bin(31)) external;

 /*--- poll(2) ---*/

 /* Poll a single socket for readiness.
  * Translates the libnet event mask (POLLIN=1, POLLOUT=2) to libc
  * poll(2) bits (C_POLLIN, C_POLLOUT), calls poll(2) with nfds=1.
  * Revents are not translated for this variant.  Returns the count of
  * ready descriptors (0 = timed out) or -1 on failure (check errno). */
 dcl sock_poll entry (
     fixed bin(31),  /* file descriptor to poll                     */
     fixed bin(31),  /* desired events: POLLIN and/or POLLOUT       */
     fixed bin(31)   /* timeout ms: -1 block, 0 immediate, else ms  */
 )  returns (fixed bin(31)) external;

 /* Poll multiple file descriptors for readiness.
  * Builds an array of struct pollfd from the caller-supplied fd and
  * event arrays, then calls libc poll(2).  Libnet event masks are
  * translated to libc poll bits for input, and libc revents are
  * translated back to libnet masks for output.  Returns the count of
  * ready descriptors (0 = timed out) or -1 on failure (check errno). */
 dcl sock_poll_multi entry (
     pointer,        /* ptr to array of fixed bin(31) descriptors   */
     pointer,        /* ptr to array of fixed bin(31) event masks   */
     pointer,        /* ptr to array of revent masks (out)          */
     fixed bin(31),  /* number of descriptors to poll               */
     fixed bin(31)   /* timeout ms: -1 block, 0 immediate, else ms  */
 )  returns (fixed bin(31)) external;

 /*--- DNS ---*/

 /* Resolve a host name and inject a port, returning a raw sockaddr_in.
  * Wraps libc getaddrinfo(2): sets hints to AF_INET / SOCK_STREAM,
  * resolves the host, copies the first result's sockaddr_in to
  * addr_buf, overwrites sin_port with the requested port (network
  * byte order), then frees the addrinfo list.  EAI_* error codes are
  * translated to errno.  Returns 0 on success, or -1 on failure
  * (check errno, which may be ENOMEM, ENOENT, or EINVAL). */
 dcl sock_getaddrinfo entry (
     char(*),        /* host name to resolve                       */
     fixed bin(31),  /* port number (host byte order)              */
     char(*)         /* 16-byte buffer for resolved sockaddr_in    */
 )  returns (fixed bin(31)) external;

 /* Free a linked list of addrinfo structures returned by getaddrinfo.
  * Wraps libc freeaddrinfo(2). */
 dcl sock_freeaddrinfo entry (
     pointer         /* pointer to the addrinfo list to free */
 ) external;
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
/* errno-windows.inc - POSIX errno constants as %replace (Windows reference).
 *
 * Values match the errno normalization in source/c_bridge.c (WSA codes
 * translated to the LNX_* table, which equals the darwin table below).
 * This file is copied over include/errno.inc at CMake configure time
 * when LIBSOCK_PLATFORM=windows; see CMakeLists.txt.
 *
 * Windows getaddrinfo returns raw WSA error codes via ws2tcpip.h;
 * EAI_* codes are not defined there and mapping them is out of scope
 * (see the pre-existing mismatch in sock_getaddrinfo).
 */

%replace EPERM        by   1;  /* Operation not permitted        */
%replace ENOENT       by   2;  /* No such file or directory      */
%replace EINTR        by   4;  /* Interrupted system call        */
%replace EBADF        by   9;  /* Bad file descriptor            */
%replace ECHILD       by  10;  /* No child processes             */
%replace EAGAIN       by  35;  /* Try again                      */
%replace ENOMEM       by  12;  /* Out of memory                  */
%replace EACCES       by  13;  /* Permission denied              */
%replace EFAULT       by  14;  /* Bad address                    */
%replace EINVAL       by  22;  /* Invalid argument               */
%replace EPIPE        by  32;  /* Broken pipe                    */
%replace EMFILE       by  24;  /* Too many open files            */
%replace ENOTSOCK     by  38;  /* Socket operation on non-socket */
%replace EMSGSIZE     by  40;  /* Message too long               */
%replace EADDRINUSE   by  48;  /* Address already in use         */
%replace EADDRNOTAVAIL by  49; /* Cannot assign requested address */
%replace ENETUNREACH  by  51;  /* Network is unreachable         */
%replace ECONNRESET   by  54;  /* Connection reset by peer       */
%replace ENOBUFS      by  55;  /* No buffer space available      */
%replace ENOTCONN     by  57;  /* Transport endpoint not conn.   */
%replace ETIMEDOUT    by  60;  /* Connection timed out           */
%replace ECONNREFUSED by  61;  /* Connection refused             */
%replace EHOSTDOWN    by  64;  /* Host is down                   */
%replace EHOSTUNREACH by  65;  /* No route to host               */
%replace EINPROGRESS  by  36;  /* Operation now in progress      */

/* Windows: EAI_* constants not defined in ws2tcpip.h (raw WSA codes).
 * Left intentionally undefined here. */
