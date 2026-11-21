
#include <cassert>
#include <cstring>

#include "sao/application.hpp"
#include "sao/config.hpp"
#include "sao/engine.hpp"
#include "sao/error.hpp"

class TestApplication final : public sao::Application {
public:
    int initialized = 0;
    int updated = 0;
    int shutdown = 0;

    void onInitialize() override {
        ++initialized;
    }

    void onUpdate(float delta_time) override {
        assert(delta_time >= 0.0f);

        ++updated;

        if (updated >= 3) {
            requestStop();
        }
    }

    void onShutdown() override {
        ++shutdown;
    }
};

int main() {
    // Version test

    assert(
        std::strcmp(sao::Engine::version(), SAO2X_VERSION) == 0
    );

    // Configuration test

    sao::EngineConfig config;

    config.application_name = "Sao-2X Test";
    config.enable_console_logging = false;
    config.enable_file_logging = false;
    config.target_fps = 1000;

    config.validate();

    // Invalid configuration test

    bool invalid_config_detected = false;

    try {
        sao::EngineConfig invalid;
        invalid.target_fps = 0;
        invalid.validate();

    } catch (const sao::EngineError& error) {
        invalid_config_detected =
            error.code() == sao::ErrorCode::InvalidConfiguration;
    }

    assert(invalid_config_detected);

    // Engine initialization and shutdown test

    sao::Engine engine;

    assert(engine.state() == sao::EngineState::Created);

    engine.initialize(config);

    assert(engine.is_initialized());

    engine.shutdown();

    assert(engine.state() == sao::EngineState::Shutdown);

    // Application lifecycle test

    TestApplication application;

    const int result = application.run(config);

    assert(result == 0);

    assert(application.initialized == 1);
    assert(application.updated == 3);
    assert(application.shutdown == 1);

    assert(
        application.state() == sao::ApplicationState::Stopped
    );

    return 0;
}
