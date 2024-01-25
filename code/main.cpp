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

int readnChange_jpeg_file(std::string filename, std::string outname, int** A)
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
    JBLOCKARRAY buffer_one;
    JCOEFPTR blockptr_one;
    jpeg_component_info* compptr_one;
	/*
    //change one dct:
    int ci = 0; // between 0 and number of image component
    int by = 0; // between 0 and compptr_one->height_in_blocks
    int bx = 0; // between 0 and compptr_one->width_in_blocks
    int bi = 0; // between 0 and 64 (8x8)
	*/
	for (int n = 0; n < 3; n++) { // ci
    	compptr_one = cinfo.comp_info + n;
		for (int i = 0; i < compptr_one->height_in_blocks; i++) { //by
    		buffer_one = (cinfo.mem->access_virt_barray)((j_common_ptr)&cinfo, coeffs_array[n], i, (JDIMENSION)1, FALSE);
    		for (int j = 0; j < compptr_one->width_in_blocks; j++) { //bx
    			blockptr_one = buffer_one[0][j]; // YES, left index must be 0 otherwise it gets SIGSEGV after half of rows. Idk why.
				printf("Block %d row, %d column from %d rows %d columns\n", i, j, compptr_one->height_in_blocks, compptr_one->width_in_blocks);
				for (int l = 0; l < 64; l += 8) {
					for (int k = l; k < 8 + l; k++) { // bi = k + l
						printf("%d ", blockptr_one[k]);
						blockptr_one[k] += A[l][k]; // modifying each coefficient
					}
					printf("\n");
				}
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
        fprintf(stderr, "Usage: %s <input_file> <output_file_base>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    std::string infilename(argv[1]), outfilename(argv[2]);

    /*
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
        0   1   2   3   4   5   6   7
        1   2   3   4   5   6   7   8
        2   3   4   5   6   7   8   9
        3   4   5   6   7   8   9   10
        4   5   6   7   8   9   10  11
        5   6   7   8   9   10  11  12
        6   7   8   9   10  11  12  13
        7   8   9   10  11  12  13  14
        // 1 3 6 10 15 21 28 34
    */
    
    for (k = 0; k < 7; ++k) {
        int A[8][8] = {0};
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (i + j >= 14 - k) {
                    A[i][j] = 50;
                }
            }
        }

        /* Try reading and changing a jpeg*/
        if (readnChange_jpeg_file(infilename, outfilename + std::to_string(k), A) == 0)
        {
            std::cout << "It's Okay..." << std::endl;
        }
        else return 1;
    }
    return 0;
}