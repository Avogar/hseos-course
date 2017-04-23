## Измерение времени

Для представления интервалов времени в POSIX системах существуют два типа данных.
Некоторые системные вызовы используют тип `struct timeval`, определенный следующим образом:

```
struct timeval
{
    time_t      tv_sec;         /* seconds */
    suseconds_t tv_usec;        /* microseconds */
};
```

Тип `suseconds_t` - это 32-битный знаковый тип, достаточный для представления числа микросекунд
в секунде (от 0 до 999999). Если поле `tv_usec` содержит значение, выходящее из этого интервала,
системные вызовы вернут ошибку `EINVAL`.

Некоторые системные вызовы используют тип `struct timespec`, определенный следующим образом:

```
struct timespec
{
    time_t tv_sec;        /* seconds */
    long   tv_nsec;       /* nanoseconds */
};
```

Поле `tv_nsec` должно содержать значение в интервале от 0 до 999999999.

При использовании этих структур секундная часть требуемого интервала времени записывается в поле `tv_sec`,
а остаток меньший секунды - в поле `tv_usec` или `tv_nsec`.

В любом случае в приложениях, выполняющихся не с приоритетом реального времени, не следуюет ожидать точности
измерения или реакции выше, чем частота системного таймера, с которой вызывается планировщик процессов.
Частота системного таймера составляет от 100 до 1000 Гц, причем может меняться со временем.
Кроме того, система может переводиться в спящий режим, в котором системный таймер может быть приостановлен.

Во многих ситуациях удобнее всего хранить в программе и обрабатывать время, представленное как
число миллисекунд от условной точки начала отсчета (например, от эпохи Unix). Для хранения такого времени
достаточно 64-битной целой знаковой переменной. При использовании системных вызовов, требующих
структуры `struct timespec` или `struct timeval` интервал времени в миллисекундах очевидным образом
пересчитывается в требуемые значения полей структур.

Текущее астрономическое время можно получить с помощью функции `gettimeofday`:

```
#include <sys/time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz);
```

Текущее астрономическое время сохраняется по указателю `tv`. Не стоит ожидать, что возвращено будет время с точностью до микросекунды.
Точность измерения времени будет не выше, чем частота системного таймера.

## Таймеры

В данном разделе рассматривается интерфейс таймеров, реализованный с помощью системного вызова `setitimer`.
Несмотря на то, что он стандартный для POSIX систем, в современной версии стандарта он считается устаревшим (deprecated).

Таймер может настраиваться на однократное или периодическое срабатывание. Гарантируется, что таймер не сработает раньше
истечения требуемого интервала времени, но может сработать позже, если система сильно загружена.

Каждому процессу доступны три независимых таймера: таймер реального времени (`ITIMER_REAL`), таймер виртуального времени (`ITIMER_VIRTUAL`),
таймер профилирования (`ITIMER_PROF`).

Таймер реального времени (`ITIMER_REAL`) измеряет интервалы астрономического времени. По истечению таймера в процесс
посылается сигнал `SIGALRM`.

Таймер виртуального времени (`ITIMER_VIRTUAL`) измеряет интервалы процессорного времени, когда процесс работает в пользовательском
режиме. По истечению таймера в процесс посылается сигнал `SIGVTALRM`.

Профилировочный таймер (`ITIMER_PROF`) измеряет интервалы процессорного времени, когда процесс работает и в пользовательском
режиме, и в режиме ядра. По истечению таймера в процесс посылается сигнал `SIGPROF`.

Для установки таймера используется функция `setitimer`.

```
#include <sys/time.h>

int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
```

Тип `struct itimerval` определен следующим образом:

```
struct itimerval
{
    struct timeval it_interval; /* Interval for periodic timer */
    struct timeval it_value;    /* Time until next expiration */
};
```

Параметр `which` задает тип таймера (`ITIMER_REAL`, `ITIMER_VIRTUAL` или `ITIMER_PROF`). Параметр `new_value`
задает новое значение таймера, а в параметр `old_value` возвращается текущее значение.

Чтобы установить таймер на однократное срабатывание, в поле `it_value` нужно записать интервал времени до срабатывания,
а поле `it_interval` - обнулить. Для установки таймера на периодическое срабатывание в поля `it_value` и
`it_interval` нужно записать период срабатывания таймера. Для сброса таймера (его остановки)
нужно обнулить поля `it_interval` и `it_value`.

Например, пусть переменная `timeout` хранит интервал времени в миллисекундах до срабатывания таймера.
Тогда таймер реального времени может быть настроен следующим образом:

```
    int64_t timeout;

    // ...

    struct itimerval to = {}; // структура инициализирована нулями
    to.it_value.tv_sec = timeout / 1000;
    to.it_value.tv_usec = (timeout % 1000) * 1000;
    setitimer(ITIMER_REAL, &to, NULL);
```

## Мультиплексирование ввода-вывода
