# Исследование зависимости ускорения и эффективности алгоритма от входных данных и количества потоков

## Для 100.000.000 элементов в массиве

```bash
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=1
gcc -Wall -Wextra -pedantic -Werror -pthread -o merge_sort merge_sort.c
./merge_sort threads=1
Elapsed time: 27.844674 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=2
./merge_sort threads=2
Elapsed time: 14.010450 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=3
./merge_sort threads=3
Elapsed time: 9.905401 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=4
./merge_sort threads=4
Elapsed time: 7.725247 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=5
./merge_sort threads=5
Elapsed time: 6.967299 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=6
./merge_sort threads=6
Elapsed time: 6.358381 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=7
./merge_sort threads=7
Elapsed time: 5.896012 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=8
./merge_sort threads=8
Elapsed time: 5.871150 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=9
./merge_sort threads=9
Elapsed time: 5.861945 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=20
./merge_sort threads=20
Elapsed time: 7.195837 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=100
./merge_sort threads=100
Elapsed time: 27.915806 seconds
```

## Для 100.000 элементов в массиве

```bash
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=1
gcc -Wall -Wextra -pedantic -Werror -pthread -o merge_sort merge_sort.c
./merge_sort threads=1
Elapsed time: 0.023757 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=2
./merge_sort threads=2
Elapsed time: 0.014504 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=3
./merge_sort threads=3
Elapsed time: 0.009373 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=4
./merge_sort threads=4
Elapsed time: 0.008167 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=5
./merge_sort threads=5
Elapsed time: 0.007273 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=6
./merge_sort threads=6
Elapsed time: 0.007985 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=7
./merge_sort threads=7
Elapsed time: 0.007949 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=8
./merge_sort threads=8
Elapsed time: 0.008194 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=9
./merge_sort threads=9
Elapsed time: 0.008357 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=20
./merge_sort threads=20
Elapsed time: 0.013573 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=100
./merge_sort threads=100
Elapsed time: 0.036378 seconds
```

## Для 100 элементов в массиве

```bash
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=1
gcc -Wall -Wextra -pedantic -Werror -pthread -o merge_sort merge_sort.c
./merge_sort threads=1
Elapsed time: 0.000172 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=2
./merge_sort threads=2
Elapsed time: 0.000247 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=3
./merge_sort threads=3
Elapsed time: 0.000205 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=4
./merge_sort threads=4
Elapsed time: 0.000257 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=5
./merge_sort threads=5
Elapsed time: 0.001293 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=6
./merge_sort threads=6
Elapsed time: 0.000873 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=7
./merge_sort threads=7
Elapsed time: 0.000299 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=8
./merge_sort threads=8
Elapsed time: 0.000360 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=9
./merge_sort threads=9
Elapsed time: 0.000822 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=9
./merge_sort threads=9
Elapsed time: 0.001558 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=9
./merge_sort threads=9
Elapsed time: 0.000767 seconds
winterful@WinterfulPC:/mnt/e/PROGRAMMING/workspace/Operating-Systems-Course/lab2/solution$ make run T=9
./merge_sort threads=9
Elapsed time: 0.000349 seconds
```

## Выводы

**Оптимальное количество потоков:** Для большого массива (100.000.000 элементов) наилучшее время достигается при использовании 7-9 потоков. Дальнейшее увеличение количества потоков приводит к ухудшению производительности из-за накладных расходов на создание, синхронизацию и переключение контекста потоков. Эти расходы начинают превышать выгоду от распараллеливания.

**Влияние размера данных:** Для маленького массива (100 элементов) многопоточность практически не дает прироста производительности, а в некоторых случаях даже замедляет работу. Это связано с тем, что накладные расходы на многопоточность становятся значительными по сравнению с временем выполнения самой сортировки.

**Закон Амдала:** Мои результаты иллюстрируют закон Амдала, который гласит, что ускорение программы за счет распараллеливания ограничено долей последовательного кода. В сортировке слиянием есть последовательные части (например, слияние отсортированных подмассивов), которые ограничивают максимальное ускорение.
