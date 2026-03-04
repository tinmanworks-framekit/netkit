#include "netkit/netkit.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

namespace {

class CaptureMetricsHook : public netkit::transport::ITransportLifecycleMetricsHook {
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

    auto metrics = std::make_shared<CaptureMetricsHook>();
    factory.SetMetricsHook(metrics);

    netkit::transport::ShmPipeConfig config;
    config.channel_endpoint = "e2e-control";
    config.shared_memory_name = "e2e-shm";
    config.shared_memory_capacity = 256;

    auto bundle = factory.Create(config);
    assert(bundle.control);
    assert(bundle.data);
    assert(bundle.control->IsOpen());
    assert(bundle.data->IsOpen());
    assert(factory.LastError().code == netkit::transport::TransportErrorCode::kNone);

    netkit::control::ControlEnvelope startup;
    startup.sequence_id = 1;
    startup.message_type = "child-ready";
    startup.payload = "ok";
    assert(bundle.control->Send(startup));
    const auto startup_rx = bundle.control->Receive();
    assert(startup_rx.has_value());
    assert(startup_rx->message_type == "child-ready");

    std::vector<std::uint8_t> frame_payload{10, 20, 30, 40, 50};
    assert(bundle.data->Write(frame_payload));
    const auto frame_rx = bundle.data->Read();
    assert(frame_rx == frame_payload);

    bundle.control->Close();
    bundle.data->Close();
    assert(!bundle.control->IsOpen());
    assert(!bundle.data->IsOpen());

    assert(!metrics->events.empty());
    assert(metrics->events.front().operation == "create.begin");
    assert(metrics->events.back().operation == "create.complete");
    assert(metrics->events.back().success);

    return 0;
}
