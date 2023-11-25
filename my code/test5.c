#include <stdio.h>
#include <jpeglib.h>

void print_dct_blocks(const char *filename) {
    struct jpeg_decompress_struct cinfo;  // JPEG decompression struct
    struct jpeg_error_mgr jerr;  // Error handler

    FILE *infile;
    if ((infile = fopen(filename, "rb")) == NULL) {  // Open the JPEG file
        fprintf(stderr, "Error opening file\n");
        return;
    }

    cinfo.err = jpeg_std_error(&jerr);  // Initialize the error handler
    jpeg_create_decompress(&cinfo);  // Initialize the JPEG decompression object
    jpeg_stdio_src(&cinfo, infile);  // Specify the input file
    jpeg_read_header(&cinfo, TRUE);  // Read the file header
    jpeg_start_decompress(&cinfo);  // Start the decompression process

    JDIMENSION MCU_width = cinfo.comp_info[0].width_in_blocks;  // MCU width
    JDIMENSION MCU_height = cinfo.comp_info[0].height_in_blocks;  // MCU height

    JBLOCKARRAY buffer;  // Array for DCT coefficients
    buffer = (cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, MCU_width * MCU_height * DCTSIZE2, 1);  // Allocate memory for DCT coefficients

    int block_row;
    while (cinfo.output_scanline < cinfo.output_height) {  // Iterate through the scanlines
        jpeg_read_raw_data(&cinfo, buffer, MCU_height);  // Read raw DCT coefficients
        for (block_row = 0; block_row < MCU_height; block_row++) {
            JCOEFPTR block = buffer[0] + block_row * MCU_width * DCTSIZE2;  // Pointer to the current block
            for (int i = 0; i < MCU_width; i++) {  // Iterate through the blocks in the row
                for (int j = 0; j < DCTSIZE2; j++) {  // Iterate through the DCT coefficients in the block
                    printf("%d ", block[i * DCTSIZE2 + j]);  // Print the DCT coefficient
                }
                printf("\n");  // New line for the next row of coefficients
            }
            printf("\n");  // Print an extra newline between blocks
        }
    }

    jpeg_finish_decompress(&cinfo);  // Finish the decompression
    jpeg_destroy_decompress(&cinfo);  // Clean up the decompression object
    fclose(infile);  // Close the file
}

int main(int argc, char *argv[]) {
    if (argc < 2) {  // Check for command-line argument
        printf("Usage: %s <jpeg_file_name>\n", argv[0]);
        return 1;  // Exit with error
    }

    const char *filename = argv[1];
    print_dct_blocks(filename);  // Call the function to print DCT coefficients

    return 0;  // Exit successfully
}