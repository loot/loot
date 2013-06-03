# Cross-compiling from Linux to Windows.
#
# Takes the PROJECT_ARCH variable, with value "32" or "64".
set (CMAKE_SYSTEM_NAME Windows)

IF (PROJECT_ARCH MATCHES "32")
    set (MINGW i686-w64-mingw32)
ELSE ()
    set (MINGW x86_64-w64-mingw32)
ENDIF ()

set (CMAKE_C_COMPILER   ${MINGW}-gcc)
set (CMAKE_CXX_COMPILER ${MINGW}-g++)
set (CMAKE_RC_COMPILER  ${MINGW}-windres)
set (CMAKE_RANLIB       ${MINGW}-ranlib)
