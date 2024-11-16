#include <stdio.h>
#include <stdlib.h>
#include "my_timers.h"
#include <omp.h>

#define IMG_PATH        "../IMG/"
#define FOREST          "forest.jpg"
#define HIKING          "hiking.jpg"
#define LOTOS           "lotos.jpg"
#define DOG_WARRIOR     "piesek_wojownik.jpg"
#define DOG             "piesek.jpg"
#define SQUIRREL        "squirrel.jpg"
#define IMAGE           IMG_PATH IMAGE_CHOICE

#define IMAGE_CHOICE    DOG

#define STB_IMAGE_IMPLEMENTATION
#include "../STB/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../STB/stb_image_write.h"

#define COLOR_MAX_VALUE             256

struct histogram_t{
    int red[COLOR_MAX_VALUE];
    int green[COLOR_MAX_VALUE];
    int blue[COLOR_MAX_VALUE];
};

void calculate_histogram(uint8_t *img, size_t img_size, struct histogram_t *histogram, int channels)
{
    omp_set_num_threads(4);
    #pragma omp parallel for schedule(static)
        for(int i = 0; i< img_size; i += channels)
        {
            uint8_t R = img[i];
            uint8_t G = img[i + 1];
            uint8_t B = img[i + 2];
            histogram->red[R]++;
            histogram->green[G]++;
            histogram->blue[B]++;
        }
}

void save_histogram_to_csv(struct histogram_t *histogram, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Couldn't open file to write");
        return;
    }
    fprintf(file, "Intensity,Red,Green,Blue\n");
    for (int i = 0; i < COLOR_MAX_VALUE; i++) {
        fprintf(file, "%d,%d,%d,%d\n", i, histogram->red[i], histogram->green[i], histogram->blue[i]);
    }
    fclose(file);

    printf("Saved CSV file: %s\n", filename);
}

int main(void)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    size_t img_size = 0;
    unsigned char *img = NULL;
    struct histogram_t histogram = {0};
    
    start_time();
    img = stbi_load(IMAGE, &width, &height, &channels, 0);
    stop_time();
    print_time("Load time: ");

    if (!img)
    {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", 
        width, height, channels);

    img_size = width * height * channels;

    start_time();
    calculate_histogram(img, img_size, &histogram, channels);
    stop_time();
    print_time("Make time: ");

    start_time();
    save_histogram_to_csv(&histogram, "histogram.csv");
    stop_time();
    print_time("Saved to file in ");

    stbi_image_free(img);
}
