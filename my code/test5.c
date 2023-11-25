#include <stdio.h>
#include <jpeglib.h>

void print_dct_blocks(const char *filename) {
    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) {
        fprintf(stderr, "Error opening file\n");
        return;
    }

    // Open the input file for reading in binary mode.

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    // Set up JPEG decompression structure, error handling, and start decompression process.

    JDIMENSION MCU_width = cinfo.comp_info[0].width_in_blocks;
    JDIMENSION MCU_height = cinfo.comp_info[0].height_in_blocks;

    // Retrieve dimensions of Minimum Coded Unit (MCU) in width and height.

    JDIMENSION block_row;
    while (cinfo.output_scanline < cinfo.output_height) {
        JBLOCKARRAY buffer = (*cinfo.mem->alloc_barray)((j_common_ptr) &cinfo, JPOOL_IMAGE, MCU_width * DCTSIZE2);
        jpeg_read_raw_data(&cinfo, buffer, MCU_height);
        for (block_row = 0; block_row < MCU_height; block_row++) {
            JCOEFPTR block = buffer[0][block_row];
            for (JDIMENSION i = 0; i < MCU_width; i++) {
                for (JDIMENSION j = 0; j < DCTSIZE2; j++) {
                    printf("%d ", block[i * DCTSIZE2 + j]);
                }
                printf("\n");
            }
            printf("\n");
        }
        (*cinfo.mem->free_barray)((j_common_ptr) &cinfo, buffer, MCU_height);
    }

    // Loop through the image data, reading the raw data and printing DCT blocks.

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    // Finish decompression process and release decompression structure.
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <jpeg_file_name>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    print_dct_blocks(filename);
    return 0;
}

// Main function to check for command-line argument and call the print_dct_blocks function.
