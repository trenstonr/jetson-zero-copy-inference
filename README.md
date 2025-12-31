# jetson-zero-copy-inference
A C++ middleware for NVIDIA Jetson that enables direct memory sharing between camera capture and GPU processing using V4L2 (Video for Linux) and CUDA to minimize latency and bandwidth in real-time vision pipelines.

## File Structure
```
.
├── cmake/          # Build scripts for CUDA and TensorRT discovery
├── examples/       # End-to-end vision pipeline demos
├── include/        # Public headers and Middleware API
├── scripts/        # Jetson hardware and environment setup
├── src/            # V4L2 backend and CUDA kernel implementations
└── tests/          # Performance benchmarks and latency profiling
```
