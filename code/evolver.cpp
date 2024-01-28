#include "common.h"

int main(int argc, char* argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_base>\ne.g. %s examplein\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    std::string filename(argv[1]);
    evolution::Evolution model;
    int ans = model.evolve(filename);
    if (ans == -1) {
        std::cout << "No solution found." << std::endl;
    } else {
        evolution::individ gn = model.getGene(ans);
        for (int i = 0; i < gn.genLen; ++i)
            std::cout << gn.gene[i] << " ";
        std::cout << std::endl;
    }
    return 0;
}