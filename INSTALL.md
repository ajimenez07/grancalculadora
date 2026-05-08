## Installation

This software is in development and by now there are no
releases available. However, you can build it by yourself
using Autoconf:

```bash
autoreconf -i
./configure CPPFLAGS="-DDEBUG"
make
./gran_calculadora
```

You will need GCC, Autoconf and GTKmm installed.

If you are on Windows build it with Mingw, but take into account that no tests have been done yet on this OS.

**Note**: GTKmm already includes Cairo.
