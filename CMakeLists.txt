CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

# Configuration per yaul projects
SET(IP_VERSION "V0.001")
SET(IP_RELEASE_DATE "20210301")
SET(IP_AREAS "JTUBKAEL")
SET(IP_PERIPHERALS "JAMKST")
SET(IP_TITLE "VGM Player")
SET(IP_MASTER_STACK_ADDR "0x06100000")
SET(IP_SLAVE_STACK_ADDR "0x06002000")
SET(IP_1ST_READ_ADDR "0x06004000")
SET(IP_1ST_READ_SIZE "0")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/yaul.cmake)

# project...
PROJECT(VGMPlayer)

set(VGM_SRC 
	src/dbg.c 
	src/main.c 
	src/menu/menu.c
	src/vgm.c
)

add_executable(${PROJECT_NAME} ${VGM_SRC})

target_link_libraries(${PROJECT_NAME} ${YAUL_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE src)

set(LOADER_IP_BIN	${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)

gen_map(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map)
gen_start_bin(${PROJECT_NAME} ${LOADER_IP_BIN})
gen_symbol_size(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.size.txt)
gen_iso(${PROJECT_NAME} ${LOADER_IP_BIN} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.iso)