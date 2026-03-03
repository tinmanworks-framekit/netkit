#pragma once

#include "netkit/control/control_envelope.hpp"

#include <optional>
#include <string>

namespace netkit::control {

enum class BackendKind {
    kNamedPipe,
    kUnixDomainSocket,
    kInMemory,
};

class ControlChannelBackend {
public:
    explicit ControlChannelBackend(BackendKind kind) : kind_(kind) {}

    bool Open(const std::string& endpoint);
    bool Send(const ControlEnvelope& envelope);
    std::optional<ControlEnvelope> Receive();
    void Close();

    bool IsOpen() const { return open_; }
    BackendKind Kind() const { return kind_; }

private:
    BackendKind kind_;
    bool open_ = false;
};

BackendKind DefaultBackendForCurrentPlatform();

} // namespace netkit::control
