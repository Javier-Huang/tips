DISK_DEV="/dev/sda"
parted -s ${DISK_DEV} mklabel gpt
parted -s -a optimal ${DISK_DEV} mkpart primary '0%' '100%'
# enable msftdata for windows access
parted -s ${DISK_DEV} set 1 msftdata on
# set cluster-size=128k for compatible Mac/Windows
mkfs.exfat -L DataJar -c 128K ${DISK_DEV}1
sync;sync
