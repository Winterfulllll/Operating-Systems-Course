# **Курсовой проект** (Вариант 12)

## Цель работы

1. Приобретение практических навыков в использовании знаний, полученных в течении курса
2. Проведение исследования в выбранной предметной области

## Проектирование консольной клиент-серверной игры

На основе любой из выбранных технологий:

- Pipes
- Sockets
- Сервера очередей
- И другие

Создать собственную игру более, чем для одного пользователя. Игра может быть устроена по принципу: клиент-клиент, сервер-клиент.

## Задание

Необходимо спроектировать и реализовать программный прототип в соответствии с выбранным вариантом. Произвести анализ и сделать вывод на основании данных, полученных при работе программного прототипа.

Консоль-серверная игра. Необходимо написать консоль-серверную игру. Необходимо написать 2 программы: сервер и клиент. Сначала запускается сервер, а далее клиенты соединяются с сервером. Сервер координирует клиентов между собой. При запуске клиента игрок может выбрать одно из следующих действий (возможно больше, если предусмотрено вариантом):

- Создать игру, введя ее имя
- Присоединиться к одной из существующих игр по имени игры

### **«Быки и коровы»** (угадывать необходимо числа) _(12 вариант)_

Общение между сервером и клиентом необходимо организовать при помощи `memory map`. При создании каждой игры необходимо указывать количество игроков, которые будут участвовать. То есть угадывать могут несколько игроков. Должна быть реализована функция поиска игры, то есть игрок пытается войти в игру не по имени, а просто просит сервер найти ему игру.

### Команды клиента

- `create <room_id> <players_count> <digits_count>` - создание игровой комнаты `room_id` с количеством участников `players_count` (по умолчанию `players_count = 1`), загадонное число будет состоять из `2 <= digits_count <= 10` (по умолчанию `digits_count = 4`) неповторяющихся цифр;
- `enter <room_id>` - войти в игровую комнату `room_id`;
- `find` - запустить поиск свободных игровых комнат;
- `help` - вывод всех команд;