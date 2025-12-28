#ifndef V4L2_UTILITIES_HPP
#define V4L2_UTILITIES_HPP

namespace jetson_middleware {

const char* ioctl_to_str(unsigned long code);

int xioctl(int fd, unsigned long code, void* arg);

}

#endif
