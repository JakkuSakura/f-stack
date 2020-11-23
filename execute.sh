#!/bin/sh
export FF_PATH=/home/jack/f-stack
# clone F-Stack
mkdir -p $FF_PATH
git clone https://github.com/F-Stack/f-stack.git $FF_PATH

# Install libnuma-dev
#yum -y install numactl-devel          # on Centos
sudo apt -y install libnuma-dev  # on Ubuntu


# for VMware
sed -i "s/pci_intx_mask_supported(udev->pdev)/1/g" \
  $FF_PATH/dpdk/kernel/linux/igb_uio/igb_uio.c

# x86_64-native-linuxapp-gcc and exit
printf "38\n\n62\n" | $FF_PATH/dpdk/usertools/dpdk-setup.sh


cd $FF_PATH/dpdk/x86_64-native-linuxapp-gcc/ && make install

# Set hugepage
# single-node system
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# or NUMA
echo 1024 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 1024 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages

# Using Hugepage with the DPDK
mkdir /mnt/huge
mount -t hugetlbfs nodev /mnt/huge

# Close ASLR; it is necessary in multiple process
echo 0 > /proc/sys/kernel/randomize_va_space


# Offload NIC
modprobe uio
insmod $FF_PATH/dpdk/x86_64-native-linuxapp-gcc/kmod/igb_uio.ko
insmod $FF_PATH/dpdk/x86_64-native-linuxapp-gcc/kmod/rte_kni.ko carrier=on # carrier=on is necessary, otherwise need to be up `veth0` via `echo 1 > /sys/class/net/veth0/carrier`

apt -y install python
dpdk-devbind --status
#ifconfig eth0 down
dpdk-devbind --bind=igb_uio ens38 # assuming that use 10GE NIC and eth0



# On Ubuntu, use gawk instead of the default mawk.
apt -r install gawk  # or execute `sudo update-alternatives --config awk` to choose gawk.

# Install dependencies for F-Stack
apt -y install gcc make libssl-dev # On ubuntu

#export FF_PATH=/data/f-stack
export FF_DPDK=$FF_PATH/dpdk/x86_64-native-linuxapp-gcc

cd $FF_PATH/lib && make clean && make && make install

# shellcheck disable=SC2164
cd $FF_PATH/example && make clean && make
cd $FF_PATH && example/helloworld
