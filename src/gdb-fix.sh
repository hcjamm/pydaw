
echo "This script is for fixing issues with remote attaching GDB in Ubuntu"
#check for root
if [ "$(id -u)" != "0" ]; then
   echo "Error:  This script must be run as root, use sudo" 1>&2
   exit 1
fi

echo 0 > /proc/sys/kernel/yama/ptrace_scope
