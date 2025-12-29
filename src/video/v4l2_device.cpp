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

namespace jetson_middleware { 

V4L2Device::V4L2Device(const char* path, uint32_t width, uint32_t height)
	: _path(path), _width(width), _height(height) 
{
	_fd = open(path, 0);
	if (_fd < 0) {} // err
	

	// query capabilities //
	v4l2_capability cap {};
	if (xioctl(_fd, VIDIOC_QUERYCAP, &cap) < 0) {} // err
	
	const bool is_capture = 
		(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || 	// make sure device supports video capture
		(cap.device_caps & V4L2_CAP_VIDEO_CAPTURE); 	// check physical device & specific opened fd
	
	const bool is_streaming = 
		(cap.capabilities & V4L2_CAP_STREAMING) || 	// make sure device supports streaming I/O via user pointers
		(cap.device_caps & V4L2_CAP_STREAMING);

	if (!is_capture) {} 	// err
	if (!is_streaming) {} 	// err
	
	// set device format //
	v4l2_format fmt {};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = _width;
	fmt.fmt.pix.height = _height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;	// may need customization
	fmt.fmt.pix.field = V4L2_FIELD_NONE;		// may need customization
	
	if (xioctl(_fd, VIDEOC_S_FMT, &fmt) < 0) {} // err
	
	// VIDIO_C_FMT may change fmt
	_width = fmt.fmt.pix.width;
	_height = fmt.fmt.pix.height;
	// may need one for pixelformat
}

V4L2Device::~V4L2Device() {
}

void V4L2Device::request_buffers(uint32_t count) {
	if (!count) {} // err
	if (!_buffers.empty()) {} // err
	
	// request buffers //
	v4l2_requestbuffers req {};
	req.count = count;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR; // user pointers to pass to allocate via cuda
	
	if (xioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {} // err
	if (req.count < count) {
		// driver can alter req
		count = req.count;
		if (!count) {} // err
	}

	_buffers.resize(count);

	// allocate buffers to gpu memory //
	for (uint32_t i = 0; i < count; ++i) {
		DeviceBuffer& b = _buffers[i];
		b.index = i;
		// b.size = ???; bytes/frame -- probably calculate with pixelformat/width/height
		b.host_ptr = nullptr;
		b.device_ptr = nullptr;

				
	}
}

void V4L2Device::enqueue_frame() {}

// DeviceBuffer& V4L2Device::dequeue_frame() {}

void V4L2Device::start_stream() {}

void V4L2Device::stop_stream() {}

}
}
