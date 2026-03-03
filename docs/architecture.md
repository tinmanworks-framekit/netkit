# NetKit Architecture

## Components
1. `control` for command and lifecycle signaling.
2. `shared_memory` for high-throughput data plane.
3. `transport` for composed bundles such as ShmPipe.

## Integration
FrameKit owns runtime contracts; NetKit supplies optional implementations.
