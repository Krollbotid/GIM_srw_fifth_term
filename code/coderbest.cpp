#include "common.h"

void insert_by_qim(const JCOEFPTR block, const size_t len, size_t *bits_not_encoded, const std::string msg, const evolution::individ &ind) {
    JCOEF q = find_quant_step(block, 1, DCTSIZE2 - len);
    for (int i = DCTSIZE2 - len; i < DCTSIZE2; ++i) {
        if (!(ind.gene[i + len - DCTSIZE2]))
            continue;
        char c = msg[msg.size() - *bits_not_encoded] - '0';
        JCOEF sec_bit = (JCOEF) c;
        block[i] = q * (int) floor( (float) block[i] / q) + q * sec_bit / 2;
        *bits_not_encoded -= 1;
        if (*bits_not_encoded == 0)
            break;
    }
    return;
}

int write_jpeg_file(std::string outname, jpeg_decompress_struct in_cinfo, jvirt_barray_ptr *coeffs_array)
{

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

int readnChange_jpeg_file(const std::string filename, const std::string outname, const size_t len, size_t *bits_not_encoded, const std::string msg, const evolution::individ &ind)
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
                insert_by_qim(blockptr_one, len, bits_not_encoded, msg, ind);
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
	if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_base>\ne.g. %s examplein\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    std::string infilename("./source/"), outfilename("./encoded/");
    infilename += argv[1];
    outfilename += argv[1];

    // secret message setup
    std::ifstream file("info.txt");
    std::string msg;

    if (file.is_open()) {
        std::string line;
        while (file >> line) {
            msg += line + " ";
        }
        file.close();
    } else {
        std::cerr << "Error: Unable to open the file." << std::endl;
    }

    std::string bmsg;
    for (char c : msg) {
        std::bitset<8> binary(c);
        bmsg += binary.to_string();
    }
    size_t bits_not_encoded;

    evolution::individ bestind;
    int bestgene[] = {1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1};
    for (int i = 0; i < bestind.genLen; ++i) {
        bestind.gene[i] = bestgene[i];
    }
    for (int k = MAXPOP; k < MAXPOP + 1; ++k) {
        // Try reading and changing a jpeg
        bits_not_encoded = bmsg.size();
        if (readnChange_jpeg_file(infilename + std::string(".jpg"), outfilename + std::to_string(k) + std::string(".jpg"), bestind.genLen, &bits_not_encoded, bmsg, bestind) == 0)
        {
            std::cout << "It's Okay... Gene #" << k << " " << bits_not_encoded << "bits left not encoded." << std::endl;
        }
        else return 1;
    }
    return 0;
}