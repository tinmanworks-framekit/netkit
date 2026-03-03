#pragma once

#include <cstdint>
#include <string>

namespace netkit::control {

struct ControlEnvelope {
    std::uint64_t sequence_id = 0;
    std::string message_type;
    std::string payload;
};

} // namespace netkit::control
