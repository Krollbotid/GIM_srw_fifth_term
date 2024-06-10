#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <jpeglib.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>
#include <cmath>
#include <fstream>

#define MAXPOP	100
#define DESIRED_FITNESS 50

void to_zigzag(const JCOEFPTR in);
void from_zigzag(const JCOEFPTR in);
JCOEF find_quant_step(const JCOEFPTR arr, const size_t begin, const size_t end);

namespace evolution {
    struct individ {
        static const int genLen = 15;
        int gene[genLen];
        double fitness;
        individ() : gene{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, fitness{0} {};
        individ(const int (&ch1)[genLen]);
        int getSum();
    };

    bool operator== (const individ &id1, const individ &id2);
    individ breed(const individ &p1, const individ &p2);
    bool IndividComparator(const individ& a, const individ& b);


    class Evolution {
    private:
        individ population[MAXPOP];
        inline static const char genStorFilename[] = "geneticStorage.txt";
    public:
        int popSave();
        int popLoad();
        double CreateFitnesses(const std::string &filename);
        int CreateNewPopulation();
        individ getGene(const int &i) { return population[i]; }
        double evolve(const std::string &filename);
    };
}