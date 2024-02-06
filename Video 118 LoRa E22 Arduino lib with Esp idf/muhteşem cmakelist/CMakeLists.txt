get_filename_component(COMPONENT_CMAKELIST_DIR ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
if (IS_DIRECTORY "${COMPONENT_CMAKELIST_DIR}/src") 
message("Kütüphane içinde header dosyları SRC klasöründe: ${COMPONENT_CMAKELIST_DIR}")

#src klasörünün içindeki header file ları getir.
set(COMPONENT_SRCDIRS
"src"
)
 
set(COMPONENT_ADD_INCLUDEDIRS
"src"
)

else()
message("SRC klasörü Yok header dosyları ana dizinde")

set(COMPONENT_SRCDIRS
"."
)
 
set(COMPONENT_ADD_INCLUDEDIRS
"."
)

endif()
 
set(COMPONENT_REQUIRES
"arduino"
)
 
register_component()
 
target_compile_definitions(${COMPONENT_TARGET} PUBLIC -DESP32)
target_compile_options(${COMPONENT_TARGET} PRIVATE -fno-rtti)