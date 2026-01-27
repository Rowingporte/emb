#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <gpiod.h>
#include <time.h>

#define BROADCAST_IP "10.42.0.255"
#define PORT 5000
#define CONSUMER "hcsr04_bridge"

int main() {
    int sock;
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;
    char buffer[64];

    struct gpiod_chip *chip;
    struct gpiod_line *trig, *echo;
    struct timespec start, end;

    // 1. Setup Hardware (Userspace GPIO instead of /dev/hcsr04)
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) { perror("GPIO chip open failed"); exit(1); }

    echo = gpiod_chip_get_line(chip, 12); // PA11
    trig = gpiod_chip_get_line(chip, 13); // PA12
    gpiod_line_request_input(echo, CONSUMER);
    gpiod_line_request_output(trig, CONSUMER, 0);

    // 2. Setup UDP Socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission));
    
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcast_addr.sin_port = htons(PORT);

    printf("Hardware Bridge active: GPIO PA11/12 -> UDP %s:%d\n", BROADCAST_IP, PORT);

    // 3. Measurement & Transfer Loop
    while (1) {
        // Trigger pulse
        gpiod_line_set_value(trig, 1);
        usleep(10);
        gpiod_line_set_value(trig, 0);

        // Wait for echo HIGH (with safety timeout)
        int timeout = 1000000;
        while (gpiod_line_get_value(echo) == 0 && timeout--) ;
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Wait for echo LOW
        timeout = 1000000;
        while (gpiod_line_get_value(echo) == 1 && timeout--) ;
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate Duration (microseconds)
        long duration_us = (end.tv_sec - start.tv_sec) * 1000000 + 
                           (end.tv_nsec - start.tv_nsec) / 1000;

        // Prepare string for broadcast
        int len = snprintf(buffer, sizeof(buffer), "%ld\n", duration_us);
        
        // 4. Send to PC via UDP
        sendto(sock, buffer, len, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));

        // Wait 60ms to avoid ultrasonic interference
        usleep(60000); 
    }

    gpiod_chip_close(chip);
    close(sock);
    return 0;
}