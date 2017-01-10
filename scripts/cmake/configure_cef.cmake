# Set CEF_PATH to the path to the root of the CEF folder that contains the
# files to be edited.

# Remove the "add_subdirectory(cefclient)" and "add_subdirectory(cefsimple)"
# lines from CEF's CMakeLists.txt.
set(CEF_CMAKELISTS_PATH "CMakeLists.txt")
file(READ ${CEF_CMAKELISTS_PATH} CEF_CMAKELISTS)

string(REPLACE "add_subdirectory(cefclient)" "" CEF_CMAKELISTS ${CEF_CMAKELISTS})
string(REPLACE "add_subdirectory(cefsimple)" "" CEF_CMAKELISTS ${CEF_CMAKELISTS})
string(REGEX REPLACE "add_subdirectory\\(tests/.+\\)" "" CEF_CMAKELISTS ${CEF_CMAKELISTS})

file(WRITE ${CEF_CMAKELISTS_PATH} ${CEF_CMAKELISTS})

message("MSVC_STATIC_RUNTIME: ${MSVC_STATIC_RUNTIME}")

if (NOT MSVC_STATIC_RUNTIME)
    # Replace the "/MT" and "/MTd" linker flags with "/MD" and "/MDd".
    set(CEF_VARIABLES_PATH "cmake/cef_variables.cmake")
    file(READ ${CEF_VARIABLES_PATH} CEF_VARIABLES)

    string(REPLACE "/MT" "/MD" CEF_VARIABLES ${CEF_VARIABLES})

    file(WRITE ${CEF_VARIABLES_PATH} ${CEF_VARIABLES})
endif ()
