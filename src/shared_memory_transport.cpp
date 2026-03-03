#include "netkit/shared_memory/shared_memory_transport.hpp"

namespace netkit::shared_memory {

bool SharedMemoryTransport::Open(const char* name) {
    (void)name;
    open_ = true;
    return true;
}

bool SharedMemoryTransport::Write(const std::vector<std::uint8_t>& payload) {
    if (!open_ || payload.size() > capacity_bytes_) {
        return false;
    }
    in_memory_buffer_ = payload;
    return true;
}

std::vector<std::uint8_t> SharedMemoryTransport::Read() {
    if (!open_) {
        return {};
    }
    return in_memory_buffer_;
}

void SharedMemoryTransport::Close() {
    open_ = false;
    in_memory_buffer_.clear();
}

} // namespace netkit::shared_memory
