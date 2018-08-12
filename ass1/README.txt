Author: Lianfeng Luo
Student id: z5170521
Programming language C
The program is sperated into 3 parts:
1. encode:
    First, we need to count the frequency of each character. After that, we reorder them base on the frequency, and trime it to the size than is only as large as the character in the orginal text. And build the tree basing on the two least freqent character. And build tree base on that, and in the meanwhile, we use the deep first search to record the structure of the tree, which can be use to retrieve that coded tree. Create a dictionary that map the character to the corresponsed code. Just write the encoded file, and try to padding the bits that not as big as 8 bits.
2. decode:
    Read the text that we put in the encoded file and base on the dfs to retrieve the tree. Then, we just store the "0" and "1" code from the encoded file to a buff and decode to chararcter. Stop when we reach the orginal size. Of course, it has to be a buff that is big enough to get the possible max len which is about 254.
3. search:
    Repeat several steps that what we did in the decode stage and after we get the tree. Just place the decoded character to the same length buff as the searching pattern. And just match the pattern from the end to the beginning(I actually try to implement BM, But it seems in most of the case it will a bit slower than the dummy way, and I think this is because I am not implementing it in a right method). Then, just count the number of matches.