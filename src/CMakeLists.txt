add_library(system STATIC
	analog.cpp
	core.cpp
	cpu_mc6809.cpp
	cpu_mc68000.cpp
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
	sn74ls148.cpp
	vdc.cpp
)

target_link_libraries(system mc6809 moira resid)

add_subdirectory(mc6809/)
add_subdirectory(Moira/)
add_subdirectory(resid-0.16/)
