#include "sao/engine.hpp"

#ifndef SAO2X_VERSION
#define SAO2X_VERSION "0.0.0"
#endif

namespace sao {

const char* version() noexcept {
    return SAO2X_VERSION;
}

}
