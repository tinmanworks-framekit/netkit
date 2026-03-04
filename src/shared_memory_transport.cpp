#include "netkit/shared_memory/shared_memory_transport.hpp"

#include <algorithm>
#include <cstring>

#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

namespace netkit::shared_memory {

namespace {

constexpr std::size_t kHeaderSize = sizeof(std::uint32_t);

std::string NormalizeShmName(const char* name) {
    if (name == nullptr || *name == '\0') {
        return "/netkit-default";
    }
    if (name[0] == '/') {
        return std::string{name};
    }
    return "/" + std::string{name};
}

} // namespace

SharedMemoryTransport::SharedMemoryTransport(std::size_t capacity_bytes)
    : capacity_bytes_(capacity_bytes), backend_kind_(DetectBackendKind()) {}

SharedMemoryBackendKind SharedMemoryTransport::DetectBackendKind() {
#if defined(__linux__)
    return SharedMemoryBackendKind::kLinuxPosix;
#elif defined(__APPLE__)
    return SharedMemoryBackendKind::kMacPosix;
#elif defined(_WIN32)
    return SharedMemoryBackendKind::kWindowsFileMapping;
#else
    return SharedMemoryBackendKind::kInMemoryFallback;
#endif
}

bool SharedMemoryTransport::Open(const char* name) {
#if defined(__linux__) || defined(__APPLE__)
    if (backend_kind_ == SharedMemoryBackendKind::kLinuxPosix ||
        backend_kind_ == SharedMemoryBackendKind::kMacPosix) {
        mapped_name_ = NormalizeShmName(name);
        mapped_size_ = capacity_bytes_ + kHeaderSize;
        shm_fd_ = shm_open(mapped_name_.c_str(), O_CREAT | O_RDWR, 0600);
        if (shm_fd_ < 0) {
            backend_kind_ = SharedMemoryBackendKind::kInMemoryFallback;
            mapped_name_.clear();
            open_ = true;
            return true;
        }

        if (ftruncate(shm_fd_, static_cast<off_t>(mapped_size_)) != 0) {
            close(shm_fd_);
            shm_fd_ = -1;
            backend_kind_ = SharedMemoryBackendKind::kInMemoryFallback;
            mapped_name_.clear();
            mapped_size_ = 0;
            open_ = true;
            return true;
        }

        void* mapped = mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (mapped == MAP_FAILED) {
            close(shm_fd_);
            shm_fd_ = -1;
            backend_kind_ = SharedMemoryBackendKind::kInMemoryFallback;
            mapped_name_.clear();
            mapped_size_ = 0;
            open_ = true;
            return true;
        }
        mapped_data_ = static_cast<std::uint8_t*>(mapped);
        std::memset(mapped_data_, 0, mapped_size_);
        open_ = true;
        return true;
    }
#endif
#if defined(_WIN32)
    if (backend_kind_ == SharedMemoryBackendKind::kWindowsFileMapping) {
        mapped_name_ = (name == nullptr || *name == '\0') ? "netkit-default" : name;
        mapped_size_ = capacity_bytes_ + kHeaderSize;
        mapping_handle_ = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            0,
            static_cast<DWORD>(mapped_size_),
            mapped_name_.c_str());
        if (mapping_handle_ == nullptr) {
            backend_kind_ = SharedMemoryBackendKind::kInMemoryFallback;
            mapped_name_.clear();
            open_ = true;
            return true;
        }

        void* mapped = MapViewOfFile(mapping_handle_, FILE_MAP_ALL_ACCESS, 0, 0, mapped_size_);
        if (mapped == nullptr) {
            CloseHandle(static_cast<HANDLE>(mapping_handle_));
            mapping_handle_ = nullptr;
            backend_kind_ = SharedMemoryBackendKind::kInMemoryFallback;
            mapped_name_.clear();
            mapped_size_ = 0;
            open_ = true;
            return true;
        }
        mapped_data_ = static_cast<std::uint8_t*>(mapped);
        std::memset(mapped_data_, 0, mapped_size_);
        open_ = true;
        return true;
    }
#endif

    (void)name;
    open_ = true;
    return true;
}

bool SharedMemoryTransport::Write(const std::vector<std::uint8_t>& payload) {
    if (!open_ || payload.size() > capacity_bytes_) {
        return false;
    }

#if defined(__linux__) || defined(__APPLE__)
    if (backend_kind_ == SharedMemoryBackendKind::kLinuxPosix ||
        backend_kind_ == SharedMemoryBackendKind::kMacPosix) {
        if (mapped_data_ == nullptr || mapped_size_ < payload.size() + kHeaderSize) {
            return false;
        }
        const auto payload_size = static_cast<std::uint32_t>(payload.size());
        std::memcpy(mapped_data_, &payload_size, kHeaderSize);
        if (!payload.empty()) {
            std::memcpy(mapped_data_ + kHeaderSize, payload.data(), payload.size());
        }
        return true;
    }
#endif
#if defined(_WIN32)
    if (backend_kind_ == SharedMemoryBackendKind::kWindowsFileMapping) {
        if (mapped_data_ == nullptr || mapped_size_ < payload.size() + kHeaderSize) {
            return false;
        }
        const auto payload_size = static_cast<std::uint32_t>(payload.size());
        std::memcpy(mapped_data_, &payload_size, kHeaderSize);
        if (!payload.empty()) {
            std::memcpy(mapped_data_ + kHeaderSize, payload.data(), payload.size());
        }
        return true;
    }
#endif

    in_memory_buffer_ = payload;
    return true;
}

std::vector<std::uint8_t> SharedMemoryTransport::Read() {
    if (!open_) {
        return {};
    }

#if defined(__linux__) || defined(__APPLE__)
    if (backend_kind_ == SharedMemoryBackendKind::kLinuxPosix ||
        backend_kind_ == SharedMemoryBackendKind::kMacPosix) {
        if (mapped_data_ == nullptr || mapped_size_ < kHeaderSize) {
            return {};
        }

        std::uint32_t payload_size = 0;
        std::memcpy(&payload_size, mapped_data_, kHeaderSize);
        const auto max_payload = mapped_size_ - kHeaderSize;
        const auto size = std::min<std::size_t>(payload_size, max_payload);

        std::vector<std::uint8_t> output(size);
        if (size > 0) {
            std::memcpy(output.data(), mapped_data_ + kHeaderSize, size);
        }
        return output;
    }
#endif
#if defined(_WIN32)
    if (backend_kind_ == SharedMemoryBackendKind::kWindowsFileMapping) {
        if (mapped_data_ == nullptr || mapped_size_ < kHeaderSize) {
            return {};
        }

        std::uint32_t payload_size = 0;
        std::memcpy(&payload_size, mapped_data_, kHeaderSize);
        const auto max_payload = mapped_size_ - kHeaderSize;
        const auto size = std::min<std::size_t>(payload_size, max_payload);

        std::vector<std::uint8_t> output(size);
        if (size > 0) {
            std::memcpy(output.data(), mapped_data_ + kHeaderSize, size);
        }
        return output;
    }
#endif

    return in_memory_buffer_;
}

void SharedMemoryTransport::Close() {
#if defined(__linux__) || defined(__APPLE__)
    if (backend_kind_ == SharedMemoryBackendKind::kLinuxPosix ||
        backend_kind_ == SharedMemoryBackendKind::kMacPosix) {
        if (mapped_data_ != nullptr && mapped_size_ > 0) {
            munmap(mapped_data_, mapped_size_);
            mapped_data_ = nullptr;
        }
        if (shm_fd_ >= 0) {
            close(shm_fd_);
            shm_fd_ = -1;
        }
        if (!mapped_name_.empty()) {
            shm_unlink(mapped_name_.c_str());
            mapped_name_.clear();
        }
        mapped_size_ = 0;
    }
#endif
#if defined(_WIN32)
    if (backend_kind_ == SharedMemoryBackendKind::kWindowsFileMapping) {
        if (mapped_data_ != nullptr) {
            UnmapViewOfFile(mapped_data_);
            mapped_data_ = nullptr;
        }
        if (mapping_handle_ != nullptr) {
            CloseHandle(static_cast<HANDLE>(mapping_handle_));
            mapping_handle_ = nullptr;
        }
        mapped_name_.clear();
        mapped_size_ = 0;
    }
#endif

    open_ = false;
    in_memory_buffer_.clear();
}

} // namespace netkit::shared_memory
