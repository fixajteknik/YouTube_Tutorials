# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1.1/components/bootloader/subproject"
  "C:/deneme/adc_test/sample_project/build/bootloader"
  "C:/deneme/adc_test/sample_project/build/bootloader-prefix"
  "C:/deneme/adc_test/sample_project/build/bootloader-prefix/tmp"
  "C:/deneme/adc_test/sample_project/build/bootloader-prefix/src/bootloader-stamp"
  "C:/deneme/adc_test/sample_project/build/bootloader-prefix/src"
  "C:/deneme/adc_test/sample_project/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/deneme/adc_test/sample_project/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/deneme/adc_test/sample_project/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
