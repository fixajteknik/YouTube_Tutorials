# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/vscode/v5.2.1/esp-idf/components/ulp/cmake"
  "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main"
  "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main-prefix"
  "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main-prefix/tmp"
  "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp"
  "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main-prefix/src"
  "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/ESP/vscodeexample/ulp/ulp11/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp${cfgdir}") # cfgdir has leading slash
endif()
