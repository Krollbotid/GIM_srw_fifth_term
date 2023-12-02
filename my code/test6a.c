#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>

// Define the 8x8 matrix A
int A[8][8] = {
    {5, 4, 6, 8, 1, 1, 7, 9},
    {1, 7, 7, 8, 8, 5, 2, 6},
    {8, 7, 0, 0, 6, 0, 6, 4},
    {2, 6, 4, 1, 7, 2, 3, 3},
    {1, 3, 2, 3, 6, 0, 8, 5},
    {6, 2, 3, 9, 2, 7, 5, 7},
    {2, 7, 5, 1, 7, 2, 5, 2},
    {3, 7, 4, 0, 8, 5, 2, 5}
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    FILE *input_file = fopen(argv[1], "rb");
    if (!input_file) {
        fprintf(stderr, "Error opening file for reading.\n");
        exit(EXIT_FAILURE);
    }
    jpeg_stdio_src(&cinfo, input_file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    // Create a new JPEG compression object for the output file
    struct jpeg_compress_struct cinfo_dest;
    struct jpeg_error_mgr jerr_dest;
    
    cinfo_dest.err = jpeg_std_error(&jerr_dest);
    jpeg_create_compress(&cinfo_dest);
    
    FILE *output_file = fopen(argv[2], "wb");
    if (!output_file) {
        fprintf(stderr, "Error opening file for writing.\n");
        exit(EXIT_FAILURE);
    }
    jpeg_stdio_dest(&cinfo_dest, output_file);

    cinfo_dest.image_width = cinfo.image_width;
    cinfo_dest.image_height = cinfo.image_height;
    cinfo_dest.input_components = cinfo.output_components;
    cinfo_dest.in_color_space = cinfo.out_color_space;

    jpeg_set_defaults(&cinfo_dest);
    jpeg_set_quality(&cinfo_dest, 75, TRUE);
    jpeg_start_compress(&cinfo_dest);

    // Process each DCT block and write to the new JPEG file
    JDIMENSION block_width = cinfo.output_width / 8;
    JDIMENSION block_height = cinfo.output_height / 8;

    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPARRAY buffer = calloc(64, sizeof(JSAMPLE));
        jpeg_read_scanlines(&cinfo, buffer, 8);

        // Apply the matrix A to the DCT coefficients
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                buffer[i][j] += A[i][j];
            }
        }

        // Write the modified DCT coefficients to the new JPEG file
        jpeg_write_scanlines(&cinfo_dest, buffer, 8);
        free(buffer);
    }

    // Finish writing the new JPEG file
    jpeg_finish_compress(&cinfo_dest);
    fclose(output_file);
    jpeg_destroy_compress(&cinfo_dest);

    // Finish processing the original JPEG file
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(input_file);

    return 0;
}
