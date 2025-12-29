#include "../../include/middleware/v4l2_device.hpp"
#include "../../include/middleware/v4l2_utils.hpp"

#include <linux/videodev2.h>

#include <iostream> 	// debugging/logging
#include <filesystem>	// check path
#include <stdexcept>	// error handling
#include <fcntl.h> 	// open()
#include <unistd.h>	// close(), read(), write()
#include <stdio.h>
#include <errno.h>
#include <string.h> 	// strerror()

#include <cuda_runtime.h>
#include <linux/dma-buf.h>

namespace jetson_middleware { 

V4L2Device::V4L2Device(const char* path, uint32_t width, uint32_t height, uint32_t format)
	: _path(path), _width(width), _height(height), _format(format)
{
	if (_width < 1 || _height < 1)
		throw std::runtime_error("Error: width and height must be positive integers");

	_fd = open(path, 0);
	if (_fd < 0) 
		throw std::runtime_error("Error: issue opening device path");
	

	// query capabilities //
	v4l2_capability cap {};
	if (xioctl(_fd, VIDIOC_QUERYCAP, &cap) < 0) 
		throw std::runtime_error("Error: VIDIOC_QUERYCAP");
	
	const bool is_capture = 
		(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || 	// make sure device supports video capture
		(cap.device_caps & V4L2_CAP_VIDEO_CAPTURE); 	// check physical device & specific opened fd
	
	const bool is_streaming = 
		(cap.capabilities & V4L2_CAP_STREAMING) || 	// make sure device supports streaming I/O via dma-buf
		(cap.device_caps & V4L2_CAP_STREAMING);

	if (!is_capture) 
		throw std::runtime_error("Error: device does not support video capture");
	if (!is_streaming) 
		throw std::runtime_error("Error: device does not support dma-buf I/O streaming");
	
	// set device format //
	v4l2_format fmt {};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = _width;
	fmt.fmt.pix.height = _height;
	fmt.fmt.pix.pixelformat = _format;	// probably V4L2_PIX_FMT_YUYV
	fmt.fmt.pix.field = V4L2_FIELD_NONE;	// may need customization
	
	if (xioctl(_fd, VIDIOC_S_FMT, &fmt) < 0) 
		throw std::runtime_error("Error: VIDIOC_S_FMT");
	
	// VIDIO_C_FMT may change fmt
	_width = fmt.fmt.pix.width;
	_height = fmt.fmt.pix.height;
	_format = fmt.fmt.pix.pixelformat;
}

void V4L2Device::request_buffers(uint32_t count) {
	if (count < 1) 
		throw std::runtime_error("Error: count must be a positive integer");
	if (!_buffers.empty()) 
		throw std::runtime_error("Error: device already has buffers");
	
	// request buffers //
	v4l2_requestbuffers req {};
	req.count = count;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	
	if (xioctl(_fd, VIDIOC_REQBUFS, &req) < 0) 
		throw std::runtime_error("Error: VIDIOC_REQBUFS");
	if (!req.count) 
		throw std::runtime_error("Error: device driver set buffer count to 0 (VIDIOC_REQBUFS)");

	_buffers.resize(req.count);

	// allocate buffers to gpu memory //
	for (uint32_t i = 0; i < req.count; ++i) {
		DeviceBuffer& b = _buffers[i];
		b.index = i;
		
		// query buffer metadata //
		v4l2_buffer buf {};
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (xioctl(_fd, VIDIOC_QUERYBUF, &buf) < 0)
			throw std::runtime_error("Error: VIDIOC_QUERYBUF");

		b.size = buf.length;

		// Export buffer as DMA-BUF //
		v4l2_exportbuffer exp {};
		exp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		exp.index = i;
		exp.plane = 0;		// single-planar video format
		exp.flags = O_CLOEXEC;	// ensure fd does not leak across child processes

		if (xioctl(_fd, VIDIOC_EXPBUF, &exp) < 0) 
			throw std::runtime_error("Error: VIDIOC_EXPBUF");

		b.dmabuf_fd = exp.fd;

		// Import DMA-BUF into CUDA //
		cudaExternalMemoryHandleDesc hdesc {};
		hdesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
		hdesc.handle.fd = b.dmabuf_fd;
		hdesc.size = b.size;

		if (cudaImportExternalMemory(&b.ext_mem, &hdesc) != cudaSuccess) 
			throw std::runtime_error("Error: cudaImportExternalMemory");

		// Map DMA-BUF into GPU memory //
		cudaExternalMemoryBufferDesc bdesc {};
		bdesc.offset = 0;
		bdesc.size = b.size;

		if (cudaExternalMemoryGetMappedBuffer(&b.device_ptr, b.ext_mem, &bdesc) != cudaSuccess)
			throw std::runtime_error("Error: cudaExternalMemoryGetMappedBuffer");
	}
}

}
