# TRS

<h3>& >> TSEZAR RELIABLE SYSTEM << &</h3>

<p>TRS __DOS__ это учебная 32-битная операционная система для архитектуры x86 написанная на чистом(<strong>ffreestanding</strong>) C и NASM. </p>

$ Текущая версия: **0x0x2**

## Структура проекта

```
TRS/
├── bootloader/   # stage1 (boot_one.asm) и stage2 (boot_two.asm)
├── kernel/       # ядро: kmain, IDT, PIC, драйверы, стримы, linker.ld
├── libc/         # freestanding-библиотека: mem, string, mutex
├── terminal/     # терминал (в разработке)
├── trfs/         # файловая система (в разработке)
├── build/        # артефакты сборки (создаётся автоматически)
└── Makefile      # сборка, запуск и отладка
```
**Реализовано:**
- Двухстадийный pfuhepxbr (stage1 — 1 сектор, stage2 — до 64 секторов)
- Текстовый вывод на VGA с поддержкой цветов (`driver/vga.c`)
- Таблица прерываний IDT и контроллер PIC
- Драйвер PS/2-клавиатуры
- Система потоков ввода-вывода на ring buffer (`kernel/stream.c`)
- Мини-libc: `memset` / `memcpy` / `strlen` и др., мьютексы на атомарных операциях

**В планах:**
- Терминал (shell) — `terminal/`
- Файловая система TRFS (VFS) — `trfs/`
- Менеджер памяти и пейджинг — `kernel/memory_pagging.c`
- Динамическое выделение памяти (`malloc`/`free`)


**ВДОХНОВЕНИЕ**
- https://github.com/levex/osdev
- https://youtube.com/@vividbw