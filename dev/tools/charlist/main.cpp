#include <iostream>

int main(/*int argc, char** argv*/)
{
    constexpr char first {32};
    constexpr char last {126};
    constexpr unsigned columns {8};

    std::cout << "chars" << std::endl;

    for (char i {first}; i > 0 && i <= last; ++i) {
        std::cout << i;
        if (i % columns == 0) { std::cout << std::endl; }
    }

    std::cout << std::endl;

    return 0;
}
