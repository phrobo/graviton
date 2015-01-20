prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_PREFIX@/lib@LIB_SUFFIX@
includedir=@CMAKE_INSTALL_PREFIX@/include/graviton-server-@GRAVITON_VERSION_PLATFORM@

Name: graviton-server
Description: graviton server library
Version: @GRAVITON_VERSION@
Requires: glib-2.0 gobject-2.0
Libs: -L${libdir} -lgraviton-server -lgraviton-common
Cflags: -I${includedir}
