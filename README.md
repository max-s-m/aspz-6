# Варіант 18: Порівняння звітів ASan та Valgrind на UAF

## Умова:

Test Case #8: Use-after-free. Порівняйте звіт ASan із Valgrind.

## [Програмний код](1.c)

## Опис:

Проведено порівняння звітів Asan та Valgrind до однієї і тієї ж програми Test Case #8: Use after free, код якої надано у лекції
6 Debugging Tools for Memory Issues in Linux. Для виконання частини завдання з Asan виникла проблема, а саме повідомлення про помилку,
що свідчило про несумісність санітайзеру з ASLR та PIE.

Спочатку з аргументом -no-pie при компіляції прибрано параметр "Position
Independent Executables" (pie дозволяє програмам працювати правильно незалежно від розташуванні в пам'яті), що не дало результату
та призвело до тієї ж помилки. Далі перевірено статус ASLR та вимкнено його (присвоєно 0) командою sudo sysctl kern.elf64.aslr.enable=0.
Таким чином Address Space Layout Randomization (рандомізує місце завантаження в пам'яті виконуваних файлів для захисту від buffer
overflow) відключено до наступного перезапуску системи.

Вимкнення ASLR дозволило провести запуск з ASAN, а Valgrind і без цього не мав проблем. Про обидва результати можна сказати що:
- Asan виводить чітку назву помилки "heap-use-after-free", а Valgrind більш неявно "Invalid write of size 4", проте обидва цю
помилку виявляють.
- Обидва методи показують нормально локації дій у коді (1.c:6 - free, 1.c:5 - malloc()).
- Asan видає значно детальніший звіт зі станами байтів (fa, fd в тд) та детальними адресами дій, навіть з адресами як у файловому
провіднику, а не тільки як шіснадцядкові числа, коли вивід valgrind більш короткий.
- І там і там розпізнано помилку пам'яті (ERROR SUMMARY у valgrind, та SUMMARY у ASan).

## Результати виконання:

### Звіт asan (потребувалося тимчасове вимкнення aslr):

```
ax@aspz:~/c/6 $ gcc -fsanitize=address -g -o 1_asan 1.c
max@aspz:~/c/6 $ ./1_asan
This sanitizer is not compatible with enabled ASLR and binaries compiled with PIE
max@aspz:~/c/6 $ gcc -fsanitize=address -no-pie -g 1.c -o 1_asan
max@aspz:~/c/6 $ ./1_asan
This sanitizer is not compatible with enabled ASLR and binaries compiled with PIE
max@aspz:~/c/6 $ sysctl kern.elf64.aslr.enable
kern.elf64.aslr.enable: 1
max@aspz:~/c/6 $ sudo sysctl kern.elf64.aslr.enable=0
kern.elf64.aslr.enable: 1 -> 0
max@aspz:~/c/6 $ gcc -fsanitize=address -g 1.c -o 1_asan
max@aspz:~/c/6 $ ./1_asan
=================================================================
==1114==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010 at pc 0x000000400771 bp 0x7fffffffe4b0 sp 0x7fffffffe4a8
WRITE of size 4 at 0x602000000010 thread T0
    #0 0x400770 in main /home/max/c/6/1.c:8
    #1 0x800b4ec39 in __libc_start1 /usr/src/lib/libc/csu/libc_start1.c:157
    #2 0x40065f in _start (/home/max/c/6/1_asan+0x40065f)

0x602000000010 is located 0 bytes inside of 4-byte region [0x602000000010,0x602000000014)
freed by thread T0 here:
    #0 0x8004eb0d8  (/usr/local/lib/gcc13/libasan.so.8+0xa60d8)
    #1 0x400733 in main /home/max/c/6/1.c:6
    #2 0x800b4ec39 in __libc_start1 /usr/src/lib/libc/csu/libc_start1.c:157
    #3 0x40065f in _start (/home/max/c/6/1_asan+0x40065f)
    #4 0x800425007  (<unknown module>)

previously allocated by thread T0 here:
    #0 0x8004ebe2f in malloc (/usr/local/lib/gcc13/libasan.so.8+0xa6e2f)
    #1 0x400723 in main /home/max/c/6/1.c:5
    #2 0x800b4ec39 in __libc_start1 /usr/src/lib/libc/csu/libc_start1.c:157
    #3 0x40065f in _start (/home/max/c/6/1_asan+0x40065f)
    #4 0x800425007  (<unknown module>)

SUMMARY: AddressSanitizer: heap-use-after-free /home/max/c/6/1.c:8 in main
Shadow bytes around the buggy address:
  0x601ffffffd80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x601ffffffe00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x601ffffffe80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x601fffffff00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x601fffffff80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x602000000000: fa fa[fd]fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x602000000080: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x602000000100: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x602000000180: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x602000000200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x602000000280: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==1114==ABORTING
```

### Запуск з перевіркою витоків valgrind:

```
max@aspz:~/c/6 $ gcc -g 1.c -o 1
max@aspz:~/c/6 $ valgrind --leak-check=full ./1
==1065== Memcheck, a memory error detector
==1065== Copyright (C) 2002-2024, and GNU GPL'd, by Julian Seward et al.
==1065== Using Valgrind-3.24.0 and LibVEX; rerun with -h for copyright info
==1065== Command: ./1
==1065==
==1065== Invalid write of size 4
==1065==    at 0x4005E8: main (1.c:8)
==1065==  Address 0x5561040 is 0 bytes inside a block of size 4 free'd
==1065==    at 0x48502BC: free (vg_replace_malloc.c:993)
==1065==    by 0x4005E3: main (1.c:6)
==1065==  Block was alloc'd at
==1065==    at 0x484E2E4: malloc (vg_replace_malloc.c:450)
==1065==    by 0x4005D3: main (1.c:5)
==1065==
==1065==
==1065== HEAP SUMMARY:
==1065==     in use at exit: 0 bytes in 0 blocks
==1065==   total heap usage: 1 allocs, 1 frees, 4 bytes allocated
==1065==
==1065== All heap blocks were freed -- no leaks are possible
==1065==
==1065== For lists of detected and suppressed errors, rerun with: -s
==1065== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```
