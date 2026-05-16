## Installation

This software is a completed, functional project, but no pre-built releases are available. You can build it yourself using Autoconf.

**Requirements:** GCC, Autoconf, and GTKmm (which includes Cairo).

### Building on Linux

```bash
autoreconf -i
./configure
make
./gran_calculadora
```

### A note on debugging

If you wish to build with debug symbols, use:

```bash
./configure CPPFLAGS="-DDEBUG"
```

### Building on other platforms

*   **Windows**: You may build it with MinGW, but please note that the software has not been tested on this OS.
*   **macOS**: No tests have been performed on this platform.
