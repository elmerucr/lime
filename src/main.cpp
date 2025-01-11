// ---------------------------------------------------------------------
// main.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "system.hpp"

int main()
{
	system_t *system = new system_t();
	system->run();
	delete system;

    return 0;
}
