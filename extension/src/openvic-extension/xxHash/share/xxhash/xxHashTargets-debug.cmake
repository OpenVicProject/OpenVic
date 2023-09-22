#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "xxHash::xxhash" for configuration "Debug"
set_property(TARGET xxHash::xxhash APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(xxHash::xxhash PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/xxhash.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/xxhash.dll"
  )

list(APPEND _cmake_import_check_targets xxHash::xxhash )
list(APPEND _cmake_import_check_files_for_xxHash::xxhash "${_IMPORT_PREFIX}/debug/lib/xxhash.lib" "${_IMPORT_PREFIX}/debug/bin/xxhash.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
