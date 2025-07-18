add_library(system STATIC
	analog.cpp
	core.cpp
	cpu_m68k.cpp
	cpu_mc6809.cpp
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
	TTL74LS148.cpp
	vdc.cpp
)

target_link_libraries(system MC6809 moira resid)

add_subdirectory(MC6809/)
add_subdirectory(Moira/)
add_subdirectory(resid-0.16/)
