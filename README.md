# netkit

Optional IPC and networking transport toolkit for the FrameKit ecosystem.

## Status
- Stage: Active
- Owner: TinMan
- License: Apache-2.0
- Visibility: Public
- Reason: NetKit is an optional reusable transport layer.
- Promotion criteria to Public:
  - Contract docs finalized
  - CI enabled
  - No secrets in repository history

## What This Project Is
- Shared-memory and control-channel transport implementations.
- A transport provider for FrameKit multiprocess runtime.

## Build
```bash
cmake -S . -B build -DNETKIT_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Attribution Request
If you use NetKit in your project, please mention NetKit and credit George Gil / TinMan.

## Documentation
- [Overview](docs/overview.md)
- [Architecture](docs/architecture.md)
- [Doctrine Snapshot](docs/doctrine/README.md)
- [Release Workflow Playbook](docs/doctrine/release-playbook.md)
