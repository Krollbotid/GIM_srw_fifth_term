#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <iostream>
#include <string>    

int write_jpeg_file(std::string outname, jpeg_decompress_struct in_cinfo, jvirt_barray_ptr *coeffs_array){

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * infile;

    if ((infile = fopen(outname.c_str(), "wb")) == NULL) {
    	fprintf(stderr, "can't open %s\n", outname.c_str());
    	return 1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, infile);

    j_compress_ptr cinfo_ptr = &cinfo;
    jpeg_copy_critical_parameters((j_decompress_ptr)&in_cinfo, cinfo_ptr);
    jpeg_write_coefficients(cinfo_ptr, coeffs_array);

    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );
    fclose( infile );

    return 0;
}

int read_jpeg_file(std::string filename, std::string outname)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * infile;

    if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
    	fprintf(stderr, "can't open %s\n", filename.c_str());
    	return 1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);


    jvirt_barray_ptr *coeffs_array = jpeg_read_coefficients(&cinfo);

    //change one dct:
    int ci = 0; // between 0 and number of image component
    int by = 0; // between 0 and compptr_one->height_in_blocks
    int bx = 0; // between 0 and compptr_one->width_in_blocks
    int bi = 0; // between 0 and 64 (8x8)
    JBLOCKARRAY buffer_one;
    JCOEFPTR blockptr_one;
    jpeg_component_info* compptr_one;
    compptr_one = cinfo.comp_info + ci;
    buffer_one = (cinfo.mem->access_virt_barray)((j_common_ptr)&cinfo, coeffs_array[ci], compptr_one->height_in_blocks, (JDIMENSION)1, FALSE);
	for (int i = 0; i < compptr_one->height_in_blocks; i++) { //by
    	for (int j = 0; j < compptr_one->width_in_blocks; j++) { //bx
    		blockptr_one = buffer_one[i][j];
			printf("Block %d row, %d column from %d rows %d columns\n", i, j, compptr_one->height_in_blocks, compptr_one->width_in_blocks)
			for (int k = 0; k < 64; k += l) {
				for (int k = 0; k < 8 + l; k++) {
					printf("%d ", blockptr_one[k]);
					blockptr_one[k]++;
				}
				printf("\n");
			}
    	}
    }

    write_jpeg_file(outname, cinfo, coeffs_array);

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    fclose( infile );

    return 0;
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    std::string infilename(argv[1]), outfilename(argv[2]);

    /* Try opening a jpeg*/
    if (read_jpeg_file(infilename, outfilename) == 0)
    {
        std::cout << "It's Okay..." << std::endl;
    }
    else return 1;
    return 0;
}