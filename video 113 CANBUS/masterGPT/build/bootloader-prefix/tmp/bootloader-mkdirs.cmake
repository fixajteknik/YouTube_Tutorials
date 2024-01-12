# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/ESP/vscodeidf/esp-idf/components/bootloader/subproject"
  "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader"
  "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader-prefix"
  "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader-prefix/tmp"
  "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader-prefix/src/bootloader-stamp"
  "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader-prefix/src"
  "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/ESP/vscodeexample/cangptMaster/masterGPT/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
