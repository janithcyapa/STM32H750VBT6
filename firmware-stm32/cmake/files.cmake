# Add sources to executable/library
target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/core/syscall.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/core/sysmem.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/core/startup_stm32h750xx.S"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/main.c"
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/stm32h750xb_flash.ld" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/stm32h750xb_flash.ld")
