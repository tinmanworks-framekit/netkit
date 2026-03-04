#pragma once

#include <string>

namespace netkit::transport {

struct TransportLifecycleEvent {
    std::string component;
    std::string operation;
    bool success = false;
};

class ITransportLifecycleMetricsHook {
public:
    virtual ~ITransportLifecycleMetricsHook() = default;

    virtual void OnLifecycleEvent(const TransportLifecycleEvent& event) = 0;
};

} // namespace netkit::transport
