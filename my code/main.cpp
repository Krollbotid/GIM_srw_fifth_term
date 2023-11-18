#include <iostream>
#include <jpeglib.h>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <jpeglib.h>

void decompress_jpeg(const char* input_filename, const char* output_filename) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* input_file;
    FILE* output_file;
    JSAMPARRAY buffer;
    int row_stride;

    // Initialize the JPEG decompression object and error handler
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Open the input file
    if ((input_file = fopen(input_filename, "rb")) == NULL) {
        fprintf(stderr, "Error opening input file\n");
        exit(1);
    }

    // Specify the source of JPEG data
    jpeg_stdio_src(&cinfo, input_file);

    // Read the JPEG header
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        fprintf(stderr, "Invalid JPEG file\n");
        exit(1);
    }

    // Start the decompression process
    jpeg_start_decompress(&cinfo);

    // Open the output file
    if ((output_file = fopen(output_filename, "wb")) == NULL) {
        fprintf(stderr, "Error opening output file\n");
        exit(1);
    }

    // Allocate memory for the image buffer
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

    // Set the row_stride for the output file
    row_stride = cinfo.output_width * cinfo.output_components;

    // Decompress and write the image to the output file
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        fwrite(buffer[0], row_stride, 1, output_file);
    }

    // Finish decompression
    jpeg_finish_decompress(&cinfo);

    // Clean up
    jpeg_destroy_decompress(&cinfo);
    fclose(input_file);
    fclose(output_file);
}

int main(int agrc, char* argv[]) {
    const char* input_filename = "bub.jpg";
    const char* output_filename = "bubout.jpg";

    decompress_jpeg(input_filename, output_filename);

    printf("JPEG decompression complete.\n");

    return 0;
}