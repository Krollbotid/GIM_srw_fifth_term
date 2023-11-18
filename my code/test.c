#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

void process_JPEG_file(const char *input_filename) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jvirt_barray_ptr *coefs;
    FILE *input_file;
    
    if ((input_file = fopen(input_filename, "rb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", input_filename);
        exit(EXIT_FAILURE);
    }
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    
    jpeg_stdio_src(&cinfo, input_file);
    
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        fprintf(stderr, "Invalid JPEG file: %s\n", input_filename);
        fclose(input_file);
        jpeg_destroy_decompress(&cinfo);
        exit(EXIT_FAILURE);
    }
    
    coefs = jpeg_read_coefficients(&cinfo);
    
    for (int ci = 0; ci < cinfo.num_components; ci++) {
        JBLOCKARRAY buf = (*cinfo.mem->access_virt_barray)((j_common_ptr)&cinfo, coefs[ci], 0, cinfo.comp_info[ci].height_in_blocks, TRUE);
        
        for (int bi = 0; bi < cinfo.comp_info[ci].height_in_blocks; bi++) {
            JCOEFPTR dataptr = buf[bi][0];
            
            for (int bj = 0; bj < cinfo.comp_info[ci].width_in_blocks; bj++) {
                // Do something with the dequantized DCT coefficients
                // For example: Print the coefficients
                for (int k = 0; k < DCTSIZE2; k++) {
                    printf("%d ", dataptr[k]);
                }
                printf("\n");
                dataptr += DCTSIZE;
            }
        }
    }
    
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(input_file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input.jpg>\n", argv[0]);
        return 1;
    }
    
    const char *input_filename = argv[1];
    
    process_JPEG_file(input_filename);
    
    return 0;
}