#include "netkit/netkit.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

namespace {

class FakeMetricsHook : public netkit::transport::ITransportLifecycleMetricsHook {
public:
    void OnLifecycleEvent(const netkit::transport::TransportLifecycleEvent& event) override {
        events.push_back(event);
    }

    std::vector<netkit::transport::TransportLifecycleEvent> events;
};

} // namespace

int main() {
    netkit::transport::ShmPipeFactory factory;
    netkit::transport::RetryPolicy retry_policy;
    retry_policy.max_attempts = 2;
    factory.SetRetryPolicy(retry_policy);
    assert(factory.CurrentRetryPolicy().max_attempts == 2);
    auto metrics = std::make_shared<FakeMetricsHook>();
    factory.SetMetricsHook(metrics);

    netkit::transport::ShmPipeConfig cfg;
    cfg.channel_endpoint = "local-control";
    cfg.shared_memory_name = "local-shm";
    cfg.shared_memory_capacity = 32;

    auto bundle = factory.Create(cfg);
    assert(bundle.control);
    assert(bundle.data);
    assert(bundle.control->IsOpen());

#if defined(__linux__) || defined(__APPLE__)
    assert(bundle.control->Kind() == netkit::control::BackendKind::kUnixDomainSocket);
#endif
#if defined(_WIN32)
    assert(bundle.control->Kind() == netkit::control::BackendKind::kNamedPipe);
#endif

#if defined(__linux__)
    const auto linux_backend = bundle.data->BackendKind();
    assert(
        linux_backend == netkit::shared_memory::SharedMemoryBackendKind::kLinuxPosix ||
        linux_backend == netkit::shared_memory::SharedMemoryBackendKind::kInMemoryFallback);
#endif
#if defined(__APPLE__)
    const auto mac_backend = bundle.data->BackendKind();
    assert(
        mac_backend == netkit::shared_memory::SharedMemoryBackendKind::kMacPosix ||
        mac_backend == netkit::shared_memory::SharedMemoryBackendKind::kInMemoryFallback);
#endif
#if defined(_WIN32)
    const auto windows_backend = bundle.data->BackendKind();
    assert(
        windows_backend == netkit::shared_memory::SharedMemoryBackendKind::kWindowsFileMapping ||
        windows_backend == netkit::shared_memory::SharedMemoryBackendKind::kInMemoryFallback);
#endif

    std::vector<std::uint8_t> payload{1, 2, 3};
    assert(bundle.data->Write(payload));

    auto read_back = bundle.data->Read();
    assert(read_back.size() == payload.size());

    netkit::control::ControlEnvelope control_message;
    control_message.sequence_id = 7;
    control_message.message_type = "start";
    control_message.payload = "payload";
    assert(bundle.control->Send(control_message));

    const auto received = bundle.control->Receive();
    assert(received.has_value());
    assert(received->sequence_id == 7);
    assert(received->message_type == "start");
    assert(factory.LastError().code == netkit::transport::TransportErrorCode::kNone);
    assert(!metrics->events.empty());
    assert(metrics->events.front().operation == "create.begin");
    assert(metrics->events.back().operation == "create.complete");
    assert(metrics->events.back().success);

    return 0;
}
