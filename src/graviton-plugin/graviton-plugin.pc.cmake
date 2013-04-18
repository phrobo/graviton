prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_PREFIX@/lib@LIB_SUFFIX@
includedir=@CMAKE_INSTALL_PREFIX@/include

Name: graviton-plugin
Description: graviton plugin library
Version: @GRAVITON_VERSION@
Requires: glib-2.0
Libs: -L${libdir} -lgraviton-plugin
Cflags: -I${includedir}/graviton
