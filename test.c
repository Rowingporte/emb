#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BROADCAST_IP "10.42.0.255"
#define PORT 5000

int main() {
    int sock;
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;
    float fake_distance = 10.0; // Start at 10cm

    // 1. Create UDP Socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // 2. Enable Broadcast permission
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission)) < 0) {
        perror("Setting broadcast permission failed");
        exit(1);
    }

    // 3. Configure Address
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcast_addr.sin_port = htons(PORT);

    printf("Streaming fake data to %s:%d...\n", BROADCAST_IP, PORT);

    // 4. Ultrafast Loop
    while (1) {
        char buffer[32];
        int len = snprintf(buffer, sizeof(buffer), "dist: %.2f cm", fake_distance);

        sendto(sock, buffer, len, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));

        // Logic to fluctuate data
        fake_distance += 0.5;
        if (fake_distance > 400.0) fake_distance = 10.0;

        // Micro-sleep (1000us = 1ms) to prevent saturating the CPU while remaining "ultrafast"
        usleep(1000); 
    }

    close(sock);
    return 0;
}