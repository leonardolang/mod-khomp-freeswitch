#include <format.hpp>
#include <iostream>

int main ()
{
	Format fmt("teste: %s");

	std::cout << STG((fmt % "teste")) << std::endl;
}
