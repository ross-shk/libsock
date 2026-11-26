 /* addrinfo_view.inc - linux variant of res_view struct layout.
  *
  * Linux struct addrinfo orders ai_addr before ai_canonname.
  * Included inside sock_getaddrinfo() in source/posix/sock.pli.
  */
 dcl 1 res_view based(res_ptr) unaligned,
       2 rai_flags   fixed bin(31),
       2 rai_family  fixed bin(31),
       2 rai_socktype fixed bin(31),
       2 rai_protocol fixed bin(31),
       2 rai_addrlen fixed bin(63),
       2 rai_addr    pointer,
       2 rai_canonname pointer,
       2 rai_next    pointer;
