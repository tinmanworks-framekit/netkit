#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace netkit::shared_memory {

enum class SharedMemoryBackendKind : std::uint8_t {
    kLinuxPosix = 0,
    kMacPosix = 1,
    kWindowsFileMapping = 2,
    kInMemoryFallback = 255,
};

class SharedMemoryTransport {
public:
    explicit SharedMemoryTransport(std::size_t capacity_bytes);

    bool Open(const char* name);
    bool Write(const std::vector<std::uint8_t>& payload);
    std::vector<std::uint8_t> Read();
    void Close();

    bool IsOpen() const { return open_; }
    std::size_t CapacityBytes() const { return capacity_bytes_; }
    SharedMemoryBackendKind BackendKind() const { return backend_kind_; }

private:
    static SharedMemoryBackendKind DetectBackendKind();

    std::size_t capacity_bytes_;
    SharedMemoryBackendKind backend_kind_ = SharedMemoryBackendKind::kInMemoryFallback;
    bool open_ = false;
    std::string mapped_name_;
    std::vector<std::uint8_t> in_memory_buffer_;

#if defined(__linux__)
    int shm_fd_ = -1;
    std::uint8_t* mapped_data_ = nullptr;
    std::size_t mapped_size_ = 0;
#endif
};

} // namespace netkit::shared_memory
