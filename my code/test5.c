#include <stdio.h>
#include <jpeglib.h>

void print_dct_blocks(const char *filename) {
    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) {
        fprintf(stderr, "Error opening file\n");
        return;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    JDIMENSION MCU_width = cinfo.comp_info[0].width_in_blocks;
    JDIMENSION MCU_height = cinfo.comp_info[0].height_in_blocks;

    JDIMENSION block_row;
    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, MCU_width * DCTSIZE2, MCU_height);
        jpeg_read_raw_data(&cinfo, &buffer, MCU_height);
        for (block_row = 0; block_row < MCU_height; block_row++) {
            JSAMPROW row = buffer[block_row];
            for (JDIMENSION i = 0; i < MCU_width; i++) {
                for (JDIMENSION j = 0; j < DCTSIZE2; j++) {
                    printf("%d ", row[i * DCTSIZE2 + j]);
                }
                printf("\n");
            }
            printf("\n");
        }
        (*cinfo.mem->free_pool)((j_common_ptr) &cinfo, JPOOL_IMAGE);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
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
