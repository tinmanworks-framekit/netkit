#include "netkit/transport/shm_pipe_factory.hpp"

namespace netkit::transport {

ShmPipeBundle ShmPipeFactory::Create(const ShmPipeConfig& config) {
    last_error_ = {};

    auto control = std::make_unique<netkit::control::ControlChannelBackend>(
        netkit::control::DefaultBackendForCurrentPlatform());
    auto data = std::make_unique<netkit::shared_memory::SharedMemoryTransport>(
        config.shared_memory_capacity);

    const auto attempts = retry_policy_.max_attempts == 0 ? 1 : retry_policy_.max_attempts;
    bool control_opened = false;
    bool data_opened = false;
    for (std::size_t attempt = 0; attempt < attempts; ++attempt) {
        control_opened = control->Open(config.channel_endpoint);
        data_opened = data->Open(config.shared_memory_name.c_str());
        if (control_opened && data_opened) {
            break;
        }

        if (control_opened) {
            control->Close();
        }
        if (data_opened) {
            data->Close();
        }
    }

    if (!control_opened) {
        last_error_.code = TransportErrorCode::kControlOpenFailed;
        last_error_.message = "failed to open control channel backend";
        last_error_.retryable = attempts > 1;
    } else if (!data_opened) {
        last_error_.code = TransportErrorCode::kDataOpenFailed;
        last_error_.message = "failed to open shared memory data plane";
        last_error_.retryable = attempts > 1;
    }

    return ShmPipeBundle{std::move(control), std::move(data)};
}

} // namespace netkit::transport
