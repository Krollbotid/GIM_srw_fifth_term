#include "common.h"
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

void to_zigzag(const JCOEFPTR in)
{
    JBLOCK buf;
    for (int i = 0; i < DCTSIZE2; ++i) {
        buf[my_jpeg_zigzag_order[i]] = in[i];
    }
    for (int i = 0; i < DCTSIZE2; ++i) {
        in[i] = buf[i];
    }
    return;
}

void from_zigzag(const JCOEFPTR in)
{
    JBLOCK buf;
    for (int i = 0; i < DCTSIZE2; ++i) {
        buf[my_jpeg_natural_order[i]] = in[i];
    }
    for (int i = 0; i < DCTSIZE2; ++i) {
        in[i] = buf[i];
    }
    return;
}

JCOEF find_quant_step(const JCOEFPTR arr, const size_t begin, const size_t end) // [begin, end)
{
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
    JCOEF ans = *std::min_element(leastFrequentElements.begin(), leastFrequentElements.end());
    if (ans < 3)
        ans = 3;
    if (ans > 20)
        ans = 20;
    return ans;
}

namespace evolution {
    individ::individ(const int (&ch1)[genLen])
    {
        for (int i = 0; i < genLen; ++i) {
            gene[i] = ch1[i];
        }
    }

    int individ::getSum()
    {
        int sum = 0;
        for (int i = 0; i < genLen; ++i) {
            sum += gene[i];
        }
        return sum;
    }

    bool operator==(const individ &id1, const individ &id2)
    {
        for (int i = 0; i < id1.genLen; ++i) {
            if (id1.gene[i] != id2.gene[i])
                return false;
        }
        return true;
    }

    individ breed(const individ &p1, const individ &p2) 
    {
        int crossover = rand() % p1.genLen; // Create the crossover point (not first).
	    int first = rand() % 100;// Which parent comes first?

	    individ child = p1; // Child is all first parent initially.

	    int initial = 0, final = p1.genLen; // The crossover boundaries.
	    if (first < 50)
            initial = crossover;
	    else
            final = crossover + 1;
 
	    for(int i = initial; i < final; ++i) { // Crossover!
	    	child.gene[i] = p2.gene[i];
	    }
        for (int i = 0; i < child.genLen; ++i) {
	    	if (rand() % 100 < 10) {
                child.gene[i] = rand() % 2;
            }
        }

	    return child; // Return the kid...
    }

    int Evolution::popSave()
    {
        std::ofstream myfile;
        myfile.open(genStorFilename);
        for (int i = 0; i < MAXPOP; ++i) {
            for (int j = 0; j < population[i].genLen; ++j)
                myfile << population[i].gene[j] << " ";
            myfile << std::endl;
        }
        return 0;
    }

    int Evolution::popLoad()
    {
        std::ifstream myfile;
        myfile.open(genStorFilename);
        for (int i = 0; i < MAXPOP; ++i) {
            for (int j = 0; j < population[i].genLen; ++j)
                myfile >> population[i].gene[j];
        }
        return 0;
    }

    double Evolution::CreateFitnesses(const std::string &filename)
    {
        
        std::string baseCom("./coder ");
        std::string comEnd(".jpg"), comEnd2(".csv");
        std::string resultdir("./PSNR-result/");
        system((baseCom + filename).c_str());
        system("python PSNRCalc.py");
        std::ifstream myfile;
        myfile.open((resultdir + filename + comEnd2).c_str());
        double bestfitness = -1;
        for (int i = 0; i < MAXPOP; ++i) {
            myfile >> population[i].fitness;
            if (population[i].fitness > bestfitness) {
                bestfitness = population[i].fitness;
            }
        }
        return bestfitness;
    }

    bool IndividComparator(const individ& a, const individ& b)
    {
        // Compare based on Fitness value in descending order
        return a.fitness > b.fitness;
    }

    int Evolution::CreateNewPopulation()
    {
        std::vector<individ> arr(std::begin(population), std::end(population));

        // Use std::partial_sort to get the n largest elements
        std::partial_sort(arr.begin(), arr.begin() + MAXPOP / 10, arr.end(), IndividComparator);

	    for(int i = 0; i < MAXPOP / 10; ++i) {
	    	for (int j = i + 1; j < MAXPOP / 10; ++j) {
                int sum = 0;
                individ ind;
                while (ind.getSum() != 10) {
                    ind = breed(arr[i], arr[j]);
                }
                population[i * 10 + j] = ind;
            }
	    }

        for (int i = MAXPOP - MAXPOP / 10; i < MAXPOP; ++i) {
            population[i] = arr[i % 10];
        }

        return 0;
    }

    double Evolution::evolve(const std::string &filename)
    {

    	// Generate initial population.
    	srand((unsigned)time(NULL));

    	for (int i = 0; i < MAXPOP; ++i) { // Fill the population with numbers between
            while (population[i].getSum() != 10) {
    		    for (int j = 0; j < population[i].genLen; ++j) {// 0 and the result.
    		    	population[i].gene[j] = rand() % 2;
    		    }
            }
    	}

        double bestfitness = CreateFitnesses(filename);
        double oldf = bestfitness / 2;

    	int iterations = 0; // Keep record of the iterations.
        char del[50] = {'/'};
    	while ((bestfitness - oldf > (bestfitness / 1000)) && iterations < 50) { // Repeat while fitness rapidly increases and until 50 iterations.
            std::cout << del << " " << iterations << std::endl;
    		CreateNewPopulation();
            popSave();
            oldf = bestfitness;
            bestfitness = CreateFitnesses(filename);
    		++iterations;
    	}
        return bestfitness;
    }
}

