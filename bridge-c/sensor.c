#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BROADCAST_IP "10.42.0.255"
#define PORT 5000
#define DEVICE_PATH "/dev/hcsr04"


#define NS_TO_CM_DIVISOR 58309.0

int main() {
    int sock;
    int fd;
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;
    char read_buffer[64];  
    char send_buffer[64];   

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        return 1;
    }

    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission));

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcast_addr.sin_port = htons(PORT);

    printf("Monitoring HC-SR04 (Raw Mode) -> Broadcast %s:%d\n", BROADCAST_IP, PORT);

    while (1) {
        fd = open(DEVICE_PATH, O_RDONLY);
        if (fd < 0) {
            perror("Erreur ouverture driver");
            sleep(1);
            continue;
        }

        memset(read_buffer, 0, sizeof(read_buffer));
        ssize_t bytes_read = read(fd, read_buffer, sizeof(read_buffer) - 1);
        
        if (bytes_read > 0) {
            long long delta_ns = atoll(read_buffer);

            double distance_cm = (double)delta_ns / NS_TO_CM_DIVISOR;

            int len = snprintf(send_buffer, sizeof(send_buffer), "%.2f cm", distance_cm);

            sendto(sock, send_buffer, len, 0, 
                   (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            
            printf("Raw: %lld ns | Dist: %.2f cm\n", delta_ns, distance_cm);
        }

        close(fd);

        usleep(200000); 
    }

    close(sock);
    return 0;
}