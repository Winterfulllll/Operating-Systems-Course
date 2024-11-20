# **Лабораторная работа №4** (Вариант 24)

## Цель работы

Приобретение практических навыков в:

- Создание динамических библиотек;
- Создание программ, которые используют функции динамических библиотек.

## Задание

**Требуется создать _динамические библиотеки_, которые реализуют заданный вариантом функционал. Далее использовать данные библиотеки 2-мя способами:**

1. Во время компиляции (на этапе «линковки»/linking)
2. Во время исполнения программы. Библиотеки загружаются в память с помощью интерфейса ОС для работы с динамическими библиотеками

**В конечном итоге, в лабораторной работе необходимо получить следующие части:**

- Динамические библиотеки, реализующие контракты, которые заданы вариантом;
- Тестовая программа (программа №1), которая используют одну из библиотек, используя информацию полученные на этапе компиляции;
- Тестовая программа (программа №2), которая загружает библиотеки, используя только их относительные пути и контракты.

_Провести анализ двух типов использования библиотек._

**Пользовательский ввод для обоих программ должен быть организован следующим образом:**

1. Если пользователь вводит команду «0», то программа переключает одну реализацию контрактов на другую (необходимо только для программы №2). Можно реализовать лабораторную работу без данной функции, но максимальная оценка в этом случае будет «хорошо»;
2. «1 arg1 arg2 … argN», где после «1» идут аргументы для первой функции, предусмотренной контрактами. После ввода команды происходит вызов первой функции, и на экране появляется результат её выполнения;
3. «2 arg1 arg2 … argM», где после «2» идут аргументы для второй функции, предусмотренной контрактами. После ввода команды происходит вызов второй функции, и на экране появляется результат её выполнения.

## Контракты и реализации функций

### 4. Подсчёт наибольшего общего делителя для двух натуральных чисел

**Сигнатура:** `Int GCF(int A, int B)`
**Реализация 1:** Алгоритм Евклида
**Реализация 2:** Наивный алгоритм. Пытаться разделить числа на все числа, что меньше A и B.

### 7. Подсчет площади плоской геометрической фигуры по двум сторонам

**Сигнатура:** `Float Square(float A, float B)`
**Реализация 1:** Фигура прямоугольник
**Реализация 2:** Фигура прямоугольный треугольник

## Справочный материал

1. https://msdn.microsoft.com/en-us/library/ms235636.aspx
2. https://msdn.microsoft.com/en-us/library/windows/desktop/ms684175(v=vs.85).aspx
3. https://msdn.microsoft.com/en-us/library/windows/desktop/ms683212(v=vs.85).aspx
4. http://www.ibm.com/developerworks/library/l-dynamic-libraries/