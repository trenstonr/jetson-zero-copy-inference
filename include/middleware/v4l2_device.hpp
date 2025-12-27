#ifndef JETSON_VIDEO_MIDDLEWARE_HPP
#define JETSON_VIDEO_MIDDLEWARE_HPP

namespace jetson_middleware {

// buffer = frame
struct DeviceBuffer {
	uint32_t index;	// v4l2 buffer idx
	void *data;	// pointer to CUDA Unified Memory
	size_t size;	// size of frame data (bytes)
	int dmabuf_fd;	// DMA-BUF file descriptor
};

class V4L2Device {
public:
	V4L2Device(std::string path, uint32_t width, uint32_t height);
	~V4L2Device();

	void request_buffers(uint32_t count); 	// allocate buffer pool (v4l2_requestbuffers, VIDIOC_REQBUFS, cudaMallocManaged)
	
	void start_stream(); 	// start device streaming (v4l2_buffer, VIDIOC_QBUF, VIDIOC_STREAMON)
	void stop_stream();	// stop device streaming (VIDIOC_DQBUF, VIDIOC_STREAMOFF)
	
	void enqueue_frame();		// enqueue empty frames for device to write to (v4l2_buffer, VIDIOC_QBUF)
	DeviceBuffer& dequeue_frame();	// dequeue written frames to process (v4l2_buffer, VIDIOC_DQBUF) 


private:
	int _fd = -1;
	std::string _path;

	uint32_t _width, _height; 			// frame dimensions (pixels)
	uint32_t _pixel_fmt = V4L2_PIX_FMT_UYVY;	// may need to be changed / may be corrected by _set_format()
	
	std::vector<DeviceBuffer> _buffers;		// buffer pool

	void _set_format(); // (VIDIOC_S_FMT)
};

}

#endif
