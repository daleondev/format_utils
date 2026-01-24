# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-src")
  file(MAKE_DIRECTORY "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-build"
  "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-subbuild/reflect-populate-prefix"
  "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-subbuild/reflect-populate-prefix/tmp"
  "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-subbuild/reflect-populate-prefix/src/reflect-populate-stamp"
  "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-subbuild/reflect-populate-prefix/src"
  "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-subbuild/reflect-populate-prefix/src/reflect-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-subbuild/reflect-populate-prefix/src/reflect-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Dev.Windows-Desktop/Desktop/FormatUtils/build/Debug/_deps/reflect-subbuild/reflect-populate-prefix/src/reflect-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
