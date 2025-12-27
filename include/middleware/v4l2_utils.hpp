#ifndef V4L2_UTILITIES_HPP
#define V4L2_UTILITIES_HPP

namespace jetson_middleware {


int xioctl(int fd, unsigned long code, void* arg);

}

#endif
