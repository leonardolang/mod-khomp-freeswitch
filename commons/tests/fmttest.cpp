#include <format.hpp>
#include <iostream>

typedef enum {
    TYPE0,
    TYPE1,
    TYPE2,
    TYPE3
} type;

int main ()
{
    type t = TYPE1;

	Format fmt("string: %s valor: %d");

	std::cout << STG((fmt % ("test") % t)) << std::endl;

}

