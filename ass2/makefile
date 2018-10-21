CC = gcc
# CFLAGS = -Wall -Wextra -g -std=c99
CFLAGS = -std=c99 -O3 -Wall -Wextra
ODIR = ./
all: bwtsearch bwtencode

bwtencode: bwtencode.c encoder.o
	$(CC) $(CFLAGS) $^ -o $@ 


encoder.o: encoder.c
	$(CC) -c $< -o $@ $(CFLAGS) 

bwtsearch: bwtsearch.c search_count_uniq.o search_range.o search_count_duplicate.o search_exist.o
	$(CC) $(CFLAGS) -o $@ $^


search_count_uniq.o: search_count_uniq.c
	$(CC) $(CFLAGS) -c $< -o $@

search_count_duplicate.o: search_count_duplicate.c
	$(CC) $(CFLAGS) -c $< -o $@

search_exist.o: search_exist.c
	$(CC) $(CFLAGS) -c $< -o $@

search_range.o: search_range.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(ODIR)/*.o
