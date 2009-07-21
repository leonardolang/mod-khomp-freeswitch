#include <function.hpp>

#include <iostream>

typedef Function::Function1 < bool, int > TesteFun;

struct Teste123
{
    bool operator()(int i)
    {
        std::cout << i << std::endl;
        return true;
    };
};

int main ()
{
    Teste123   t123;
    TesteFun t(t123);

    t(2);

    return 0;
};
