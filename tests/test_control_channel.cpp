#include "netkit/control/control_channel_backend.hpp"

#include <cassert>
#include <string>

namespace {

void AssertControlRoundTrip(netkit::control::BackendKind kind, const std::string& endpoint) {
    netkit::control::ControlChannelBackend channel{kind};
    assert(channel.Open(endpoint));
    assert(channel.IsOpen());

    netkit::control::ControlEnvelope envelope;
    envelope.sequence_id = 101;
    envelope.message_type = "ping";
    envelope.payload = "payload";
    assert(channel.Send(envelope));

    const auto received = channel.Receive();
    assert(received.has_value());
    assert(received->sequence_id == 101);
    assert(received->message_type == "ping");
    assert(received->payload == "payload");

    channel.Close();
    assert(!channel.IsOpen());
}

} // namespace

int main() {
    const auto default_backend = netkit::control::DefaultBackendForCurrentPlatform();
#if defined(_WIN32)
    assert(default_backend == netkit::control::BackendKind::kNamedPipe);
#elif defined(__linux__) || defined(__APPLE__)
    assert(default_backend == netkit::control::BackendKind::kUnixDomainSocket);
#else
    assert(default_backend == netkit::control::BackendKind::kInMemory);
#endif

#if defined(__linux__) || defined(__APPLE__)
    AssertControlRoundTrip(netkit::control::BackendKind::kUnixDomainSocket, "netkit-uds-test");
#endif
#if defined(_WIN32)
    AssertControlRoundTrip(netkit::control::BackendKind::kNamedPipe, "netkit-pipe-test");
#endif
    AssertControlRoundTrip(netkit::control::BackendKind::kInMemory, "memory-test");

    return 0;
}
