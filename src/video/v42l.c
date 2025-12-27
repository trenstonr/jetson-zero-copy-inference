include <fcntl.h> // open
#include <unistd.h> // read/write/close
#include <stdio.h> // I/O printf
#include <sys/ioctl.h>
#include <stdlib.h> // exit
		   
#include <linux/videodev2.h> // v4l2

#define DEVICE "/dev/video0"

int main() {
	int fd = open(DEVICE, O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Error opening device: %s\n", DEVICE);
		exit(EXIT_FAILURE);
	}

	// https://github.com/Adityasakare/V4L2-Programs/blob/main/1_DeviceInfo/Program1.c
	struct v4l2_capability caps {0}; 	// info about driver and hardware capabilities
	struct v4l2_cropcap crop_caps {0};	// cropping limits (pixel aspect of images / calculate scale factors)
	
	int query;

	query = ioctl(fd, VIDIOC_QUERYCAP, &caps);
	if (query == -1) {
		fprintf(stderr, "Error querying device capabilties: %s\n", DEVICE);
		exit(EXIT_FAILURE);
	}

	query = ioctl(fd, VIDEOC_CROPCAP, &crop_caps);
	if (query == -1) {
		fprintf(stderr, "Error querying device cropping capabilties: %s\n", DEVICE);
		exit(EXIT_FAILURE);
	}

	struct v4l2_format fmt {0};

	// query device format
	query = ioctl(fd, VIDIOC_G_FMT, &fmt);	
	if (query == -1) {
		fprintf(stderr, "Error querying device format: %s\n", DEVICE);
		exit(EXIT_FAILURE);
	}

	// validate device format (pixel width, height, format, etc...)
	// validate_format();
	
	// then make changes as needed
	// fmt.fmt.pix.width = x, ....
	// query = ioctl(fd, VIDEOC_S_FMT, &fmt);


	// request buffers
	struct v4l2_requestbuffers req {0};
	req.count = 5; // adjust this as needed
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE; // using multi-planar not single-planr
	req.memory = V4L2_MEMORY_DMABUF

	query = ioctl(fd, VIDIOC_REQBUFS, &req);
	if (query == -1) {
		fprintf(stderr, "Error requesting buffers: %s\n", DEVICE);
		exit(EXIT_FAILURE);
	}
	
	// close fd
	query = close(fd);
	if (query == -1) {
		fprintf(stderr, "Error closing device fd: %s\n", DEVICE);
		exit(EXIT_FAILURE);
	}

	return 0;
}
