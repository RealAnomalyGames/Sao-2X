#include <cassert>
#include <cstring>

#include "sao/engine.hpp"

int main() {
    const char* engine_version = sao::version();

    assert(engine_version != nullptr);
    assert(std::strlen(engine_version) > 0);

    assert(std::strcmp(engine_version, SAO2X_VERSION) == 0);

    return 0;
}
