#include "system.hpp"

int main()
{
	system_t *system = new system_t();
	system->run();
	delete system;

    return 0;
}
