#include "common.h"

void extract_by_qim(const JCOEFPTR block, const size_t len, size_t *bits_decoded, std::string *msg, const evolution::individ &ind) {
    JCOEF q = find_quant_step(block, 1, DCTSIZE2 - len);
    for (int i = DCTSIZE2 - len; i < DCTSIZE2; ++i) {
        if (!(ind.gene[i + len - DCTSIZE2]))
            continue;
        int b = block[i] - q * (int) floor( (float) block[i] / q); // intermediate term
        if (abs(b) < abs(b - q / 2)) {
            *msg += '0';
        }
        else {
            *msg += '1';
        }
        *bits_decoded += 1;
    }
    return;
}

int readnChange_jpeg_file(const std::string filename, const size_t len, size_t *bits_decoded, std::string *msg, const evolution::individ &ind)
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
                std::cout << std::endl;
				to_zigzag(blockptr_one);
                extract_by_qim(blockptr_one, len, bits_decoded, msg, ind);
                from_zigzag(blockptr_one);
    		}
    	}
	}

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
    std::string infilename("./encoded/");
    infilename += argv[1];
    
    // output
    std::ofstream file("out.txt");

    evolution::Evolution model;
    size_t lens[] = {model.getGene(0).genLen}; // amount of coefficients for inserting
    //uses for training
    model.popLoad();
    for (int k = 0; k < MAXPOP; ++k) {
        // secret message setup
        std::string msg;
        std::string bmsg;
        size_t bits_decoded = 0;
        // Try reading and changing a jpeg
        if (readnChange_jpeg_file(infilename + std::to_string(k) + std::string(".jpg"), lens[0], &bits_decoded, &bmsg, model.getGene(k)) == 0)
        {
            //std::cout << "It's Okay... " << bits_decoded << "bits decoded." << std::endl;
            //suppressed output
            for (int i = 0; i < bits_decoded; i += 8) {
                std::bitset<8> byte(bmsg.substr(i, 8));
                char c = static_cast<char>(byte.to_ulong());
                msg += c;
            }
            file << "Message:" << msg << std::endl;
        }
        else return 1;
    }
    return 0;
}