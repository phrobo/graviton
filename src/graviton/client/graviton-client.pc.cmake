prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_PREFIX@/lib@LIB_SUFFIX@
includedir=@CMAKE_INSTALL_PREFIX@/include/graviton-client-@GRAVITON_VERSION_PLATFORM@

Name: graviton-client
Description: graviton client library
Version: @GRAVITON_VERSION@
Requires: glib-2.0
Libs: -L${libdir} -lgraviton-client
Cflags: -I${includedir}
