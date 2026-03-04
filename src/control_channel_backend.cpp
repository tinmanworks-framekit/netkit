#include "netkit/control/control_channel_backend.hpp"

#if defined(__linux__) || defined(__APPLE__)
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

namespace netkit::control {

BackendKind DefaultBackendForCurrentPlatform() {
#if defined(_WIN32)
    return BackendKind::kNamedPipe;
#elif defined(__APPLE__) || defined(__linux__)
    return BackendKind::kUnixDomainSocket;
#else
    return BackendKind::kInMemory;
#endif
}

bool ControlChannelBackend::Open(const std::string& endpoint) {
#if defined(__linux__) || defined(__APPLE__)
    if (kind_ == BackendKind::kUnixDomainSocket) {
        endpoint_ = endpoint.empty() ? "netkit-control" : endpoint;

        uds_fd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (uds_fd_ < 0) {
            return false;
        }

        open_ = true;
        return true;
    }
#endif
#if defined(_WIN32)
    if (kind_ == BackendKind::kNamedPipe) {
        endpoint_ = endpoint.empty() ? "\\\\.\\pipe\\netkit-control" : endpoint;
        if (endpoint_.rfind("\\\\.\\pipe\\", 0) != 0) {
            endpoint_ = "\\\\.\\pipe\\" + endpoint_;
        }

        named_pipe_handle_ = CreateNamedPipeA(
            endpoint_.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            4096,
            4096,
            0,
            nullptr);
        if (named_pipe_handle_ == INVALID_HANDLE_VALUE) {
            named_pipe_handle_ = nullptr;
            return false;
        }

        open_ = true;
        return true;
    }
#endif

    endpoint_ = endpoint;
    open_ = true;
    return true;
}

bool ControlChannelBackend::Send(const ControlEnvelope& envelope) {
    if (!open_) {
        return false;
    }

    inbox_.push_back(envelope);
    return true;
}

std::optional<ControlEnvelope> ControlChannelBackend::Receive() {
    if (!open_) {
        return std::nullopt;
    }

    if (inbox_.empty()) {
        return std::nullopt;
    }

    auto envelope = inbox_.front();
    inbox_.pop_front();
    return envelope;
}

void ControlChannelBackend::Close() {
#if defined(__linux__) || defined(__APPLE__)
    if (uds_fd_ >= 0) {
        close(uds_fd_);
        uds_fd_ = -1;
    }
#endif
#if defined(_WIN32)
    if (named_pipe_handle_ != nullptr) {
        CloseHandle(static_cast<HANDLE>(named_pipe_handle_));
        named_pipe_handle_ = nullptr;
    }
#endif

    open_ = false;
    endpoint_.clear();
    inbox_.clear();
}

} // namespace netkit::control
