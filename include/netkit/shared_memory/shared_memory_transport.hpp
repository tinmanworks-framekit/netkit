#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace netkit::shared_memory {

class SharedMemoryTransport {
public:
    explicit SharedMemoryTransport(std::size_t capacity_bytes) : capacity_bytes_(capacity_bytes) {}

    bool Open(const char* name);
    bool Write(const std::vector<std::uint8_t>& payload);
    std::vector<std::uint8_t> Read();
    void Close();

    bool IsOpen() const { return open_; }
    std::size_t CapacityBytes() const { return capacity_bytes_; }

private:
    std::size_t capacity_bytes_;
    bool open_ = false;
    std::vector<std::uint8_t> in_memory_buffer_;
};

} // namespace netkit::shared_memory
