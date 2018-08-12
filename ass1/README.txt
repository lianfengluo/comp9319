Author: Lianfeng Luo
Student id: z5170521
Programming language C
The program is sperated into 3 parts:
1. encode:
    First, we need to count the frequency of each character. After that, we reorder them base on the frequency, and trime it to the size than is only as large as the character in the orginal text. And build the tree basing on the two least freqent character. And build tree base on that, and in the meanwhile, we use the deep first search to record the structure of the tree, which can be use to retrieve that coded tree. Create a dictionary that map the character to the corresponsed code. Just write the encoded file, and try to padding the bits that not as big as 8 bits.
2. decode:
    Read the text that we put in the encoded file and base on the dfs to retrieve the tree. Then, I just use the tree structure to decode the character. Stop when we reach the orginal size.
3. search:
    Repeat several steps that what we did in the decode stage and after we get the tree. Just place the decoded character to the same length buff as the searching pattern. And just match the pattern from the end to the beginning. Because it search from right to left, we can think of BM search method will do such a job. Therefore, I build a BM table to do such a job. Then, just count the number of matches.