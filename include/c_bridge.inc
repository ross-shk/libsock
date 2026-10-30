/* c_bridge.inc - raw C entry declarations for libnet.
 *
 * These declare the raw system calls via pli-llvm's by-value FFI
 * (OPTIONS(LINKAGE(SYSTEM)) / OPTIONS(BYVALUE)).  The PL/I-side names
 * use a `c_` prefix and bind to same-named C symbols
 * (`external ('c_socket')`, `external ('c_connect')`, ...); a separate
 * `errno` variable is declared so PL/I can read/clear the C error
 * number.
 *
 * Portability:
 *   - POSIX (Linux/macOS): the libnet static archive contains only the
 *     PL/I objects (sock.o, net.o); no C source is compiled.  The c_*
 *     symbols resolve to libc at executable link time (a POSIX shim
 *     layer mapping c_* to the libc names is required there).
 *   - Windows: source/c_bridge.c (compiled) provides these same
 *     c_*-named symbols wrapping Winsock2, with WSAStartup, an
 *     fd->SOCKET table, and errno translation.
 *
 * ABI notes:
 *   - scalar fixed bin(31) / float params pass by value,
 *     character buffers pass as char* with an explicit length,
 *     pointers pass by value.
 *   - ssize_t/size_t params (recv/send/sendto/recvfrom
 *     length) are wider than fixed bin(31) but, on the arm64 /
 *     x86-64 C ABI, integer args ride in whole registers so a
 *     32-bit value is read correctly.
 */

/* --- fcntl constants (for c_fcntl / sock_setnonblock) ---
 * Values are macOS; Linux noted. */
%replace F_GETFL    by  3;  /* macOS; Linux: 3 */
%replace F_SETFL    by  4;  /* macOS; Linux: 4 */
%replace O_NONBLOCK by  4; /* macOS: 0x4; Linux: 0x4000 (2048) */

 /* --- libc poll(2) event bits ---
  * Distinct from the libnet mask (type_defs POLLIN=1/POLLOUT=2): libc
  * POLLIN is 1 but POLLOUT is 4 (not 2).  sock_poll/sock_poll_multi
  * translate between the two. */
 %replace C_POLLIN   by    1;  /* macOS; Linux: 1  */
 %replace C_POLLOUT  by    4;  /* macOS; Linux: 4  */
 %replace C_POLLERR  by    8;  /* macOS; Linux: 8  */
 %replace C_POLLHUP  by   16;  /* macOS; Linux: 16 */
 %replace C_POLLNVAL by   32;  /* macOS; Linux: 32 */

 /*===============================================================*/
 /* Socket lifecycle (raw syscalls)                               */
 /*===============================================================*/

dcl c_socket entry (fixed bin(31), fixed bin(31), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_socket');

dcl c_close entry (fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_close');

dcl c_shutdown entry (fixed bin(31), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_shutdown');

dcl c_bind entry (fixed bin(31), char(*), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_bind');

dcl c_listen entry (fixed bin(31), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_listen');

dcl c_accept entry (fixed bin(31), pointer, pointer)
     returns (fixed bin(31))
     options (linkage(system)) external ('c_accept');

/*--- Raw connect (takes a raw struct sockaddr_in buffer) ---*/
dcl c_connect entry (fixed bin(31), char(*), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_connect');

/*--- Raw send / recv ---*/
dcl c_send entry (fixed bin(31), char(*), fixed bin(31),
    fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_send');

dcl c_recv entry (fixed bin(31), char(*), fixed bin(31),
    fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_recv');

/*--- Raw sendto / recvfrom (raw sockaddr_in buffers) ---*/
dcl c_sendto entry (fixed bin(31), char(*), fixed bin(31),
    fixed bin(31), char(*), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_sendto');

dcl c_recvfrom entry (fixed bin(31), char(*), fixed bin(31),
    fixed bin(31), char(*), pointer)
     returns (fixed bin(31))
     options (linkage(system)) external ('c_recvfrom');

/*--- Raw socket options ---
 * level/optname are the real SOL_SOCKET/SO_* values; PL/I
 * maps NETOPT_* codes via map_opt. */
dcl c_setsockopt entry (fixed bin(31), fixed bin(31), fixed bin(31),
     pointer, fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_setsockopt');

dcl c_getsockopt entry (fixed bin(31), fixed bin(31), fixed bin(31),
     pointer, pointer)
     returns (fixed bin(31))
     options (linkage(system)) external ('c_getsockopt');

/*--- Raw address queries (fill raw struct sockaddr_in buffer) ---*/
dcl c_getpeername entry (fixed bin(31), char(*), pointer)
     returns (fixed bin(31))
     options (linkage(system)) external ('c_getpeername');

dcl c_getsockname entry (fixed bin(31), char(*), pointer)
     returns (fixed bin(31))
     options (linkage(system)) external ('c_getsockname');

/*--- Nonblocking I/O control (c_fcntl wraps F_GETFL/F_SETFL) ---
 * Always called with three arguments (fd, cmd, arg); for F_GETFL the
 * third is ignored by fcntl. */
dcl c_fcntl entry (fixed bin(31), fixed bin(31), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_fcntl');

/*--- poll(2): single call over an array of struct pollfd ---
 * fds is an array of struct pollfd (fd:int, events:short,
 * revents:short, 8 bytes on macOS/arm64).  nfds is the count;
 * ms is the timeout (-1 = infinite).  Returns the number of
 * ready descriptors. */
dcl c_poll entry (ptr, fixed bin(31), fixed bin(31))
     returns (fixed bin(31))
     options (linkage(system)) external ('c_poll');

/*--- DNS ---
 * getaddrinfo returns EAI_* codes (not errno); sock_getaddrinfo
 * translates them to errno.  Free the result list with
 * freeaddrinfo. */
dcl c_getaddrinfo entry (pointer, pointer, pointer, pointer)
     returns (fixed bin(31))
     options (linkage(system)) external ('c_getaddrinfo');

dcl c_freeaddrinfo entry (pointer)
     options (linkage(system)) external ('c_freeaddrinfo');
