#include "huffman.h"

int main(int argv, char **argc){
    if (argv != 4)
        return -1;
    if (strcmp(argc[1], "-e") == 0)
        encode(argc[2], argc[3]);
    else if (strcmp(argc[1], "-d") == 0)
        decode(argc[2], argc[3]);
    else if (strcmp(argc[1], "-s") == 0)
	search(argc[2], argc[3]);
    return 0;
}
