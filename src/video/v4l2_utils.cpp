#include "../../include/middleware/v4l2_utils.hpp"

#include <linux/videodev2.h>

#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

const char* ioctl_to_str(unsigned long code) {
	switch (code) {
		case VIDIOC_QUERYCAP:   return "VIDIOC_QUERYCAP";
		case VIDIOC_S_FMT:      return "VIDIOC_S_FMT";
		case VIDIOC_G_FMT:      return "VIDIOC_G_FMT";
		case VIDIOC_REQBUFS:    return "VIDIOC_REQBUFS";
		case VIDIOC_QBUF:       return "VIDIOC_QBUF";
		case VIDIOC_DQBUF:      return "VIDIOC_DQBUF";
		case VIDIOC_STREAMON:   return "VIDIOC_STREAMON";
		case VIDIOC_STREAMOFF:  return "VIDIOC_STREAMOFF";
		default:                return "UNKNOWN_IOCTL";
	}
}

int xioctl(int fd, unsigned long code, void* arg) {
	int retval;
	
	do retval = ioctl(fd, code, arg);
	while (retval == -1 && errno == EINTR); // retry if interrupted by a signal
	
	if (retval == -1) {
		int err = errno;
		fprintf(stderr, "ERROR: ioctl '%s', fd %d: %s\n", ioctl_to_str(code), fd, strerror(err));
	}

	return retval;
}
