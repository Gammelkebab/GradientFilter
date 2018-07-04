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

int width = 400;
int height = 202;
// the two gradients
int grad[2][3][3] = {{{-1, 0, 1},
                      {-2, 0, 2},
                      {-1, 0, 1}},
                     {{-1, -2, -1},
                      {0, 0, 0},
                      {1, 2, 1}}};

int applyFilter(unsigned char sqr[3][3], int mat[3][3])
{
    // you do not have to apply the filter on the first row/col and last row/col
    // since the filter needs left/right/top/bottom neighbours of a pixel
    // Iterate blockwise over the data
    int tmp = 0 for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++)
        tmp += sqr[i][j] * mat[i][j];
    return tmp;
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
    unsigned char *buf = malloc(pixels * sizeof(unsigned char));

    unsigned float calls = sqrt(world_size * (((float)width - 2) / (height - 2)));
    int rows = floor(calls);
    int cols = floor(world_size / calls);

    // calculate workload for each process
    // read image with MPI_File_...
    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    // apply horizontal filter on input image
    // master writes header to image
    // write image with MPI_File_...
    for (int i = 1; i < width - 1; i++)      //full image width
        for (int j = 1; j < height - 1; j++) // full image height
        {
            unsigned char tmp[3][3][3]; // 3x3 square
            for (int k = -1; k < 2; k++)
            { // width
                for (int h = -1; h < 2; h++)
                { // height
                    for (int p = 0; p < 3; p++)
                    { // RGB
                        tmp[k + 1][h + 1][p] = buf[((j + h) * width + (i + k)) * 3 + p];
                        applyFilter(tmp[red], grad[0]);
                        applyFilter(tmp[green], grad[0]);
                        applyFilter(tmp[blue], grad[0]);
                    }
                }
            }
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
