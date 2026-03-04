#pragma once

#include "netkit/control/control_channel_backend.hpp"
#include "netkit/shared_memory/shared_memory_transport.hpp"
#include "netkit/transport/lifecycle_metrics.hpp"
#include "netkit/transport/transport_error.hpp"

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
    ShmPipeBundle Create(const ShmPipeConfig& config);

    void SetRetryPolicy(const RetryPolicy& retry_policy) { retry_policy_ = retry_policy; }
    const RetryPolicy& CurrentRetryPolicy() const { return retry_policy_; }
    const TransportError& LastError() const { return last_error_; }
    void SetMetricsHook(std::shared_ptr<ITransportLifecycleMetricsHook> metrics_hook) {
        metrics_hook_ = std::move(metrics_hook);
    }

private:
    void EmitEvent(std::string operation, bool success) const;

    RetryPolicy retry_policy_{};
    TransportError last_error_{};
    std::shared_ptr<ITransportLifecycleMetricsHook> metrics_hook_;
};

} // namespace netkit::transport
