#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/gl/types.hpp>

namespace mbgl {
namespace gl {

class UniformBufferGL final : public gfx::UniformBuffer {
    UniformBufferGL(const UniformBufferGL&);

public:
    UniformBufferGL(const void* data, std::size_t size_);
    UniformBufferGL(UniformBufferGL&& other)
        : UniformBuffer(std::move(other)) {}
    ~UniformBufferGL() override;

    BufferID getID() const { return id; }
    void update(const void* data, std::size_t size_) override;

    UniformBufferGL clone() const { return {*this}; }

protected:
    BufferID id = 0;
    uint32_t hash;
};

/// Stores a collection of uniform buffers by name
class UniformBufferArrayGL final : public gfx::UniformBufferArray {
public:
    UniformBufferArrayGL() = default;
    UniformBufferArrayGL(UniformBufferArrayGL&& other)
        : UniformBufferArray(std::move(other)) {}
    UniformBufferArrayGL(const UniformBufferArrayGL&) = delete;

    UniformBufferArrayGL& operator=(UniformBufferArrayGL&& other) {
        UniformBufferArray::operator=(std::move(other));
        return *this;
    }
    UniformBufferArrayGL& operator=(const UniformBufferArrayGL& other) {
        UniformBufferArray::operator=(other);
        return *this;
    }

private:
    std::unique_ptr<gfx::UniformBuffer> copy(const gfx::UniformBuffer& uniformBuffers) override {
        return std::make_unique<UniformBufferGL>(static_cast<const UniformBufferGL&>(uniformBuffers).clone());
    }
};

} // namespace gl
} // namespace mbgl
