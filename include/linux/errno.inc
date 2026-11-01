/* errno-linux.inc - POSIX errno constants as %replace (Linux reference).
 *
 * Values are Linux (glibc). This file is copied over include/errno.inc
 * at CMake configure time when LIBSOCK_PLATFORM=linux; see CMakeLists.txt.
 *
 * Note: glibc getaddrinfo EAI_* codes are negative integers, unlike
 * the positive macOS codes. PL/I %replace supports negative values.
 */

%replace EPERM        by   1;  /* Operation not permitted        */
%replace ENOENT       by   2;  /* No such file or directory      */
%replace EINTR        by   4;  /* Interrupted system call        */
%replace EBADF        by   9;  /* Bad file descriptor            */
%replace ECHILD       by  10;  /* No child processes             */
%replace EAGAIN       by  11;  /* Try again (macOS: 35)          */
%replace ENOMEM       by  12;  /* Out of memory                  */
%replace EACCES       by  13;  /* Permission denied              */
%replace EFAULT       by  14;  /* Bad address                    */
%replace EINVAL       by  22;  /* Invalid argument               */
%replace EPIPE        by  32;  /* Broken pipe                    */
%replace EMFILE       by  24;  /* Too many open files            */
%replace ENOTSOCK     by  38;  /* Socket operation on non-socket */
%replace EMSGSIZE     by  40;  /* Message too long               */
%replace EADDRINUSE   by  98;  /* Address already in use         */
%replace EADDRNOTAVAIL by  99; /* Cannot assign requested address */
%replace ENETUNREACH  by 101;  /* Network is unreachable         */
%replace ECONNRESET   by 104;  /* Connection reset by peer       */
%replace ENOBUFS      by 105;  /* No buffer space available      */
%replace ENOTCONN     by 107;  /* Transport endpoint not conn.   */
%replace ETIMEDOUT    by 110;  /* Connection timed out           */
%replace ECONNREFUSED by 111;  /* Connection refused             */
%replace EHOSTDOWN    by 112;  /* Host is down                   */
%replace EHOSTUNREACH by 113;  /* No route to host               */
%replace EINPROGRESS  by  36;  /* Operation now in progress      */

/*--- getaddrinfo(2) error codes (EAI_*) ---
 * Returned by libc getaddrinfo (POSIX); not errno values.
 * sock_getaddrinfo maps them to errno. Values are Linux (glibc);
 * macOS values are noted in each comment.                */
%replace EAI_BADFLAGS   by -1; /* Linux: -1; macOS: 3   */
%replace EAI_AGAIN      by -3; /* Linux: -3; macOS: 2   */
%replace EAI_FAIL       by -4; /* Linux: -4; macOS: 4   */
%replace EAI_FAMILY     by -6; /* Linux: -6; macOS: 5   */
%replace EAI_MEMORY     by -10;/* Linux: -10; macOS: 6  */
%replace EAI_NONAME     by -2; /* Linux: -2; macOS: 8   */
%replace EAI_SERVICE    by -8; /* Linux: -8; macOS: 9   */
%replace EAI_SOCKTYPE   by -7; /* Linux: -7; macOS: 10  */
%replace EAI_SYSTEM     by -11;/* Linux: -11; macOS: 11 */
%replace EAI_OVERFLOW   by -12;/* Linux: -12; macOS: 14 */
