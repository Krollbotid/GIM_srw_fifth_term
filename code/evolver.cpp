#include "common.h"

int main(int argc, char* argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_base>\ne.g. %s examplein\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    std::string filename(argv[1]);
    evolution::Evolution model;
    double bf = model.evolve(filename);
    for (int i = 0; i < MAXPOP; ++i) {
        evolution::individ gn = model.getGene(i);
        if (gn.fitness >= bf) {
            std::cout << "Best fitness: " << bf << std::endl << "Best gene: ";
            for (int j = 0; j < gn.genLen; ++j) {
                std::cout << gn.gene[j] << " ";
            }
            std::cout << std::endl;
            break;
        }
    }
    return 0;
}
//Best fitness: 34.0559
//Best gene: 1 1 0 1 1 1 0 0 0 1 0 1 1 1 1