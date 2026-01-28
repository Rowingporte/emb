#include <gpiod.h>
#include <unistd.h>

int main() {
    struct gpiod_chip *chip = gpiod_chip_open_by_name("gpiochip4");
    struct gpiod_line *line = gpiod_chip_get_line(chip, 10);
    gpiod_line_request_output(line, "pulse", 0);

    while (1) {
        gpiod_line_set_value(line, 1);
        usleep(10); // 10 microseconds
        gpiod_line_set_value(line, 0);
        usleep(100000); // Wait 100ms
    }
    return 0;
}