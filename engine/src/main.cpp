#include <iostream>

#include "sao/engine.hpp"

int main() {
    std::cout << "Sao-2X Game Engine\n";
    std::cout << "Neptune Corporation\n";
    std::cout << "Version " << sao::version() << "\n";

    std::cout << "\nEngine initialized successfully.\n";

    return 0;
}
