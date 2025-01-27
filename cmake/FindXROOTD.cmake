#
# Try to find XROOTD
# Once done this will define
#
#  XROOTD_FOUND - system has XROOTD
#  XROOTD_INCLUDE_DIR - the XROOTD include directory
#  XROOTD_INCLUDE_DIRS - with additonal include directories (non cached)
#  XROOTD_LIBRARIES - The libraries needed to use XROOTD
#  XROOTD_CFLAGS - Additional compilation flags (defines)
#  XROOTD_OLDPACK - old-style packaging for XROOTD libraries
#  XROOTD_NOMAIN - No main available: xproofd not build
#  XROOTD_NOOLDCLNT - No old client available: use built-in version
#

if(XROOTD_XrdPosix_LIBRARY AND XROOTD_INCLUDE_DIR)
  set(XROOTD_FIND_QUIETLY TRUE)
endif()

set(searchpath ${XROOTD_ROOT_DIR} $ENV{XROOTD_DIR} $ENV{XRDSYS} /opt/xrootd)

find_path(XROOTD_INCLUDE_DIR NAMES XrdVersion.hh
  HINTS ${searchpath}
  PATH_SUFFIXES include include/xrootd
)

if (XROOTD_INCLUDE_DIR)
  file(STRINGS ${XROOTD_INCLUDE_DIR}/XrdVersion.hh xrdvers REGEX "^#define XrdVERSION ")
  string(REGEX REPLACE "#define[ ]+XrdVERSION[ ]+" "" xrdvers ${xrdvers})
  string(REGEX REPLACE "[^v\\.]+" "" xrdversdots ${xrdvers})
  if (${xrdversdots} STREQUAL "v..")
    # Regular version string; parse it out
    string(REGEX MATCH "[0-9\\.]+" xrdvers ${xrdvers})
    string(REGEX REPLACE "^([^.]*)\\.(.*)\\.(.*)" "\\1" xrdversmajor ${xrdvers})
    string(REGEX REPLACE "^([^.]*)\\.(.*)\\.(.*)" "\\2" xrdversminor ${xrdvers})
    string(REGEX REPLACE "^([^.]*)\\.(.*)\\.(.*)" "\\3" xrdverspatch ${xrdvers})
    math(EXPR xrdversnum ${xrdversmajor}*100000000+${xrdversminor}*10000+${xrdverspatch})
  else ()
    # Old version string: we keep only the first numerics, i.e. the date
    string(REGEX REPLACE "[v\"]" "" xrdvers ${xrdvers})
    string(SUBSTRING ${xrdvers} 0 8 xrdversnum)
  endif ()
  if ( ${xrdversnum} EQUAL 300030000 )
     SET(XROOTD_FOUND FALSE)
     message(WARNING " >>> Cannot build with XRootD version 3.3.0: please install >=3.3.1 or <= 3.2.x")
  else()
     SET(XROOTD_FOUND TRUE)
  endif ()
endif()

if(XROOTD_FOUND)
  # Set include dirs and compiler macro variable

  if(NOT XROOTD_FIND_QUIETLY )
    message(STATUS "Found Xrootd version num: ${xrdvers} (setting -DROOTXRDVERS=${xrdversnum})")
  endif()
  set(XROOTD_CFLAGS "-DROOTXRDVERS=${xrdversnum}")

  if ( ${xrdversnum} LESS 300010000 )
     set(XROOTD_OLDPACK TRUE)
     set(XROOTD_INCLUDE_DIRS ${XROOTD_INCLUDE_DIR})
     message(STATUS "Setting OLDPACK TRUE")
  else()
     set(XROOTD_OLDPACK FALSE)
     set(XROOTD_INCLUDE_DIRS ${XROOTD_INCLUDE_DIR} ${XROOTD_INCLUDE_DIR}/private)
  endif()
endif()

if(XROOTD_FOUND)
  # Search for the required libraries; this depends on packaging ...

  if(XROOTD_OLDPACK)
    foreach(l XrdPosixPreload XrdPosix)
      find_library(XROOTD_${l}_LIBRARY
         NAMES ${l}
         HINTS ${searchpath}
         PATH_SUFFIXES lib)
      if(XROOTD_${l}_LIBRARY)
		  list(APPEND XROOTD_LIBRARIES ${XROOTD_${l}_LIBRARY})
		endif()
    endforeach()
  else()


    foreach(l XrdPosixPreload XrdPosix)
      find_library(XROOTD_${l}_LIBRARY
         NAMES ${l}
         HINTS ${searchpath}
         PATH_SUFFIXES lib)
      if (XROOTD_${l}_LIBRARY)
         list(APPEND XROOTD_LIBRARIES ${XROOTD_${l}_LIBRARY})
         if(NOT XROOTD_FIND_QUIETLY)
           message(STATUS "             lib${l} found")
         endif()
      else ()
         if(NOT XROOTD_FIND_QUIETLY)
           message(STATUS "             lib${l} not found")
         endif()
		endif()
    endforeach()

  endif()

  if(XROOTD_LIBRARIES)
    set(XROOTD_FOUND TRUE)
    if(NOT XROOTD_FIND_QUIETLY )
      message(STATUS "             XROOTD_INCLUDE_DIR: ${XROOTD_INCLUDE_DIR}")
      message(STATUS "             XROOTD_LIBRARIES: ${XROOTD_LIBRARIES}")
    endif()
  else ()
    set(XROOTD_FOUND FALSE)
  endif ()
endif()

mark_as_advanced(XROOTD_INCLUDE_DIR
                 XROOTD_XrdPosix_LIBRARY
                 XROOTD_XrdPosixPreload_LIBRARY
					  )

