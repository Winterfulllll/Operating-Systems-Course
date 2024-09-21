PARENT = parent
CHILD = child

all: $(PARENT) $(CHILD) output.txt

$(PARENT): parent.c
	gcc -Wall -Wextra -pedantic -Werror -g parent.c -o $(PARENT)

$(CHILD): child.c
	gcc -Wall -Wextra -pedantic -Werror -g child.c -o $(CHILD)

output.txt:
	touch output.txt

run: all
	./$(PARENT)

clean:
	rm -f $(PARENT) $(CHILD) output.*
