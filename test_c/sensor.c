#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define CHIP "gpiochip4"
#define TRIG_PIN 10  // PE10
#define ECHO_PIN 1   // PE1

int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *trig_line, *echo_line;
    struct timespec ts_start, ts_end;

    chip = gpiod_chip_open_by_name(CHIP);
    if (!chip) {
        perror("Erreur ouverture chip");
        return 1;
    }

    trig_line = gpiod_chip_get_line(chip, TRIG_PIN);
    echo_line = gpiod_chip_get_line(chip, ECHO_PIN);

    // Configurer Trig en sortie
    gpiod_line_request_output(trig_line, "hcsr04_trig", 0);
    
    // Configurer Echo en entrée simple (Polling)
    // On ne demande plus d'événements, donc plus de conflit d'IRQ !
    gpiod_line_request_input(echo_line, "hcsr04_echo");

    printf("Mesure par POLLING sur PE10 (Trig) et PE1 (Echo)...\n");

    while (1) {
        // 1. Impulsion Trigger de 10us
        gpiod_line_set_value(trig_line, 0);
        usleep(2);
        gpiod_line_set_value(trig_line, 1);
        usleep(10);
        gpiod_line_set_value(trig_line, 0);

        // 2. Attendre que l'Echo passe à 1 (Début du signal)
        // On ajoute un timeout de sécurité pour ne pas bloquer le CPU
        int timeout = 1000000; 
        while (gpiod_line_get_value(echo_line) == 0 && timeout--) ;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);

        // 3. Attendre que l'Echo repasse à 0 (Fin du signal)
        timeout = 1000000;
        while (gpiod_line_get_value(echo_line) == 1 && timeout--) ;
        clock_gettime(CLOCK_MONOTONIC, &ts_end);

        // 4. Calcul de la durée en nanosecondes
        long long duration_ns = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000LL + 
                                (ts_end.tv_nsec - ts_start.tv_nsec);

        if (duration_ns > 0 && duration_ns < 1000000000LL) {
            double duration_s = (double)duration_ns / 1000000000.0;
            double distance_cm = (duration_s * 34300.0) / 2.0;
            
            if (distance_cm < 400.0) { // Limite théorique du HC-SR04
                printf("Distance : %.2f cm\n", distance_cm);
            }
        } else {
            printf("Erreur : Signal hors limite ou capteur non branché\n");
        }

        usleep(500000); // Pause entre deux mesures
    }

    gpiod_line_release(trig_line);
    gpiod_line_release(echo_line);
    gpiod_chip_close(chip);
    return 0;
}