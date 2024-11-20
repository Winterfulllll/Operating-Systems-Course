# Компилятор и флаги
CC = gcc
CFLAGS = -c -fPIC
LDFLAGS = -shared
DLFLAGS = -ldl
RM = rm -f

# Исходные файлы
GCF_SRCS = libgcf_euclid.c libgcf_naive.c
SQUARE_SRCS = libsquare_rectangle.c libsquare_triangle.c
PROGRAMS = program1 program2

# Цели
all: $(PROGRAMS)

# Динамические библиотеки
libgcfeuclid.so: libgcf_euclid.o
	$(CC) $(LDFLAGS) -o $@ $^

libgcfnaive.so: libgcf_naive.o
	$(CC) $(LDFLAGS) -o $@ $^

libsquarerectangle.so: libsquare_rectangle.o
	$(CC) $(LDFLAGS) -o $@ $^

libsquaretriangle.so: libsquare_triangle.o
	$(CC) $(LDFLAGS) -o $@ $^

# Компиляция исходников
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Сборка программ
program1: program1.c libgcfeuclid.so libsquarerectangle.so
	$(CC) -o $@ $< -L. -lgcfeuclid -lsquarerectangle

program2: program2.c libgcfeuclid.so libsquarerectangle.so
	$(CC) -o $@ $< $(DLFLAGS)

# Запуск программ
run1: program1
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program1

run2: program2
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program2

# Запуск программ с strace
srun1: program1
	env LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH strace -e trace=openat,stat,read,write,mmap,mprotect,close ./program1

srun2: program2
	env LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH strace -e trace=openat,stat,read,write,mmap,mprotect,close ./program2

# Очистка
clean:
	$(RM) *.o *.so $(PROGRAMS)
