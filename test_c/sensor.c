#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CHIP "gpiochip4"
#define TRIG_PIN 10
#define ECHO_PIN 1

// Vos nouveaux paramètres
#define BROADCAST_IP "10.42.0.255"
#define PORT 5000

int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *trig_line, *echo_line;
    struct timespec ts_start, ts_end;

    // --- Setup Réseau ---
    int sock;
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;
    char buffer[128];

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

    // --- Setup GPIO ---
    chip = gpiod_chip_open_by_name(CHIP);
    trig_line = gpiod_chip_get_line(chip, TRIG_PIN);
    echo_line = gpiod_chip_get_line(chip, ECHO_PIN);
    gpiod_line_request_output(trig_line, "hcsr04_trig", 0);
    gpiod_line_request_input(echo_line, "hcsr04_echo");

    printf("Monitoring HC-SR04 -> Broadcast vers %s:%d\n", BROADCAST_IP, PORT);

    while (1) {
        // Trigger
        gpiod_line_set_value(trig_line, 1);
        usleep(10);
        gpiod_line_set_value(trig_line, 0);

        // Mesure (Polling)
        int timeout = 1000000;
        while (gpiod_line_get_value(echo_line) == 0 && timeout--);
        clock_gettime(CLOCK_MONOTONIC, &ts_start);

        timeout = 1000000;
        while (gpiod_line_get_value(echo_line) == 1 && timeout--);
        clock_gettime(CLOCK_MONOTONIC, &ts_end);

        long long duration_ns = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000LL + (ts_end.tv_nsec - ts_start.tv_nsec);
        
        if (duration_ns > 0 && duration_ns < 40000000LL) { // Limite ~6-7 mètres max
            double distance_cm = ((double)duration_ns / 1e9 * 34300.0) / 2.0;
            
            // Envoi UDP
            int len = snprintf(buffer, sizeof(buffer), "%.2f", distance_cm);
            sendto(sock, buffer, len, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            
            printf("Envoyé: %s cm\n", buffer);
        }

        usleep(200000); // 5 mesures par seconde
    }

    return 0; // Pensez à fermer les ressources proprement en cas de sortie
}