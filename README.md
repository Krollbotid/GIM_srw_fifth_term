Known problems - decoder can't write \n to output :-(
Compile instructions:
    You need to install libjpeg (libjpeg-turbo) library.
    Compiling commands:
        g++ -o coder coder.cpp common.cpp -ljpeg
    Evolver uses coder and "coder" name is hardcoded in it, so you must use this name for evolver. For all other cases - feel free to change name as you like.
Usage instructions:
    PSNRResult.py - standalone script. Calculates PSNR for all encoded images. Names of these images are hardcoded (sometime I'll do something about it). Encoded images must be named [name]100.jpg (coderbest output).
    PSNRCalc.py - inner script for evolver - calculates PSNR for [name]0.jpg - [name]99.jpg
    psnrone.py - inner script for fullsolver and standalone script. Calculates PSNR for one image named [name]101.jpg (fullsolver test image). Don't use while fullsolver is active! Command:
        python psnrone [name]
to be continued
