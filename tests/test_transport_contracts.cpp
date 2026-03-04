#include "netkit/netkit.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    netkit::transport::ShmPipeFactory factory;
    netkit::transport::ShmPipeConfig cfg;
    cfg.channel_endpoint = "local-control";
    cfg.shared_memory_name = "local-shm";
    cfg.shared_memory_capacity = 32;

    auto bundle = factory.Create(cfg);
    assert(bundle.control);
    assert(bundle.data);

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

    return 0;
}
