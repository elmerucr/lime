add_library(system STATIC
	analog.cpp
	core.cpp
	cpu.cpp
	debugger.cpp
	exceptions.cpp
	host.cpp
	keyboard.cpp
	osd.cpp
	sound.cpp
	stats.cpp
	system.cpp
	terminal.cpp
	timer.cpp
	vdc.cpp
)

target_link_libraries(system MC6809 resid)

add_subdirectory(MC6809/)
add_subdirectory(resid-0.16/)
