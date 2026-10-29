# libsock — PL/I socket compatibility layer

A thin socket compatibility layer that provides `c_*` and `sock_*` bindings for
PL/I programs. It is consumed as an external black-box dependency by
[libnet](https://github.com/ross-shk/libnet), which builds and links against
`libsock.a` rather than compiling the socket source itself.

## Requirements

- **pli-llvm** — modern open-source PL/I compiler targetting LLVM (optional;
  when absent, only the C bridge is built)
  - GitHub: [https://github.com/pli-llvm/pli-llvm](https://github.com/pli-llvm/pli-llvm)
- **C compiler** (clang/gcc/MSVC/MinGW) — for `c_bridge.c` (Windows only)
- **CMake ≥ 3.16** — portable build system (Linux, macOS, Windows)

> **Platform note:** `include/errno.inc` and `include/type_defs.inc` currently
> contain macOS arm64 socket/errno values. Linux/Windows need matching
> constant files.

## Quick Start

```bash
cmake -S . -B build
cmake --build build

# Install (libnet consumes an installed libsock)
cmake --install build --prefix /usr/local
```

Then build libnet:

```bash
cmake -S /path/to/libnet -B build -DLIBSOCK_ROOT=/usr/local
cmake --build build
```

## Architecture

- **C bridge** (`source/c_bridge.c`): the ONLY C — thin syscall wrappers
  (`c_*` naming) around Winsock2 on Windows. On POSIX, the `c_*` symbols
  resolve to libc directly via pli-llvm's by-value FFI; no C source is
  compiled.
- **PL/I wrappers** (`source/sock.pli`): `sock_*` procedures that wrap the raw
  `c_*` syscalls with argument translation (e.g. poll mask mapping,
  `fcntl`-based nonblock control, `getaddrinfo` port injection).

## Layout

| Path | Contents |
| --- | --- |
| `source/c_bridge.c` | the ONLY C — thin Winsock2 syscall wrappers (`c_*`) |
| `source/sock.pli` | PL/I wrappers (`sock_*`) over the raw `c_*` syscalls |
| `include/sock.inc` | `sock_*` interface declarations (transitively includes `c_bridge.inc`) |
| `include/c_bridge.inc` | raw `c_*` FFI declarations + `errno`, fcntl/poll constants |
| `include/errno.inc` | POSIX errno constants (`E*` + `EAI_*`) |
| `include/type_defs.inc` | socket/type constants (`AF_*`, `SOCK_*`, `SOL_SOCKET`, `NET_*`, sizes) |

## CMake options

- `LIBSOCK_ENABLE_PLI` (default ON) — build the compiled PL/I module when the
  PL/I compiler is found
- `PLI_LLVM` / `PLI_EXECUTABLE` — locate the pli-llvm compiler
- `LIBSOCK_PLIFLAGS` — extra flags forwarded to the PL/I compiler
