#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BROADCAST_IP "10.42.0.255"
#define PORT 5000
#define DEVICE_PATH "/dev/hcsr04"

int main() {
    int sock;
    int fd;
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;
    char buffer[64]; // Buffer pour lire la chaîne du driver (ex: "244 cm")

    // --- Setup Réseau ---
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        return 1;
    }

    // Autoriser le broadcast
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission));

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcast_addr.sin_port = htons(PORT);

    printf("Lecture de %s et envoi vers %s:%d\n", DEVICE_PATH, BROADCAST_IP, PORT);

    while (1) {
        // 1. Ouverture du driver
        fd = open(DEVICE_PATH, O_RDONLY);
        if (fd < 0) {
            perror("Erreur : impossible d'ouvrir /dev/hcsr04. Driver chargé ?");
            sleep(1);
            continue;
        }

        // 2. Lecture de la donnée générée par le driver
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            // Supprimer le saut de ligne éventuel pour un envoi propre
            buffer[strcspn(buffer, "\n")] = 0;

            // 3. Envoi UDP du message (ex: "244 cm")
            sendto(sock, buffer, strlen(buffer), 0, 
                   (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            
            printf("Broadcast envoyé: [%s]\n", buffer);
        }

        close(fd);

        // 4. Fréquence d'envoi (500ms)
        usleep(500000); 
    }

    close(sock);
    return 0;
}