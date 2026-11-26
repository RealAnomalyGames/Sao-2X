#include "sao/platform.hpp"
#include "sao/platform_info.hpp"
#include "sao/filesystem.hpp"
#include "sao/memory/memory_manager.hpp"
#include "sao/resources/resource.hpp"
#include "sao/resources/resource_manager.hpp"
#include "sao/rendering/math3d.hpp"
#include "sao/rendering/mesh.hpp"
#include "sao/rendering/camera3d.hpp"
#include "sao/rendering/graphics_device.hpp"
#include "sao/rendering/texture_sampler.hpp"
#include "sao/rendering/render_state.hpp"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <filesystem>
#include <iostream>

class TestResource : public sao::Resource {
public:
    explicit TestResource(int resource_value)
        : value(resource_value) {
    }

    int value;
};


class MockGraphicsBackend final : public sao::GraphicsBackend {
public:
    const char* name() const noexcept override {
        return "MockGraphicsBackend";
    }

    bool initialize() override {
        initialized = true;
        return true;
    }

    void shutdown() override {
        initialized = false;
        shutdown_called = true;
    }

    bool compile_shader(
        sao::ShaderStage stage,
        const std::string& source,
        std::uint32_t& shader_handle,
        std::string& log
    ) override {
        (void)stage;

        if (source.empty()) {
            log = "Shader source is empty.";
            return false;
        }

        shader_handle = next_handle++;
        return true;
    }

    void destroy_shader(std::uint32_t handle) override {
        destroyed_shaders.push_back(handle);
    }

    bool create_program(
        std::uint32_t& program_handle,
        std::string& log
    ) override {
        (void)log;

        program_handle = next_handle++;
        return true;
    }

    bool attach_shader(
        std::uint32_t program_handle,
        std::uint32_t shader_handle,
        std::string& log
    ) override {
        (void)program_handle;
        (void)log;

        attached_shaders.push_back(shader_handle);
        return true;
    }

    bool link_program(
        std::uint32_t program_handle,
        std::string& log
    ) override {
        (void)log;

        return program_handle != 0;
    }

    void destroy_program(std::uint32_t handle) override {
        destroyed_programs.push_back(handle);
    }

    void use_program(std::uint32_t handle) override {
        active_program = handle;
    }

    void apply_render_state(
        const sao::RenderState& state
    ) override {
        current_render_state = state;
        render_state_applied = true;
    }

    void apply_texture_sampler(
        unsigned int texture_unit,
        const sao::TextureSampler& sampler
    ) override {
        last_texture_unit = texture_unit;
        current_sampler = sampler;
        sampler_applied = true;
    }

    bool initialized = false;
    bool shutdown_called = false;
    bool render_state_applied = false;
    bool sampler_applied = false;

    std::uint32_t next_handle = 1;
    std::uint32_t active_program = 0;

    unsigned int last_texture_unit = 0;

    sao::RenderState current_render_state;
    sao::TextureSampler current_sampler;

    std::vector<std::uint32_t> destroyed_shaders;
    std::vector<std::uint32_t> destroyed_programs;
    std::vector<std::uint32_t> attached_shaders;
};

void test_resource_manager() {
    std::cout << "[TEST] Resource Manager\n";
    
    int load_count = 0;

    sao::ResourceManager<TestResource> manager(
        [&load_count](const sao::AssetReference& asset) {
            ++load_count;

            if (asset.path() == "assets/test.asset") {
                return std::make_shared<TestResource>(42);
            }

            return std::shared_ptr<TestResource>();
        }
    );

    const sao::AssetReference asset("assets/test.asset");

    const auto first = manager.load(asset);

    if (!first.is_valid()) {
        throw std::runtime_error(
            "Resource manager returned an invalid handle."
        );
    }

    if (!manager.is_loaded(first)) {
        throw std::runtime_error(
            "Resource was not marked as loaded."
        );
    }

    auto resource = manager.get(first);

    if (!resource || resource->value != 42) {
        throw std::runtime_error(
            "Resource retrieval failed."
        );
    }

    const auto second = manager.load(asset);

    if (first != second || load_count != 1) {
        throw std::runtime_error(
            "Resource caching failed."
        );
    }

    if (manager.loaded_count() != 1) {
        throw std::runtime_error(
            "Resource count is incorrect."
        );
    }

    if (!manager.unload(first)) {
        throw std::runtime_error(
            "Resource unloading failed."
        );
    }

    if (manager.get(first)) {
        throw std::runtime_error(
            "An unloaded resource handle remained valid."
        );
    }

    if (manager.loaded_count() != 0) {
        throw std::runtime_error(
            "Resource count was not updated after unloading."
        );
    }

    const auto third = manager.load(asset);

    if (!third.is_valid() || third == first) {
        throw std::runtime_error(
            "Resource handle generation tracking failed."
        );
    }

    if (load_count != 2) {
        throw std::runtime_error(
            "Resource was not loaded again after unloading."
        );
    }

    manager.unload_all();

    if (manager.loaded_count() != 0 ||
        manager.get(third)) {
        throw std::runtime_error(
            "ResourceManager::unload_all() failed."
        );
    }
}

void test_memory_manager() {
    std::cout << "[TEST] Memory Manager\n";
    
    sao::MemoryManager memory;

    void* first = memory.allocate(256, 16, "First test allocation");
    void* second = memory.allocate(512, 32, "Second test allocation");

    if (!memory.owns(first) || !memory.owns(second)) {
        throw std::runtime_error(
            "Memory manager ownership tracking failed."
        );
    }

    sao::MemoryStats stats = memory.get_stats();

    if (stats.current_bytes != 768 ||
        stats.active_allocations != 2 ||
        stats.allocation_count != 2) {
        throw std::runtime_error(
            "Memory allocation statistics are incorrect."
        );
    }

    if (!memory.deallocate(first)) {
        throw std::runtime_error(
            "Memory deallocation failed."
        );
    }

    stats = memory.get_stats();

    if (stats.current_bytes != 512 ||
        stats.active_allocations != 1) {
        throw std::runtime_error(
            "Memory statistics were not updated after deallocation."
        );
    }

    if (memory.deallocate(first)) {
        throw std::runtime_error(
            "Memory manager accepted a duplicate deallocation."
        );
    }

    if (!memory.deallocate(second)) {
        throw std::runtime_error(
            "Second memory deallocation failed."
        );
    }

    stats = memory.get_stats();

    if (stats.current_bytes != 0 ||
        stats.active_allocations != 0) {
        throw std::runtime_error(
            "Memory manager did not release all test allocations."
        );
    }

    if (stats.peak_bytes != 768) {
        throw std::runtime_error(
            "Memory peak allocation tracking failed."
        );
    }
}

void test_platform_detection() {
    std::cout << "[TEST] Platform Detection\n";
    
    const sao::PlatformInfo info =
        sao::PlatformDetector::detect();

    if (info.type == sao::PlatformType::Unknown) {
        throw std::runtime_error(
            "Platform detection returned Unknown."
        );
    }

    if (info.name.empty()) {
        throw std::runtime_error(
            "Platform name is empty."
        );
    }

    if (info.architecture_name.empty()) {
        throw std::runtime_error(
            "Architecture name is empty."
        );
    }

    std::cout << "Detected platform: "
              << info.name << " "
              << info.version << '\n';

    std::cout << "Architecture: "
              << info.architecture_name << '\n';
}

void test_platform_lifecycle() {
    std::cout << "[TEST] Platform Lifecycle\n";
    
    sao::Platform platform;

    if (platform.is_initialized()) {
        throw std::runtime_error(
            "Platform should not initially be initialized."
        );
    }

    platform.initialize();

    if (!platform.is_initialized()) {
        throw std::runtime_error(
            "Platform initialization failed."
        );
    }

    if (platform.info().type == sao::PlatformType::Unknown) {
        throw std::runtime_error(
            "Platform information is invalid."
        );
    }

    platform.shutdown();

    if (platform.is_initialized()) {
        throw std::runtime_error(
            "Platform shutdown failed."
        );
    }
}

void test_filesystem() {
    std::cout << "[TEST] Filesystem\n";
    
    const auto test_directory =
        sao::FileSystem::current_directory() / "sao2x_test";

    const auto test_file =
        test_directory / "test.txt";

    // Remove leftovers from previous runs.
    if (sao::FileSystem::exists(test_directory)) {
        if (!sao::FileSystem::remove_directory(test_directory)) {
            throw std::runtime_error(
                "Could not remove previous filesystem test directory."
            );
        }
    }

    // Create the directory.
    const bool created =
        sao::FileSystem::create_directories(test_directory);

    if (!created || !sao::FileSystem::is_directory(test_directory)) {
        throw std::runtime_error(
            "Filesystem test directory could not be created: " +
            test_directory.string()
        );
    }

    // Write a file.
    sao::FileSystem::write_text(
        test_file,
        "Sao-2X platform test"
    );

    if (!sao::FileSystem::is_file(test_file)) {
        throw std::runtime_error(
            "Filesystem test file was not created."
        );
    }

    // Read the file.
    if (sao::FileSystem::read_text(test_file) !=
        "Sao-2X platform test") {
        throw std::runtime_error(
            "Filesystem text reading test failed."
        );
    }

    // Append text.
    sao::FileSystem::append_text(test_file, " passed");

    if (sao::FileSystem::read_text(test_file) !=
        "Sao-2X platform test passed") {
        throw std::runtime_error(
            "Filesystem append test failed."
        );
    }

    // Remove the file.
    if (!sao::FileSystem::remove_file(test_file)) {
        throw std::runtime_error(
            "Could not remove filesystem test file."
        );
    }

    // Remove the directory.
    if (!sao::FileSystem::remove_directory(test_directory)) {
        throw std::runtime_error(
            "Could not remove filesystem test directory."
        );
    }
}

void test_3d_rendering_foundation() {
    std::cout << "[TEST] 3D Rendering Foundation\n";

    using namespace sao;

    const Vector3 a{1.0f, 2.0f, 3.0f};
    const Vector3 b{2.0f, 3.0f, 4.0f};

    const Vector3 sum = a + b;

    if (sum.x != 3.0f || sum.y != 5.0f || sum.z != 7.0f) {
        throw std::runtime_error("3D vector addition failed.");
    }

    const Vector3 cross = Vector3::cross(
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}
    );

    if (cross.x != 0.0f || cross.y != 0.0f || cross.z != 1.0f) {
        throw std::runtime_error("3D vector cross product failed.");
    }

    const Matrix4 identity = Matrix4::identity();
    const Vector3 transformed =
        identity.transform_point({2.0f, 3.0f, 4.0f});

    if (transformed.x != 2.0f ||
        transformed.y != 3.0f ||
        transformed.z != 4.0f) {
        throw std::runtime_error("3D identity matrix failed.");
    }

    const Mesh cube = Mesh::create_cube();

    if (cube.vertices.size() != 24 || cube.indices.size() != 36) {
        throw std::runtime_error("Cube mesh generation failed.");
    }

    const Mesh plane = Mesh::create_plane();

    if (plane.vertices.size() != 4 || plane.indices.size() != 6) {
        throw std::runtime_error("Plane mesh generation failed.");
    }

    const Camera3D camera;
    const Matrix4 view = camera.view_matrix();
    const Matrix4 projection = camera.projection_matrix(16.0f / 9.0f);

    (void)view;
    (void)projection;
}


void test_graphics_system() {
    // Test texture filtering configuration.
    sao::TextureSampler sampler;

    if (sampler.min_filter != sao::TextureFilter::Linear ||
        sampler.mag_filter != sao::TextureFilter::Linear) {
        throw std::runtime_error(
            "Default texture filtering is incorrect."
        );
    }

    sampler.min_filter = sao::TextureFilter::Nearest;
    sampler.mag_filter = sao::TextureFilter::Nearest;
    sampler.wrap_u = sao::TextureWrap::ClampToEdge;
    sampler.wrap_v = sao::TextureWrap::ClampToEdge;

    // Test render states.
    sao::RenderState state;

    state.depth_test_enabled = true;
    state.depth_write_enabled = true;
    state.face_culling_enabled = true;
    state.cull_mode = sao::CullMode::Back;
    state.blending.enabled = true;

    sao::RenderState copied_state = state;

    if (!(state == copied_state)) {
        throw std::runtime_error(
            "Render state equality test failed."
        );
    }

    copied_state.wireframe = true;

    if (state == copied_state) {
        throw std::runtime_error(
            "Render state comparison failed."
        );
    }

    // Test graphics device initialization.
    auto backend = std::make_unique<MockGraphicsBackend>();
    MockGraphicsBackend* backend_ptr = backend.get();

    sao::GraphicsDevice device;

    if (!device.initialize(std::move(backend))) {
        throw std::runtime_error(
            "Graphics device initialization failed."
        );
    }

    if (!device.is_initialized()) {
        throw std::runtime_error(
            "Graphics device reports incorrect initialization state."
        );
    }

    if (std::string(device.backend_name()) != "MockGraphicsBackend") {
        throw std::runtime_error(
            "Graphics backend name is incorrect."
        );
    }

    // Test shader compilation.
    sao::Shader vertex_shader(
        sao::ShaderStage::Vertex,
        "void main() {}"
    );

    sao::Shader fragment_shader(
        sao::ShaderStage::Fragment,
        "void main() {}"
    );

    if (!device.compile_shader(vertex_shader) ||
        !device.compile_shader(fragment_shader)) {
        throw std::runtime_error(
            "Shader compilation test failed."
        );
    }

    if (!vertex_shader.is_compiled() ||
        !fragment_shader.is_compiled()) {
        throw std::runtime_error(
            "Shader compilation state is incorrect."
        );
    }

    // Test shader program linking.
    sao::ShaderProgram program;

    if (!device.link_program(
            program,
            {&vertex_shader, &fragment_shader})) {
        throw std::runtime_error(
            "Shader program linking test failed."
        );
    }

    if (!program.is_linked()) {
        throw std::runtime_error(
            "Shader program reports incorrect linking state."
        );
    }

    if (backend_ptr->attached_shaders.size() != 2) {
        throw std::runtime_error(
            "Shader attachment count is incorrect."
        );
    }

    // Test program activation.
    device.use_program(&program);

    if (backend_ptr->active_program != program.handle()) {
        throw std::runtime_error(
            "Shader program activation failed."
        );
    }

    // Test render state submission.
    device.set_render_state(state);

    if (!backend_ptr->render_state_applied ||
        !(backend_ptr->current_render_state == state)) {
        throw std::runtime_error(
            "Render state submission failed."
        );
    }

    // Test texture sampler submission.
    device.set_texture_sampler(2, sampler);

    if (!backend_ptr->sampler_applied ||
        backend_ptr->last_texture_unit != 2 ||
        !(backend_ptr->current_sampler == sampler)) {
        throw std::runtime_error(
            "Texture sampler submission failed."
        );
    }

    // Test resource destruction.
    const std::uint32_t vertex_handle = vertex_shader.handle();
    const std::uint32_t fragment_handle = fragment_shader.handle();
    const std::uint32_t program_handle = program.handle();

    device.destroy_shader(vertex_shader);
    device.destroy_shader(fragment_shader);
    device.destroy_program(program);

    if (vertex_shader.is_compiled() ||
        fragment_shader.is_compiled() ||
        program.is_linked()) {
        throw std::runtime_error(
            "Graphics resource destruction failed."
        );
    }

    if (backend_ptr->destroyed_shaders.size() != 2 ||
        backend_ptr->destroyed_programs.size() != 1) {
        throw std::runtime_error(
            "Graphics resource destruction count is incorrect."
        );
    }

    if (backend_ptr->destroyed_shaders[0] != vertex_handle ||
        backend_ptr->destroyed_shaders[1] != fragment_handle ||
        backend_ptr->destroyed_programs[0] != program_handle) {
        throw std::runtime_error(
            "Incorrect graphics resource handles were destroyed."
        );
    }

    // Test backend shutdown.
    device.shutdown();

    if (device.is_initialized() || !backend_ptr->shutdown_called) {
        throw std::runtime_error(
            "Graphics device shutdown failed."
        );
    }
}

int main() {
    try {
        test_resource_manager();
        test_memory_manager();
        test_platform_detection();
        test_platform_lifecycle();
        test_filesystem();
        test_3d_rendering_foundation();
        test_graphics_system();

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "TEST FAILED: " << error.what() << '\n';
        return 1;
    }
}
