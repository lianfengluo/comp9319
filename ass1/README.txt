Author: Lianfeng Luo
Student id: z5170521
Programming language C
Static huffman encode, decode and search in the encoded file

Usage:
1. Compile
    make
2. encode
    ./huffman -e (input file name) (encoded file name)
3. decode
    ./huffman -d (encoded file name) (output file name)
4. search
    ./huffman -s (search pattern) (encoded file name)


The program is sperated into 3 parts:
1. encode:
    First, I need to count the frequency of each character. After that, I reorder them base on the frequency, and trime it to the size than is only as large as the character in the orginal text. And build the tree basing on the two least freqent character. And build tree base on that, and in the meanwhile, I use the deep first search to record the structure of the tree, which can be use to retrieve that coded tree. Create a dictionary that map the character to the corresponsed code. Just write the encoded file, and try to padding the bits that not as big as 8 bits.
2. decode:
    Read the text that I put in the encoded file and base on the dfs to retrieve the tree. Then, I just use the tree structure to decode the character. Stop when I reach the orginal size.
3. search:
    Repeat several steps that what I did in the decode stage and after I get the tree. Just place the decoded character to the same length buff as the searching pattern. And just match the pattern from the end to the beginning. Because it search from right to left, I can think of BM search method will do such a job. Therefore, I build a BM table to do such a job. The normal BM algorithm(only with bad matching table) is not as good as the BM algorthm with the good suffix table. I read some article online about the way to build the good suffix table that can help me to avoid rematch the word that I already match and try to align the substring with the string that have already match. When the bad matching table preform the better job I use the bad matching table. Otherwise, I use the good suffix table. Then, just take the larger number as offset to skip the matching. Finally, return the matching count.