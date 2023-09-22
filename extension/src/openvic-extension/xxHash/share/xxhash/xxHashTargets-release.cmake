#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "xxHash::xxhash" for configuration "Release"
set_property(TARGET xxHash::xxhash APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(xxHash::xxhash PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/xxhash.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/xxhash.dll"
  )

list(APPEND _cmake_import_check_targets xxHash::xxhash )
list(APPEND _cmake_import_check_files_for_xxHash::xxhash "${_IMPORT_PREFIX}/lib/xxhash.lib" "${_IMPORT_PREFIX}/bin/xxhash.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
