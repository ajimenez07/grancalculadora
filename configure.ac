AC_INIT([GranCalculadora], [0.1], [ajimenezba@edu.tecnocampus.cat])

# license
AC_COPYRIGHT([Copyright (c) 2026 Álex Jiménez])

# init automake
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects foreign])

# check c++ compiler
AC_PROG_CXX
CXXFLAGS="$CXXFLAGS -std=c++17"

# check c++ preprocessor
AC_PROG_CXXCPP

# check gtkmm
PKG_CHECK_MODULES([GTKMM], [gtkmm-4.0])

# output files
AC_CONFIG_FILES([Makefile])
AC_OUTPUT


