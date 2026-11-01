# libsock — PL/I socket compatibility layer

A thin socket compatibility layer that provides socket bindings for PL/I programs.

## Requirements

- **pli-llvm** — modern open-source PL/I compiler targetting LLVM
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

## Usage Example

Use it similarly to the POSIX socket library, see the full example in [examples/simple_usage.pli](./examples/simple_usage.pli), compile with:

```bash
pli examples/simple_usage.pli -lsock -o simple_usage # simple_usage.exe on Windows
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
request = 'GET / HTTP/1.0' || '0D0A'x || 'Host: example.com'
       || '0D0A'x || 'Connection: close' || '0D0A'x || '0D0A'x;
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

## Architecture

- **PL/I wrappers — POSIX** (`source/posix/sock.pli`): `sock_*` procedures that call the raw libc socket calls directly via `external()` FFI; no C source is compiled.
- **PL/I wrappers — Windows** (`source/windows/sock.pli`): `sock_*` procedures that call the raw Winsock2 functions directly via `external()`.  `SOCKET` handles are pointer-sized, so a private fd→SOCKET table maps the public 32-bit fd to a `SOCKET` pointer; Winsock error codes are translated to POSIX errno.
- **Shared layouts** (`source/sock_common.inc`): struct layouts (`addrinfo_t`, `sockaddr_hdr`) shared by both variants.
- The `sock_*` procedures wrap the platform calls with argument translation (e.g. poll mask mapping, nonblock control, `getaddrinfo` port injection).  The PL/I interface is identical on both platforms.

## Layout

| Path | Contents |
| --- | --- |
| `source/posix/sock.pli` | PL/I wrappers (`sock_*`) over libc (Linux/macOS) |
| `source/windows/sock.pli` | PL/I wrappers (`sock_*`) over Winsock2 (Windows) |
| `source/sock_common.inc` | shared struct layouts (`addrinfo_t`, `sockaddr_hdr`) |
| `include/sock.inc` | `sock_*` interface declarations (identical on both platforms) |
| `include/errno.inc` | POSIX errno constants (`E*` + `EAI_*`) |
| `include/type_defs.inc` | socket/type constants (`AF_*`, `SOCK_*`, `SOL_SOCKET`, `NET_*`, sizes) |

## CMake options

- `LIBSOCK_ENABLE_PLI` (default ON) — build the compiled PL/I module when the
  PL/I compiler is found
- `PLI_LLVM` / `PLI_EXECUTABLE` — locate the pli-llvm compiler
- `LIBSOCK_PLIFLAGS` — extra flags forwarded to the PL/I compiler
