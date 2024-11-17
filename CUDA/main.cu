#include <stdio.h>
#include <stdlib.h>

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

__global__ void calculate_histogram(uint8_t *img, size_t img_size, struct histogram_t *histogram, int channels)
{
    __shared__ int local_red[COLOR_MAX_VALUE];
    __shared__ int local_green[COLOR_MAX_VALUE];
    __shared__ int local_blue[COLOR_MAX_VALUE];

    if (threadIdx.x < COLOR_MAX_VALUE)
    {
        local_red[threadIdx.x] = 0;
        local_green[threadIdx.x] = 0;
        local_blue[threadIdx.x] = 0;
    }

    int start = blockIdx.x * blockDim.x + threadIdx.x;
    for(int i = start; i< (img_size / channels); i += blockDim.x * gridDim.x)
    {
        uint8_t R = img[i * channels];
        uint8_t G = img[i * channels + 1];
        uint8_t B = img[i * channels + 2];

        atomicAdd(&local_red[R], 1);
        atomicAdd(&local_green[G], 1);
        atomicAdd(&local_blue[B], 1);
    }

    if (threadIdx.x < COLOR_MAX_VALUE)
    {
        atomicAdd(&(histogram->red[threadIdx.x]), local_red[threadIdx.x]);
        atomicAdd(&(histogram->green[threadIdx.x]), local_green[threadIdx.x]);
        atomicAdd(&(histogram->blue[threadIdx.x]), local_blue[threadIdx.x]);
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
    float time = 0.0;
    cudaEvent_t start, stop;
    size_t img_size = 0;
    unsigned char *img = NULL;
    unsigned char *gpu_img = NULL;
    struct histogram_t histogram = {0};
    struct histogram_t *gpu_histogram;
    

    img = stbi_load(IMAGE, &width, &height, &channels, 0);
    if (!img)
    {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", 
        width, height, channels);
    img_size = width * height * channels;

    cudaEventCreate(&start);
	cudaEventCreate(&stop);

    cudaMalloc((void **)&gpu_img, img_size * sizeof(char));
	cudaMalloc((void **)&gpu_histogram, sizeof(struct histogram_t));
    cudaMemcpy(gpu_img, img, img_size * sizeof(char), cudaMemcpyHostToDevice);
	cudaMemcpy(gpu_histogram, &histogram, sizeof(struct histogram_t), cudaMemcpyHostToDevice);

    cudaEventRecord(start, 0);
    calculate_histogram<<<128, 512>>>(gpu_img, img_size, gpu_histogram, channels);
    cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&time, start, stop);
	printf("Make histogram time is %f ms\n", time);

    cudaMemcpy(&histogram, gpu_histogram, sizeof(struct histogram_t), cudaMemcpyDeviceToHost);
    save_histogram_to_csv(&histogram, "histogram.csv");

    stbi_image_free(img);
	cudaFree(gpu_img);
	cudaFree(gpu_histogram);
}
