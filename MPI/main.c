#include <stdio.h>
#include <stdlib.h>
#include "my_timers.h"
#include <mpi.h>

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
#define ROOT_RANK                   0

struct histogram_t{
    int red[COLOR_MAX_VALUE];
    int green[COLOR_MAX_VALUE];
    int blue[COLOR_MAX_VALUE];
};

void calculate_histogram(uint8_t *img, size_t img_size, struct histogram_t *histogram, int channels)
{
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

int main(int argc, char** argv)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    int rank = 0;
    int world_size = 0;
    size_t img_size = 0;
    size_t sub_domain_len = 0;
    unsigned char *img = NULL;
    unsigned char *tmp_img = NULL;
    struct histogram_t histogram = {0};
    struct histogram_t tmp_histogram = {0};

    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    printf("Assigned rank: %d\n", rank);
    
    if (ROOT_RANK == rank)
    {
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
    }

    // Send variables to others
    MPI_Bcast(&img_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (ROOT_RANK == rank)
        start_time();
    sub_domain_len = img_size / world_size;
    MPI_Bcast(&sub_domain_len , 1, MPI_INT, 0, MPI_COMM_WORLD);
    tmp_img = (unsigned char *) malloc(sub_domain_len * sizeof(char));
	/// Scatter the initial dataset among the processes
	MPI_Scatter(img, sub_domain_len, MPI_CHAR,
			tmp_img, sub_domain_len, MPI_CHAR,
			0, MPI_COMM_WORLD);


    calculate_histogram(tmp_img, sub_domain_len, &tmp_histogram, channels);

    MPI_Reduce(tmp_histogram.red, histogram.red, COLOR_MAX_VALUE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(tmp_histogram.green, histogram.green, COLOR_MAX_VALUE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(tmp_histogram.blue, histogram.blue, COLOR_MAX_VALUE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (ROOT_RANK == rank)
    {
        stop_time();
        print_time("Make time: ");

        start_time();
        save_histogram_to_csv(&histogram, "histogram.csv");
        stop_time();
        print_time("Saved to file in ");

        stbi_image_free(img);
    }

    MPI_Finalize();

    
}
