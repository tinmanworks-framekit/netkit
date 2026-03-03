#include "netkit/control/control_channel_backend.hpp"

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
    (void)endpoint;
    open_ = true;
    return true;
}

bool ControlChannelBackend::Send(const ControlEnvelope& envelope) {
    (void)envelope;
    return open_;
}

std::optional<ControlEnvelope> ControlChannelBackend::Receive() {
    if (!open_) {
        return std::nullopt;
    }
    return ControlEnvelope{};
}

void ControlChannelBackend::Close() {
    open_ = false;
}

} // namespace netkit::control
