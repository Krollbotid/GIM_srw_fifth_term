#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <bitset>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <cmath>

// Tables below exist in jutils.c, but I dont want to waste my time looking for way to include them - copy-paste is faster

const int my_jpeg_zigzag_order[DCTSIZE2] = {
   0,  1,  5,  6, 14, 15, 27, 28,
   2,  4,  7, 13, 16, 26, 29, 42,
   3,  8, 12, 17, 25, 30, 41, 43,
   9, 11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63
};

const int my_jpeg_natural_order[DCTSIZE2] = {
   0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

void to_zigzag(const JCOEFPTR in) {
    JBLOCK buf;
    for (int i = 0; i < DCTSIZE2; ++i) {
        buf[my_jpeg_zigzag_order[i]] = in[i];
    }
    for (int i = 0; i < DCTSIZE2; ++i) {
        in[i] = buf[i];
    }
    return;
}

void from_zigzag(const JCOEFPTR in) {
    JBLOCK buf;
    for (int i = 0; i < DCTSIZE2; ++i) {
        buf[my_jpeg_natural_order[i]] = in[i];
    }
    for (int i = 0; i < DCTSIZE2; ++i) {
        in[i] = buf[i];
    }
    return;
}

JCOEF find_quant_step(const JCOEFPTR arr, const size_t begin, const size_t end) { // [begin, end)
    std::unordered_map<JCOEF, int> frequencyMap;

    // Count the frequency of each element in the array
    for (int i = begin; i < end; ++i) {
        frequencyMap[arr[i]]++;
    }

    // Find the minimum frequency value
    int minFrequency = std::min_element(frequencyMap.begin(), frequencyMap.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        })->second;

    // Find all elements with the minimum frequency value
    std::vector<JCOEF> leastFrequentElements;
    for (const auto& pair : frequencyMap) {
        if (pair.second == minFrequency) {
            leastFrequentElements.push_back(pair.first);
        }
    }
    std::transform(leastFrequentElements.begin(), leastFrequentElements.end(), leastFrequentElements.begin(), [](JCOEF n) { return std::abs(n); });
    return *std::min_element(leastFrequentElements.begin(), leastFrequentElements.end());
}

void insert_by_qim(const JCOEFPTR block, const size_t len, size_t *bits_not_encoded, const std::string msg) {
    JCOEF q = find_quant_step(block, 1, DCTSIZE2 - len);
    if (q < 2) 
        q = 2;
    for (int i = DCTSIZE2 - len; i < DCTSIZE2; ++i) {
        block[i] = q * (block[i] / q) + q / 2 * (int) (msg[msg.size() - *bits_not_encoded] - '0');
        --(*bits_not_encoded);
        if (!(*bits_not_encoded))
            break;
    }
    return;
}

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

int readnChange_jpeg_file(const std::string filename, const std::string outname, const size_t len, size_t *bits_not_encoded, const std::string msg)
{
    // setup for decompressing
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
    /*
        Note about types:
        JCOEPTR is defined as JCOEF*
        JBLOCK is defined as JCOEF[DCTSIZE2]
        So in general they're equal
    */
    JCOEFPTR blockptr_one;
    jpeg_component_info* compptr_one;
	/*
    //change one dct:
    int ci = 0; // between 0 and number of image component
    int by = 0; // between 0 and compptr_one->height_in_blocks
    int bx = 0; // between 0 and compptr_one->width_in_blocks
    int bi = 0; // between 0 and 64 (8x8)
	*/
	for (int color_comp = 0; color_comp < 3; ++color_comp) { // ci 
    	compptr_one = cinfo.comp_info + color_comp;
		for (int i = 0; i < compptr_one->height_in_blocks; i++) { //by
    		buffer_one = (cinfo.mem->access_virt_barray)((j_common_ptr)&cinfo, coeffs_array[color_comp], i, (JDIMENSION)1, FALSE);
    		for (int j = 0; j < compptr_one->width_in_blocks; ++j) { //bx
    			blockptr_one = buffer_one[0][j]; // YES, left index must be 0 otherwise it gets SIGSEGV after half of rows. Idk why.
				to_zigzag(blockptr_one);
                insert_by_qim(blockptr_one, len, bits_not_encoded, msg);
                from_zigzag(blockptr_one);
                if (!(*bits_not_encoded)) {
                    goto out_of_cycles;
                }
    		}
    	}
	}
    out_of_cycles:

    write_jpeg_file(outname, cinfo, coeffs_array);

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    fclose( infile );

    return 0;
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file_base>\ne.g. %s examplein.jpg exampleout\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    std::string infilename(argv[1]), outfilename(argv[2]);

    // secret message setup
    std::string msg = "Here is Johnny!";
    std::string bmsg;
    for (char c : msg) {
        std::bitset<8> binary(c);
        bmsg += binary.to_string();
    }
    size_t bits_not_encoded;

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
        // 1 3 6 10 15 21 28
    */

    size_t lens[] = {1, 3, 6, 10, 15, 21, 28}; // amount of coefficients for inserting
    for (int k = 0; k < 7; ++k) {
        // Try reading and changing a jpeg
        bits_not_encoded = bmsg.size();
        if (readnChange_jpeg_file(infilename, outfilename + std::to_string(k) + std::string(".jpg"), lens[k], &bits_not_encoded, bmsg) == 0)
        {
            std::cout << "It's Okay... " << bits_not_encoded << "bits left not encoded." << std::endl;
        }
        else return 1;
    }
    return 0;
}