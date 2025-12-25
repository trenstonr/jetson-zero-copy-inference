# Jetson TX2 setup script

# exit on error
set -e 

echo "Starting NVIDIA Jetson TX2 setup..."
echo

echo "Updating system packages..."
echo
apt update && apt upgrade -y

echo "Installing dependencies..."
echo
apt install build-essentials git

echo "Setting up v4l2loopback..."
echo
# v4l2loopback allows v4l2 to read from a "virtual device"
# mimics a camera device
# https://github.com/v4l2loopback/v4l2loopback?tab=readme-ov-file

# may have issues with kernel headers (with Linux 4 Tegra)
git clone "https://github.com/v4l2loopback/v4l2loopback.git"

# build
# build for a different kernel (maybe L4T): "$ make KERNELRELEASE=6.11.7-amd64"
cd v4l2loopback
make

# install
make && make install
depmod -a

# run
modprobe v4l2loopback

# test if device was created
# ls -1 /sys/devices/virtual/video4linux
# # video0 \ video3

cd ~

