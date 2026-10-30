 /* sock.inc - libnet socket compatibility layer interface.
  *
  * Module:  sock
  * Purpose: Declare the sock_* procedures (implemented in
  *          source/sock.pli) that provide thin PL/I wrappers over
  *          the raw libc c_* bindings declared in c_bridge.inc.
  *
  * On POSIX (Linux/macOS) c_* resolves to libc symbols directly via
  * external('socket') etc.; no C source is compiled.  On Windows,
  * source/c_bridge.c provides the c_* implementations wrapping
  * Winsock2.
  *
  * This include transitively pulls in c_bridge.inc so callers
  * (e.g. net.pli) get the errno variable and c_bridge constants;
  * net.pli can therefore replace %include c_bridge with %include sock.
  */

 %include c_bridge;

 /*--- Socket lifecycle ---*/

 /* Create a socket.  Wraps libc socket(2).
  * Returns a non-negative file descriptor on success, or -1 on failure
  * (check errno). */
 dcl sock_socket entry (
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
