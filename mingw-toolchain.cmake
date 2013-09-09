# Cross-compiling from Linux to Windows.
#
# Takes the PROJECT_ARCH variable, with value "32" or "64".
set (CMAKE_SYSTEM_NAME Windows)

IF (PROJECT_ARCH MATCHES "32")
    set (COMPILER_PREFIX i686-w64-mingw32)
ELSE ()
    set (COMPILER_PREFIX x86_64-w64-mingw32)
ENDIF ()

set (CMAKE_C_COMPILER   ${COMPILER_PREFIX}-gcc)
set (CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set (CMAKE_RC_COMPILER  windres)
    set (CMAKE_RANLIB       ${COMPILER_PREFIX}-gcc-ranlib)
ELSE ()
    set (CMAKE_RC_COMPILER  ${COMPILER_PREFIX}-windres)
    set (CMAKE_RANLIB       ${COMPILER_PREFIX}-ranlib)
ENDIF ()
