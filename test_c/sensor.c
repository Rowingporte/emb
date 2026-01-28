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

// Vitesse du son : 343 m/s => 0.0343 cm/ns divisé par 2 (aller-retour)
// On utilise la constante : distance = (delta_ns * 34300) / (2 * 10^9)
// Ce qui revient à diviser par 58309.0
#define NS_TO_CM_DIVISOR 58309.0

int main() {
    int sock;
    int fd;
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;
    char read_buffer[64];   // Pour lire les nanosecondes du driver
    char send_buffer[64];   // Pour envoyer la distance formatée

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
            // 1. Convertir la chaîne "14089000" en nombre
            long long delta_ns = atoll(read_buffer);

            // 2. Calculer la distance en cm (avec 2 décimales)
            double distance_cm = (double)delta_ns / NS_TO_CM_DIVISOR;

            // 3. Préparer le message de sortie
            // On envoie un format propre, ex: "12.45 cm"
            int len = snprintf(send_buffer, sizeof(send_buffer), "%.2f cm", distance_cm);

            // 4. Envoi UDP
            sendto(sock, send_buffer, len, 0, 
                   (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            
            printf("Raw: %lld ns | Dist: %.2f cm\n", delta_ns, distance_cm);
        }

        close(fd);

        // Pause de 200ms pour ne pas saturer le capteur (le HC-SR04 aime respirer)
        usleep(200000); 
    }

    close(sock);
    return 0;
}