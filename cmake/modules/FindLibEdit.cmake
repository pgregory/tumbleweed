# - Try to find LibEdit
# Once done this will define
#  LIBEDIT_FOUND - System has LibEdit
#  LIBEDIT_INCLUDE_DIRS - The LibEdit include directories
#  LIBEDIT_LIBRARIES - The libraries needed to use LibEdit
#  LIBEDIT_DEFINITIONS - Compiler switches required for using LibEdit

find_package(PkgConfig)
pkg_check_modules(PC_LIBEDIT QUIET libedit)
set(LIBEDIT_DEFINITIONS ${PC_LIBEDIT_CFLAGS_OTHER})

find_path(LIBEDIT_INCLUDE_DIR readline.h
          HINTS ${PC_LIBEDIT_INCLUDEDIR} ${PC_LIBEDIT_INCLUDE_DIRS}
          PATH_SUFFIXES editline)

find_library(LIBEDIT_LIBRARY NAMES edit 
             HINTS ${PC_LIBEDIT_LIBDIR} ${PC_LIBEDIT_LIBRARY_DIRS} )

set(LIBEDIT_LIBRARIES ${LIBEDIT_LIBRARY} )
set(LIBEDIT_INCLUDE_DIRS ${LIBEDIT_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBEDIT to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibEdit  DEFAULT_MSG
                                  LIBEDIT_LIBRARY LIBEDIT_INCLUDE_DIR)

mark_as_advanced(LIBEDIT_INCLUDE_DIR LIBEDIT_LIBRARY )


