#pragma once

#include "netkit/control/control_channel_backend.hpp"
#include "netkit/shared_memory/shared_memory_transport.hpp"

#include <memory>
#include <string>

namespace netkit::transport {

struct ShmPipeConfig {
    std::string channel_endpoint;
    std::string shared_memory_name;
    std::size_t shared_memory_capacity = 1024 * 1024;
};

struct ShmPipeBundle {
    std::unique_ptr<netkit::control::ControlChannelBackend> control;
    std::unique_ptr<netkit::shared_memory::SharedMemoryTransport> data;
};

class ShmPipeFactory {
public:
    ShmPipeBundle Create(const ShmPipeConfig& config) const;
};

} // namespace netkit::transport
