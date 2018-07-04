#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// ppm header can be ignored with this offset (bytes)
// first RGB value should be 72 44 33
#define OFFSET 15
#define BLOCKSIZE_X 4
#define BLOCKSIZE_Y 4

enum color
{
    RED,
    GRN,
    BLU
};
struct clr
{
	unsigned char R, G, B;
};

int width = 400;
int height = 202;
// the two gradients
int grad[2][3][3] = {{{-1, 0, 1},
                      {-2, 0, 2},
                      {-1, 0, 1}},
                     {{-1, -2, -1},
                      {0, 0, 0},
                      {1, 2, 1}}};

typedef char *Image;

int getPixelIndex(int x, int y, int width)
{
    return x + y * width;
}

int getSubpixelIndex(int x, int y, int width, enum color color)
{
    return getPixelIndex(x, y, width) + color;
}

unsigned char applyFilter(unsigned char *buf, int x, int y, int width, int **grad, enum color color)
{
    // you do not have to apply the filter on the first row/col and last row/col
    // since the filter needs left/right/top/bottom neighbours of a pixel
    // Iterate blockwise over the data
    int sum = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            unsigned char subpixel_index = getSubpixelIndex(x + i, y + j, width, color);
            sum += buf[subpixel_index] * grad[i + 1][j + 1];
        }
    }
    return sum;
}

void bufIntoClr(unsigned char* arg, struct clr* arg2[width*height]){
	for(int i = 0; i< width*height; i++)
	{
		(*arg2[i]).R = *(arg+i+0);
		(*arg2[i]).G = *(arg+i+1);
		(*arg2[i]).B = *(arg+i+2);
	}
}

int main()
{
    MPI_Init(NULL, NULL);
    const char *filename = "brickwall.ppm";
    const char *file_hori = "brickwall_horizontal.ppm";
    const char *file_verti = "brickwall_vertical.ppm";
    const char *header = "P6\n398 200\n255\n";
    int world_rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // size is width*height*3 because of RGB values
    unsigned int pixels = width * height * 3;
    unsigned char* buf = malloc(pixels * sizeof(unsigned char));

//    float calls = sqrt(world_size * (((float)width - 2) / (height - 2)));
//    int rows = floor(calls);
//    int cols = floor(world_size / calls);

    int block_width, block_height;
    int block_x, block_y;

    // calculate workload for each process
    // read image with MPI_File_...
    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

    // apply horizontal filter on input image
    // master writes header to image
    // write image with MPI_File_...
    int buffer_size = (width - 2) * 3;
    unsigned char line_buffer[buffer_size];
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            for (int color = RED; color <= BLU; color++)
            {
                line_buffer[(x - 1) * 3 + color] = applyFilter(buf, x, y, width, grad[0], (enum color) 	color);
            }
        }
        int offset = block_x * block_width + (block_y * block_height + y) * width;
//        MPI_File_write_at_all(file, offset, line_buffer, buffer_size, MPI_UNSIGNED_CHAR);
    }

    // apply vertical filter on input image
    // master writes header to image
    // write image with MPI_File_...
    //    for( int i = 1; i < height - 1; i++) applyFilter(i);
    //	MPI_File_write(*file, buf, 100000, MPI_UNSIGNED_CHAR, NULL);

    MPI_File_close(&file);
    free(buf);
    MPI_Finalize();
}
