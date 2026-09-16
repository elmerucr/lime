add_library(system STATIC
	analog.cpp
	core.cpp
	cpu.cpp
	debugger.cpp
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

target_link_libraries(system moira resid)

add_subdirectory(Moira/)
add_subdirectory(resid-0.16/)
