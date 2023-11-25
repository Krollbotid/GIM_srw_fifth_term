// work with dequantized coefs.
#include <stdio.h>
#include <jpeglib.h>

int main(int argc, char *argv[]) {
    // Step 1: Declare the JPEG decompression object and error manager
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // Step 2: Initialize the JPEG decompression object error manager
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Step 3: Specify the input file
    const char *input_file = "input.jpg";
    FILE *infile = fopen(input_file, "rb");
    if (!infile) {
        fprintf(stderr, "Error opening file %s\n", input_file);
        return 1;
    }

    // Step 4: Initialize the JPEG decompression object with the input file
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    // Step 5: Read and dequantize the DCT coefficients for each component
    JDIMENSION MCU_width = cinfo.output_width / cinfo.max_h_samp_factor;
    JDIMENSION MCU_height = cinfo.output_height / cinfo.max_v_samp_factor;
    JBLOCKARRAY buffer;
    JCOEFPTR blockptr;

    while (cinfo.output_scanline < cinfo.output_height) {
        buffer = (cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, MCU_width * DCTSIZE, MCU_height * DCTSIZE);
        jpeg_read_raw_data(&cinfo, buffer, MCU_height * DCTSIZE);
        for (int ci = 0; ci < cinfo.num_components; ci++) {
            jpeg_component_info *compptr = &cinfo.comp_info[ci];
            for (JDIMENSION bi = 0; bi < MCU_height; bi++) {
                for (JDIMENSION bj = 0; bj < MCU_width; bj++) {
                    blockptr = buffer[bi * MCU_width + bj] + ci * DCTSIZE2;
                    for (int y = 0; y < DCTSIZE; y++) {
                        for (int x = 0; x < DCTSIZE; x++) {
                            blockptr[y * DCTSIZE + x] *= cinfo.quant_tbl_ptrs[compptr->quant_tbl_no]->quantval[y * DCTSIZE + x]; // Dequantize coefficient
                            printf("%d ", blockptr[y * DCTSIZE + x]); // Print dequantized coefficient
                        }
                        printf("\n");
                    }
                    printf("\n");
                }
            }
        }
    }

    // Step 6: Finish decompression and clean up
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
}
/*In this modified code, we iterate through each component and dequantize its DCT coefficients using the corresponding quantization table.
The dequantized coefficients for each component are then printed to the console.
*/