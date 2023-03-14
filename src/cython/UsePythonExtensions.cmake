#.rst:
#
# The following functions are defined:
#
# .. cmake:command:: add_python_library
#
# Add a library that contains a mix of C, C++, Fortran, Cython, F2PY, Template,
# and Tempita sources. The required targets are automatically generated to
# "lower" source files from their high-level representation to a file that the
# compiler can accept.
#
#
#   add_python_library(<Name>
#                      SOURCES [source1 [source2 ...]]
#                      [INCLUDE_DIRECTORIES [dir1 [dir2 ...]]
#                      [LINK_LIBRARIES [lib1 [lib2 ...]]
#                      [DEPENDS [source1 [source2 ...]]])
#
#
# Example usage
# ^^^^^^^^^^^^^
#
# .. code-block:: cmake
#
#   find_package(PythonExtensions)
#
#   file(GLOB arpack_sources ARPACK/SRC/*.f ARPACK/UTIL/*.f)
#
#    add_python_library(arpack_scipy
#      SOURCES ${arpack_sources}
#              ${g77_wrapper_sources}
#      INCLUDE_DIRECTORIES ARPACK/SRC
#    )
#
# .. cmake:command:: add_python_extension
#
# Add a extension that contains a mix of C, C++, Fortran, Cython, F2PY, Template,
# and Tempita sources. The required targets are automatically generated to
# "lower" source files from their high-level representation to a file that the
# compiler can accept.
#
#
#   add_python_extension(<Name>
#                        SOURCES [source1 [source2 ...]]
#                        [INCLUDE_DIRECTORIES [dir1 [dir2 ...]]
#                        [LINK_LIBRARIES [lib1 [lib2 ...]]
#                        [DEPENDS [source1 [source2 ...]]])
#
#
# Example usage
# ^^^^^^^^^^^^^
#
# .. code-block:: cmake
#
#   find_package(PythonExtensions)
#
#   file(GLOB arpack_sources ARPACK/SRC/*.f ARPACK/UTIL/*.f)
#
#    add_python_extension(arpack_scipy
#      SOURCES ${arpack_sources}
#              ${g77_wrapper_sources}
#      INCLUDE_DIRECTORIES ARPACK/SRC
#    )
#
#
#=============================================================================
# Copyright 2011 Kitware, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=============================================================================

macro(_remove_whitespace _output)
  string(REGEX REPLACE "[ \r\n\t]+" " " ${_output} "${${_output}}")
  string(STRIP "${${_output}}" ${_output})
endmacro()

function(add_python_library _name)
  set(options STATIC SHARED MODULE)
  set(multiValueArgs SOURCES INCLUDE_DIRECTORIES LINK_LIBRARIES COMPILE_DEFINITIONS DEPENDS)
  cmake_parse_arguments(_args "${options}" "" "${multiValueArgs}" ${ARGN} )

  # Validate arguments to allow simpler debugging
  if(NOT _args_SOURCES)
    message(
      FATAL_ERROR
      "You have called add_python_library for library ${_name} without "
      "any source files. This typically indicates a problem with "
      "your CMakeLists.txt file"
    )
  endif()

  # Initialize the list of sources
  set(_sources ${_args_SOURCES})

  # Generate targets for all *.src files
  set(_processed )
  foreach(_source IN LISTS _sources)
    if(${_source} MATCHES .pyf.src$ OR ${_source} MATCHES \\.f\\.src$)
      if(NOT NumPy_FOUND)
        message(
          FATAL_ERROR
          "NumPy is required to process *.src Template files"
        )
      endif()
      string(REGEX REPLACE "\\.[^.]*$" "" _source_we ${_source})
      add_custom_command(
        OUTPUT ${_source_we}
        COMMAND ${NumPy_FROM_TEMPLATE_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/${_source}
                ${CMAKE_CURRENT_BINARY_DIR}/${_source_we}
        DEPENDS ${_source} ${_args_DEPENDS}
        COMMENT "Generating ${_source_we} from template ${_source}"
      )
      list(APPEND _processed ${_source_we})
    elseif(${_source} MATCHES \\.c\\.src$)
      if(NOT NumPy_FOUND)
        message(
          FATAL_ERROR
          "NumPy is required to process *.src Template files"
        )
      endif()
      string(REGEX REPLACE "\\.[^.]*$" "" _source_we ${_source})
      add_custom_command(
        OUTPUT ${_source_we}
        COMMAND ${NumPy_CONV_TEMPLATE_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/${_source}
                ${CMAKE_CURRENT_BINARY_DIR}/${_source_we}
        DEPENDS ${_source} ${_args_DEPENDS}
        COMMENT "Generating ${_source_we} from template ${_source}"
      )
      list(APPEND _processed ${_source_we})
    elseif(${_source} MATCHES \\.pyx\\.in$)
      if(NOT Cython_FOUND)
        message(
          FATAL_ERROR
          "Cython is required to process *.in Tempita files"
        )
      endif()
      string(REGEX REPLACE "\\.[^.]*$" "" _source_we ${_source})
      configure_file(
          ${CMAKE_CURRENT_SOURCE_DIR}/${_source}
          ${CMAKE_CURRENT_BINARY_DIR}/${_source}
          COPYONLY
      )
      set(_tempita_command
          "
            import os;
            import sys;
            from Cython.Tempita import Template;
            cwd = os.getcwd();
            open(os.path.join(cwd, '${_source_we}'), 'w+')
            .write(
                Template.from_filename(os.path.join(cwd, '${_source}'),
                encoding=sys.getdefaultencoding()).substitute()
            )
          "
      )
      _remove_whitespace(_tempita_command)
      add_custom_command(
        OUTPUT ${_source_we}
        COMMAND ${PYTHON_EXECUTABLE} -c "${_tempita_command}"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_source}"
                ${_args_DEPENDS}
      )
      list(APPEND _processed ${_source_we})
    else()
      list(APPEND _processed  ${_source})
    endif()
  endforeach()
  set(_sources ${_processed})

  # If we're building a Python extension and we're given only Fortran sources,
  # We can conclude that we need to generate a Fortran interface file
  list(FILTER _processed EXCLUDE REGEX "(\\.f|\\.f90)$")
  if(NOT _processed AND _args_MODULE)
    if(NOT NumPy_FOUND)
        message(
          FATAL_ERROR
          "NumPy is required to process *.pyf F2PY files"
        )
    endif()
    set(_sources_abs )
    foreach(_source IN LISTS _sources)
      list(APPEND _sources_abs ${CMAKE_CURRENT_SOURCE_DIR}/${_source})
    endforeach()
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_name}.pyf
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${F2PY_EXECUTABLE}
        ARGS -h ${_name}.pyf -m ${_name} --overwrite-signature
             ${_sources_abs}
        DEPENDS ${_sources} ${_args_DEPENDS}
        COMMENT "Generating ${_name} Fortran interface file"
    )
    list(APPEND _sources ${_name}.pyf)
  endif()

  # Are there F2PY targets?
  set(_has_f2py_targets OFF)
  set(_has_cython_targets OFF)

  # Generate targets for all *.pyx and *.pyf files
  set(_processed )
  foreach(_source IN LISTS _sources)
    if(${_source} MATCHES \\.pyx$)
      if(NOT Cython_FOUND)
        message(
          FATAL_ERROR
          "Cython is required to process *.pyx Cython files"
        )
      endif()
      string(REGEX REPLACE "\\.[^.]*$" "" _pyx_target_name ${_source})
      set(_has_cython_targets ON)
      add_cython_target(${_pyx_target_name}
          ${_source}
          OUTPUT_VAR _pyx_target_output
          DEPENDS ${_args_DEPENDS}
      )
      list(APPEND _processed ${_pyx_target_output})
    elseif(${_source} MATCHES \\.pyf$)
      if(NOT NumPy_FOUND)
          message(
            FATAL_ERROR
            "NumPy is required to process *.pyf F2PY files"
          )
      endif()
      string(REGEX REPLACE "\\.[^.]*$" "" _pyf_target_name ${_source})
      set(_has_f2py_targets ON)
      add_f2py_target(${_pyf_target_name}
          ${_source}
          OUTPUT_VAR _pyf_target_output
          DEPENDS ${_args_DEPENDS}
      )
      list(APPEND _processed  ${_pyf_target_output})
    else()
      list(APPEND _processed ${_source})
    endif()
  endforeach()
  set(_sources ${_processed})

  if(_args_SHARED)
    add_library(${_name} SHARED ${_sources})
  elseif(_args_MODULE)
    add_library(${_name} MODULE ${_sources})
  else()
    # Assume static
    add_library(${_name} STATIC ${_sources})
  endif()

  target_include_directories(${_name} PRIVATE ${_args_INCLUDE_DIRECTORIES})
  target_link_libraries(${_name} ${SKBUILD_LINK_LIBRARIES_KEYWORD} ${_args_LINK_LIBRARIES})

  if(_has_f2py_targets)
    target_include_directories(${_name} PRIVATE ${F2PY_INCLUDE_DIRS})
    target_link_libraries(${_name} ${SKBUILD_LINK_LIBRARIES_KEYWORD} ${F2PY_LIBRARIES})
  endif()

  if(_args_COMPILE_DEFINITIONS)
    target_compile_definitions(${_name} PRIVATE ${_args_COMPILE_DEFINITIONS})
  endif()

  if(_args_DEPENDS)
    add_custom_target(
      "${_name}_depends"
      DEPENDS ${_args_DEPENDS}
    )
    add_dependencies(${_name} "${_name}_depends")
  endif()
endfunction()

function(add_python_extension _name)
  # FIXME: make sure that extensions with the same name can happen
  # in multiple directories

  set(multiValueArgs SOURCES INCLUDE_DIRECTORIES LINK_LIBRARIES COMPILE_DEFINITIONS DEPENDS)
  cmake_parse_arguments(_args "" "" "${multiValueArgs}" ${ARGN} )

  # Validate arguments to allow simpler debugging
  if(NOT _args_SOURCES)
    message(
      FATAL_ERROR
      "You have called add_python_extension for library ${_name} without "
      "any source files. This typically indicates a problem with "
      "your CMakeLists.txt file"
    )
  endif()

  add_python_library(${_name} MODULE
    SOURCES ${_args_SOURCES}
    INCLUDE_DIRECTORIES ${_args_INCLUDE_DIRECTORIES}
    LINK_LIBRARIES ${_args_LINK_LIBRARIES}
    COMPILE_DEFINITIONS ${_args_COMPILE_DEFINITIONS}
    DEPENDS ${_args_DEPENDS}
  )
  python_extension_module(${_name})

  file(RELATIVE_PATH _relative "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  install(
    TARGETS ${_name}
    LIBRARY DESTINATION "${_relative}"
    RUNTIME DESTINATION "${_relative}"
  )
endfunction()
