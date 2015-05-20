# This generates a ROOT dictionary from a LinkDef file by using rootcint
function (ROOT_GENERATE_DICTIONARY HEADERS LINKDEF_FILE DICTFILE INCLUDE_DIRS)
  # construct -I arguments
  foreach(f ${INCLUDE_DIRS})
    list(APPEND INCLUDE_DIRS_ARGS -I"${f}")   
  endforeach()
  # construct -D arguments
  get_directory_property(DirDefs
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS)
  foreach(f ${DirDefs})
    list(APPEND DEF_ARGS -D${f})
  endforeach()
  

  # also add the outfile with extension .h 
  get_filename_component(DICTFILEDIR ${DICTFILE} PATH)
  get_filename_component(DICTFILENAME_WE ${DICTFILE} NAME_WE)
  get_filename_component(DICTFILENAME ${DICTFILE} NAME)
  set(DICTFILES ${DICTFILE} "${DICTFILEDIR}/${DICTFILENAME_WE}.h")

  # and ensure the output directory exists
  file(MAKE_DIRECTORY ${DICTFILEDIR})
  
  # prepare rootcint command
  if(CMAKE_SYSTEM_NAME MATCHES Linux)
    set(LDPREFIX "LD_LIBRARY_PATH")
  elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
    set(LDPREFIX "DYLD_LIBRARY_PATH")
  else()
    message(FATAL_ERROR "Unsupported System for ROOT Dictionary generation")
  endif()

  
  add_custom_command(OUTPUT ${DICTFILES}
    COMMAND
    ${LDPREFIX}=${ROOT_LIBRARY_DIR}
    ROOTSYS=${ROOTSYS}
    ${ROOT_CINT_EXECUTABLE}
    -f "${DICTFILE}" -c -p ${INCLUDE_DIRS_ARGS} ${DEF_ARGS} ${HEADERS} "${LINKDEF_FILE}"
    DEPENDS ${HEADERS} ${LINKDEF_FILE}
    )

  # this little trick re-runs cmake if the LINKDEF_FILE was changed
  # this is needed since rootcint needs an up-to-date list of input files
  file(RELATIVE_PATH STAMP_FILE ${CMAKE_SOURCE_DIR} ${LINKDEF_FILE})
  string(REPLACE "/" "_" STAMP_FILE ${STAMP_FILE})
  set(STAMP_FILE "${CMAKE_BINARY_DIR}/cmake/stamps/${STAMP_FILE}.stamp")
  configure_file("${LINKDEF_FILE}" "${STAMP_FILE}" COPYONLY)
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${STAMP_FILE}")
  
endfunction()

# This sets everything up such one can build a ROOT library as a target
# it also reports on the found "exe" generating sources
function(ROOT_PREPARE_LIB SOURCEDIRS INCDIRS LINKDEFDIR DICTNAME
    FOUND_LIBSOURCES FOUND_EXESOURCES)            
  #message( "############# Preparing files for ROOT-enabled lib..." )
  
  # find the one and only LinkDef  
  # globbing makes it an absolute path!
  if(${LINKDEFDIR} MATCHES "LinkDef\\.h$")
    # provided linkdefdir is already path to linkdef
    file(GLOB LINKDEF "${LINKDEFDIR}")
    # set dir correctly
    get_filename_component(LINKDEFDIR ${LINKDEF} PATH)
  else()
    # autosearch for LinkDef, useful if name is somewhat different...
    file(GLOB LINKDEF "${LINKDEFDIR}/*LinkDef.h")
  endif()

  # and returns nothing if not found, check that!
  if("${LINKDEF}" STREQUAL "")
    message(FATAL_ERROR "No '*LinkDef.h' file found in ${LINKDEFDIR}")
  endif()
  
  # append location of LinkDef if DICTNAME without (relative) path is specified
  if(${DICTNAME} MATCHES "/")
    set(DICTIONARY ${DICTNAME})
  else()
    set(DICTIONARY "${LINKDEFDIR}/${DICTNAME}")
  endif()
  file(RELATIVE_PATH LINKDEFREL ${CMAKE_SOURCE_DIR} ${LINKDEF})
  #message("---- Setting up ROOT Dict ${DICTIONARY} from ${LINKDEFREL}")

  # make it absolute
  set(DICTIONARY "${CMAKE_CURRENT_BINARY_DIR}/${DICTIONARY}")
  
  foreach(s ${SOURCEDIRS})
    #message("Scanning ${s} for sources...")
    
    # find all sources, but exclude files with a "int main(" function
    # because we want to build a shared library from them, not an executable
    # also exclude ROOT macros, which end with .C
    aux_source_directory("${s}" ALLSOURCES)
    
    foreach(f ${ALLSOURCES})
      file(STRINGS ${f} lines REGEX "main[ ]*\\(")
      #message("File: ${f}")
      if(lines)
        #message("File: ${f}")
        list(APPEND EXESOURCES ${f})
      else()
        # exclude ROOT macros at weird location (ending .C)
        # or some leftover Dict (which produces massive linker warnings)
        #message("File: ${f}")
        if(${f} MATCHES "Dict\\.cc$")
          #message("File: ${f}")
          # track this Dict, we check if it's
          # really leftover and will remove it later on
          list(APPEND LEFTOVERDICTS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
        elseif(NOT f MATCHES "\\.C$")
          #message("Source: ${f}")
          list(APPEND LIBSOURCES ${f})
        endif()
      endif()
    endforeach()
  endforeach()
    
  foreach(h ${INCDIRS})
    #message("Scanning ${h} for headers...")

    # find all headers which are ROOT-enabled
    # They are identified by being mentioned
    # in the LinkDef file
    file(GLOB HEADERS "${h}/*.h")
    file(STRINGS ${LINKDEF} LINKDEF_HAS_CLASSES REGEX "#pragma link C\\+\\+ class")

    foreach(f ${HEADERS})
      get_filename_component(classname ${f} NAME_WE)
      
      if(${f} MATCHES "Dict\\.h$")
        list(APPEND LEFTOVERDICTS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
      elseif(NOT ${classname} MATCHES "LinkDef")
        #message("Header: ${f} ${classname}")
        # only check for classname if LINKDEF has at least one definition
        # else: include unconditionally
        if(LINKDEF_HAS_CLASSES)
          file(STRINGS ${LINKDEF} lines REGEX ${classname})
          if(lines)
            list(APPEND DICTHEADERS "${f}")
          endif()
        else()
          list(APPEND DICTHEADERS "${f}")
        endif()
        # also include all headers (except Dict.h) in LIBSOURCES,
        # this provides "correct" (over-sufficient) dependency checking
        list(APPEND LIBSOURCES ${HEADERS})
      endif()
    endforeach()

    
  endforeach()
  
  get_property(allincdirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY INCLUDE_DIRECTORIES)

  ROOT_GENERATE_DICTIONARY("${DICTHEADERS}" "${LINKDEF}"
    "${DICTIONARY}" "${allincdirs}")

  
  # include the generated Dict in compiling...
  list(APPEND LIBSOURCES ${DICTIONARY})
  #message("DICT: ${DICTIONARY}")

  # make some cleaning of Dicts
  # this makes migrations more stable, hopefully
  # disabled it because LinkDef stamping seems to be enough
  #if(DEFINED LEFTOVERDICTS)
  #  file(REMOVE ${LEFTOVERDICTS})
  #endif()
  list(REMOVE_DUPLICATES LIBSOURCES)
  # tell the caller our findings
  set(${FOUND_LIBSOURCES} ${LIBSOURCES} PARENT_SCOPE)
  set(${FOUND_EXESOURCES} ${EXESOURCES} PARENT_SCOPE)
endfunction()
