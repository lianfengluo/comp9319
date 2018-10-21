Student ID: z5170521
Name: Lianfeng(Richard) Luo

Project: BWT encode and search


Space requirement: 
    Encode:
    Hard disk usage for temporary file less than 10 times of orginal text;
    memory usage less than 250MB.
    Search:
    Hard disk usage for index file less than orginal text;
    memory usage less than 15MB
Time：
    Encode:
    Less than 300s.
    Search:
    Less than 60s at the first time and 10s after the first time.

Usage:

Encode:
Command: ./bwtencode delimiter tmp_folder orginal_file_path bwt_file_path
                    delimiter is the character '\n' or acsii from 32 to 126.
                    tmp_foler is the folder that to be used to store index.
                    orginal_file is the orginal file path
                    bwt_file_path is the output path of encoded bwt file.

Search:
Command: ./bwtsearch delimiter bwt_file tmp_folder flag pattern
                    delimiter is the character '\n' or acsii from 32 to 126.
                    bwt_file is the encoded bwt file.
                    tmp_foler is the folder that to be used to store index.
                    flag: -i, -m, -n, -a.
                    pattern:
                    -i : range. i.e. '10 20' to retrieve the record that id which is 10 to 20.
                    -m : search pattern. Count how many match in orginal text with duplication.
                    -n : search pattern. Count how many match in record in orginal text without duplication.
                    -a : search pattern. Display the record id that match orginal text.

Concept: 
    Encode
    Use the extended version of bwt. 
        What I am doing is not the linear time version.
        What I am doing is the mutational version of sorting string.
        Because this assignment is base on the delimiter, We can use this feature to speed up our sort.
        I make the delimiter is ordered based on the orginal text. 
        After that, I use merge sort to sort the text. Because it's not record is unreasonably long(5000+ long). It means the suffix comparsion will not longer than 5000(which is a constant number), because I can always decide which string is sorted first and when it comes to the delimiter.
        The complexity of the algorithm will be <= 5000*nlog(n) which is feasible in the given condition.
        I am not just merge it all at once. I bucket them in alphabetic order and seperate character into each bucket and sort each bucket(skip the delimiter). After sorting those, I merge them together.
        Then, I will get the sort suffix array.
        After the create the suffix array, I also create the an array to keep track on the corresponding positional id to the bwt file I just created.
        Because of the memory requirement I am separate it into two situation,
        1. 26.5MB or less which can allow me to use 9 times of memory space besides the start up space.
        2. Otherwise, I have to do above job in the disk.

    Search:
        Basically, I will set up three circumstances in each search.(I calculate the c start memory is about 2.3MB in the cse machine)
        First(small circumstance about 6MB, range flage will be 6MB or less in bwt file), I can load the occurrence table and the whole text into the 15MB memory.
        Second(medium circumstance about 12MB range flag will be 6~12MB or less in bwt file), I can load the whole text into 15MB memory.
        Third, I only can do suff by using fseek on the occurrence table, and loading about 12.5MB in the memory.
        I will store is the c_table and the occurrence index.
        I will not use store the character. I just store the character that is appeared in the text(this can be retrieved by use the counting table), and use mapping table to downsize the space. Therefore, we can have more record in the shorter step(which will have more index). I will dynamicaly generate the step size base on the orginal file size and the number of character in the orginal text.