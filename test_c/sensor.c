#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define SENSOR_DEVICE "/dev/hcsr04"
#define BROADCAST_IP "10.42.0.255"
#define PORT 5000

int main() {
    int fd, sock;
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;
    char buffer[64];

    // 1. Open the Sensor Node
    fd = open(SENSOR_DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("Error: Could not open /dev/hcsr04. Is the module loaded?");
        exit(1);
    }

    // 2. Setup UDP Socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission));
    
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcast_addr.sin_port = htons(PORT);

    printf("Real data bridge active: /dev/hcsr04 -> %s:%d\n", BROADCAST_IP, PORT);

    // 3. High-Speed Transfer Loop
    while (1) {
        // Read raw data from driver
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate just in case
            
            // Send to PC via UDP
            sendto(sock, buffer, bytes_read, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        }

        // HC-SR04 needs a cooldown. 60ms is the "safe" speed of sound round-trip.
        usleep(60000); 
        
        /* Note: If your driver is 'blocking', it will handle the 
           timing for you and you can remove the usleep. */
    }

    close(fd);
    close(sock);
    return 0;
}