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

void to_zigzag(const JCOEFPTR in);
void from_zigzag(const JCOEFPTR in);
JCOEF find_quant_step(const JCOEFPTR arr, const size_t begin, const size_t end);
