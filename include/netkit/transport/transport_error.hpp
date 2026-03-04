#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace netkit::transport {

enum class TransportErrorCode : std::uint8_t {
    kNone = 0,
    kControlOpenFailed = 1,
    kDataOpenFailed = 2,
};

struct TransportError {
    TransportErrorCode code = TransportErrorCode::kNone;
    std::string message;
    bool retryable = false;
};

struct RetryPolicy {
    std::size_t max_attempts = 1;
};

} // namespace netkit::transport
