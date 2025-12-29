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

namespace jetson_middleware { 

V4L2Device::V4L2Device(const char* path, uint32_t width, uint32_t height) {
	_set_params(path, width, height);

	_validate_capabilities(_path);

	// figure out which V4L2_MEMORY_x ioctl to use
	// validate that ioctl against the device

	_set_format();
}

V4L2Device::~V4L2Device() {
	int res = close(_fd);
	if (res == -1) {
		int err = errno;
		fprintf(stderr, "ERROR: failed to close fd %d: %s\n", _fd, strerror(err));
	}
}

void V4L2Device::request_buffers(uint32_t count) {}

void V4L2Device::enqueue_frame() {}

// DeviceBuffer& V4L2Device::dequeue_frame() {}

void V4L2Device::start_stream() {}

void V4L2Device::stop_stream() {}

void V4L2Device::_set_format() {
	v4l2_format fmt;

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = _width;
	fmt.fmt.pix.height = _height;
	// fmt.fmt.pixelformat = V4L2_PIX_FMT_UYVY;
	// fmt.fmt.field = V4L2_FIELD_INTERLACED;

	if (xioctl(_fd, VIDIOC_S_FMT, &fmt) == -1) throw std::runtime_error("FORMAT");

	// VIDIOC_S_FMT may alter fmt. probably verify width/height after
}

void _set_params(const char* path, uint32_t width, uint32_t height) {
	const std::filesystem::path dir(path);

	if (!std::filesystem::exists(dir)) {
		std::cerr << path << " does not exist\n";
		throw std::runtime_error("PATH");
	}
	_path = path;

	if (width < 1) {
		std::cerr << "Frame width must be at least 1px\n";
		throw std::runtime_error("WIDTH");
	}
	_width = width;

	if (height < 1) {
		std::cerr << "Frame height must be at least 1px\n";
		throw std::runtime_error("HEIGHT");
	}
	_height = height;

	_fd = open(path, 0);
	if (_fd == -1) {
		int err = errno;
		fprintf(stderr, "ERROR: failed to open path '%s': %s\n", path, strerror(err));
		throw std::runtime_error("PATH");
	}
}

void _validate_capabilities(int fd) {
	struct v4l2_capability cap;
	if (xioctl(_fd, VIDIOC_QUERYCAP, &cap) == -1) throw std::runtime_error("DEVICE CAPABILITIES");
	
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE == 0) {
		fprintf(stderr, "ERROR: device does not support video capture");
		throw std::runtime_error("DEVICE VIDEO CAPTURE");
	}
}
}
