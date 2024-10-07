CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Werror -pthread
EXECUTABLE = merge_sort

$(EXECUTABLE): merge_sort.c
	$(CC) $(CFLAGS) -o $@ $^

run: $(EXECUTABLE)
ifndef T
	./$< threads=4
else
	./$< threads=$T
endif

clean:
	rm -f $(EXECUTABLE)
