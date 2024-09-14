dnl Process this file with autoconf to create configure.
AC_PREREQ(2.56)

AC_INIT(icu-le-hb, 2.0.0)

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([icu-le-hb.pc.in])

AM_INIT_AUTOMAKE(1.9)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11)
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(HARFBUZZ, harfbuzz >= 2.0.0)
PKG_CHECK_MODULES(ICU, icu-uc)

AC_CONFIG_FILES([
icu-le-hb.pc
Makefile
src/Makefile
])

AC_OUTPUT
