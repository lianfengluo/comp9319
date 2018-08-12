CC = gcc
# CFLAGS = -Wall -Werror -Wextra -g -std=c99
CFLAGS = -Wall -Werror -Wextra -O3 -std=c99
ODIR = ./
huffman: huffman_encode.o huffman_decode.o huffman_search.o huffman.c
	$(CC) $(CFLAGS) -o $@ $^

huffman_decode.o: huffman_decode.c
	$(CC) $(CFLAGS) -c $< -o $@

huffman_encode.o: huffman_encode.c
	$(CC) $(CFLAGS) -c $< -o $@

huffman_search.o: huffman_search.c
	$(CC) $(CFLAGS) -c $< -o $@
# huffman_search.o: huffman_bm_search.c
# 	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(ODIR)/*.o