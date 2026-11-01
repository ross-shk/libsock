# libsock — PL/I socket compatibility layer

A thin socket compatibility layer that provides socket bindings for PL/I programs.

## Requirements

- **pli-llvm** — modern cross-platform open-source PL/I compiler targetting LLVM
  - GitHub: [https://github.com/pli-llvm/pli-llvm](https://github.com/pli-llvm/pli-llvm)
- **CMake ≥ 3.16** — portable build system (Linux, macOS, Windows)

## Quick Start

Build with:

```bash
cmake -S . -B build
cmake --build build
```

and then install (optionally):

```bash
cmake --install build
```

## Usage Examples

### Basic Usage

A basic usage example that resolves [example.com](http://example.com) and reads its contents. Use it similarly to the POSIX socket library, see the full example in [examples/basic_usage.pli](./examples/basic_usage.pli), compile with:

```bash
pli examples/basic_usage.pli -lsock -o basic_usage   # basic_usage.exe on Windows
```

Declare variables holding the socket file descriptor, the URL, request and response buffers, and also byte counts:

```bash
 declare
   fd               fixed bin(31),
   url              char(16),
   request          char(64),
   response         char(4096),
   (sent, n, total) fixed bin(31);
```

Resolve `example.com:80` with error handling:

```c
if sock_getaddrinfo('example.com', 80, url) ^= 0 then do;
  put skip list('getaddrinfo failed, errno=', sock_errno());
  stop;
end;
```

Create a stream socket:

```c
fd = new_socket(AF_INET, SOCK_STREAM, 0);
```

Connect to the resolved address:

```c
if sock_connect(fd, url) ^= 0 then do;
  put skip list('connect failed, errno=', sock_errno());
  call sock_close(fd);
  stop;
end;
```

Send HTTP/1.0 GET request with close:

```bcash
request = 'GET / HTTP/1.0'    || '0D0A'x 
       || 'Host: example.com' || '0D0A'x 
       || 'Connection: close' || '0D0A'x || '0D0A'x;
sent = sock_send(fd, request, 0);
```

Read the response in chunks until the peer closes:

```c
total = 0;
do while ('1'b);
  n = sock_recv(fd, response, 0);

  if n < 0 then do;
    put skip list('recv failed, errno=', sock_errno());
    stop;
  end;
  if n = 0 then leave;

  total = total + n;

  put skip list(substr(response, 1, n));
end;
```

Close the socket:

```c
call sock_close(fd);
```

### Client-Server

A basic TCP echo server and client pair using raw `sock_*` calls. The server binds to `127.0.0.1:9090`, accepts a single connection, reads a message, and echoes it back before exiting; the client connects to `127.0.0.1:9090`, sends a message, reads the echo, and exits. See the full sources in [examples/server.pli](./examples/server.pli) and [examples/client.pli](./examples/client.pli), compile each with:

```bash
pli examples/server.pli -lsock -o server   # server.exe on Windows
pli examples/client.pli -lsock -o client   # client.exe on Windows
```

Resolve `127.0.0.1:9090` into a 16-byte `sockaddr_in` with error handling:

```c
if sock_getaddrinfo('127.0.0.1', 9090, addr) ^= 0 then do;
  put skip list('getaddrinfo failed, errno=', sock_errno());
  stop;
end;
```

Create a stream socket:

```c
fd = new_socket(AF_INET, SOCK_STREAM, 0);
```

Server - bind, listen, accept, echo:

```c
rc = sock_bind(fd, addr);
rc = sock_listen(fd, 5);

cfd = sock_accept(fd);   /* blocks until a client connects */
n = sock_recv(cfd, buf, 0);

rc = sock_send(cfd, buf, 0);   /* echo the bytes back */
```

Client - connect, send, read echo:

```c
rc = sock_connect(fd, addr);

msg = 'Hello, server';
rc = sock_send(fd, msg, 0);

n = sock_recv(fd, buf, 0);
```

Close the sockets:

```c
call sock_close(cfd);
call sock_close(fd);
```

Run the server in one terminal and the client in another:

```bash
./server      # server listening on 127.0.0.1:9090
./client      # client: received echo: Hello, server
```

## Architecture

- **PL/I wrappers — POSIX** (`source/posix/sock.pli`): `sock_*` procedures that call the raw libc socket calls directly via `external()` FFI; no C source is compiled.
- **PL/I wrappers — Windows** (`source/windows/sock.pli`): `sock_*` procedures that call the raw Winsock2 functions directly via `external()`.  `SOCKET` handles are pointer-sized, so a private fd→SOCKET table maps the public 32-bit fd to a `SOCKET` pointer; Winsock error codes are translated to POSIX errno.
- **Shared layouts** (`source/sock_common.inc`): struct layouts (`addrinfo_t`, `sockaddr_hdr`) shared by both variants.
- The `sock_*` procedures wrap the platform calls with argument translation (e.g. poll mask mapping, nonblock control, `getaddrinfo` port injection).  The PL/I interface is identical on both platforms.

## Layout

```text
.
├── CMakeLists.txt        # project build definition
├── LICENSE               # MIT license
├── README.md             # this file
├── cmake/                # CMake modules and pkg-config template
│   ├── GenDistInc.cmake
│   ├── cmake_uninstall.cmake.in
│   └── libsock.pc.in
├── examples/             # PL/I example programs
│   ├── basic_usage.pli   # connect to example.com and read its contents
│   ├── client.pli        # TCP echo client
│   ├── dynamic_buff.pli  # dynamic receive buffer example
│   └── server.pli        # TCP echo server
├── include/              # public PL/I include files
│   ├── darwin/           # macOS-specific defines
│   │   ├── errno.inc     # macOS errno constants
│   │   └── type_defs.inc # macOS socket/type/size constants
│   ├── errno.inc         # POSIX errno constants (E*, EAI_*)
│   ├── linux/            # Linux-specific defines
│   │   ├── errno.inc     # Linux errno constants
│   │   └── type_defs.inc # Linux socket/type/size constants
│   ├── sock.inc          # sock_* interface declarations (both platforms)
│   ├── sock.inc.in       # templated sock.inc
│   ├── type_defs.inc     # shared socket/type constants (AF_*, SOCK_*, ...)
│   └── win/              # Windows-specific defines
│       ├── errno.inc     # Winsock errno translated to POSIX
│       └── type_defs.inc # Windows socket/type/size constants
├── lib/                  # built artifacts
│   └── libsock.a         # compiled static library
├── source/               # PL/I source
│   ├── posix/            # POSIX (libc) implementation
│   │   └── sock.pli      # sock_* wrappers over libc (Linux/macOS)
│   ├── sock_common.inc   # shared struct layouts (addrinfo_t, sockaddr_hdr)
│   └── win/              # Windows (Winsock2) implementation
│       └── sock.pli      # sock_* wrappers over Winsock2 (Windows)
└── zapp.yaml             # Zapp project manifest
```

## CMake options

- `PLI_LLVM` / `PLI_EXECUTABLE` — locate the PL/I compiler
- `LIBSOCK_PLIFLAGS` — extra flags forwarded to the PL/I compiler
