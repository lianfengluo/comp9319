#include "huffman.h"

int main(int argc, char **argv){
    if (argc != 4)
        return -1;
    if (strcmp(argv[1], "-e") == 0)
        encode(argv[2], argv[3]);
    else if (strcmp(argv[1], "-d") == 0)
        decode(argv[2], argv[3]);
    else if (strcmp(argv[1], "-s") == 0)
	    search(argv[2], argv[3]);
    return 0;
}
