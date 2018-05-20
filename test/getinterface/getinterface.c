//getinterface.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <inttypes.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <ifaddrs.h>

int main(int argc, char** argv)
{
    struct ifaddrs* interface_address;
    struct ifaddrs* it;
    if (getifaddrs(&interface_address) == -1) {
        perror("main: getifaddrs()");
        exit(EXIT_FAILURE);
    }
    it = interface_address;
    size_t i;
    i = 0;
    while (it != NULL) {
        if (it->ifa_addr->sa_family == AF_PACKET) {
            printf("%d: %s\n", (int)i, it->ifa_name);
            ++i;
        }
        it = it->ifa_next;
    }	
    freeifaddrs(interface_address);
    return 0;
}
