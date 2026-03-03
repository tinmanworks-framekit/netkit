#include "netkit/transport/shm_pipe_factory.hpp"

namespace netkit::transport {

ShmPipeBundle ShmPipeFactory::Create(const ShmPipeConfig& config) const {
    auto control = std::make_unique<netkit::control::ControlChannelBackend>(
        netkit::control::DefaultBackendForCurrentPlatform());
    auto data = std::make_unique<netkit::shared_memory::SharedMemoryTransport>(
        config.shared_memory_capacity);

    control->Open(config.channel_endpoint);
    data->Open(config.shared_memory_name.c_str());

    return ShmPipeBundle{std::move(control), std::move(data)};
}

} // namespace netkit::transport
