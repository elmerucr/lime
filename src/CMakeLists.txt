add_library(system STATIC
	core.cpp
	cpu.cpp
	debugger.cpp
	host.cpp
	keyboard.cpp
	stats.cpp
	system.cpp
	terminal.cpp
	vdc.cpp
)

target_link_libraries(system MC6809)

add_subdirectory(MC6809/)
