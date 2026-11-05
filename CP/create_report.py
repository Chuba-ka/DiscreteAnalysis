from pathlib import Path
import subprocess
import tempfile
import ctypes as ct
import itertools
import random
from fractions import Fraction
from types import SimpleNamespace


def bind(library, name, result, arguments):
    function = getattr(library, name)
    function.restype = result
    function.argtypes = arguments
    return function


C = ct.CDLL('libcairo.so.2')
P = ct.CDLL('libpango-1.0.so.0')
PC = ct.CDLL('libpangocairo-1.0.so.0')
G = ct.CDLL('libgobject-2.0.so.0')
ptr, integer = ct.c_void_p, ct.c_int
pdf_create = bind(C, 'cairo_pdf_surface_create', ptr, [ct.c_char_p, ct.c_double, ct.c_double])
create = bind(C, 'cairo_create', ptr, [ptr])
move = bind(C, 'cairo_move_to', None, [ptr, ct.c_double, ct.c_double])
show_page = bind(C, 'cairo_show_page', None, [ptr])
finish = bind(C, 'cairo_surface_finish', None, [ptr])
metadata = bind(C, 'cairo_pdf_surface_set_metadata', None, [ptr, integer, ct.c_char_p])
create_layout = bind(PC, 'pango_cairo_create_layout', ptr, [ptr])
get_context = bind(P, 'pango_layout_get_context', ptr, [ptr])
set_resolution = bind(PC, 'pango_cairo_context_set_resolution', None, [ptr, ct.c_double])
show_layout = bind(PC, 'pango_cairo_show_layout', None, [ptr, ptr])
font_create = bind(P, 'pango_font_description_from_string', ptr, [ct.c_char_p])
font_free = bind(P, 'pango_font_description_free', None, [ptr])
set_font = bind(P, 'pango_layout_set_font_description', None, [ptr, ptr])
set_width = bind(P, 'pango_layout_set_width', None, [ptr, integer])
set_wrap = bind(P, 'pango_layout_set_wrap', None, [ptr, integer])
set_align = bind(P, 'pango_layout_set_alignment', None, [ptr, integer])
set_justify = bind(P, 'pango_layout_set_justify', None, [ptr, integer])
set_spacing = bind(P, 'pango_layout_set_spacing', None, [ptr, integer])
set_text = bind(P, 'pango_layout_set_text', None, [ptr, ct.c_char_p, integer])
get_size = bind(P, 'pango_layout_get_size', None, [ptr, ct.POINTER(integer), ct.POINTER(integer)])
unref = bind(G, 'g_object_unref', None, [ptr])
Pango = SimpleNamespace(Alignment=SimpleNamespace(LEFT=0, CENTER=1, RIGHT=2))
ROOT = Path(__file__).resolve().parent
OUTPUT = ROOT / 'Отчет_по_ЛР_№8_Жадные_алгоритмы.pdf'
WIDTH, HEIGHT = 595.276, 841.890
LEFT, RIGHT, TOP, BOTTOM = 72, 54, 65, 60
surface = pdf_create(str(OUTPUT).encode(), WIDTH, HEIGHT)
metadata(surface, 0, 'Лабораторная работа №8. Жадные алгоритмы. Откорм бычков'.encode())
metadata(surface, 1, 'Н. В. Храмов'.encode())
context = create(surface)
page_number = 1
y = TOP


def draw(text, x, position, width, size=14, bold=False, align=Pango.Alignment.LEFT,
         font='Liberation Serif', justify=False, spacing=2, measure=False):
    layout = create_layout(context)
    set_resolution(get_context(layout), 72)
    description = font_create(f'{font} {"Bold " if bold else ""}{size}'.encode())
    set_font(layout, description)
    set_width(layout, int(width * 1024))
    set_wrap(layout, 2)
    set_align(layout, align)
    set_justify(layout, int(justify))
    set_spacing(layout, int(spacing * 1024))
    set_text(layout, text.encode(), -1)
    w, h = integer(), integer()
    get_size(layout, ct.byref(w), ct.byref(h))
    if not measure:
        move(context, x, position)
        show_layout(context, layout)
    font_free(description)
    unref(layout)
    return h.value / 1024


def finish_page():
    if page_number > 1:
        draw(str(page_number), LEFT, HEIGHT - 38, WIDTH - LEFT - RIGHT,
             size=11, align=Pango.Alignment.CENTER)
    show_page(context)


def new_page():
    global page_number, y
    finish_page()
    page_number += 1
    y = TOP


def paragraph(text, size=14, bold=False, gap=10, justify=True):
    global y
    height = draw(text, LEFT, y, WIDTH - LEFT - RIGHT, size, bold, measure=True)
    if y + height > HEIGHT - BOTTOM:
        new_page()
    y += draw(text, LEFT, y, WIDTH - LEFT - RIGHT, size, bold,
              justify=justify) + gap


def heading(text):
    paragraph(text, size=16, bold=True, gap=12, justify=False)


def code(text, size=10):
    global y
    for line in text.splitlines():
        if y + 13 > HEIGHT - BOTTOM:
            new_page()
        height = draw(line or ' ', LEFT, y, WIDTH - LEFT - RIGHT,
                      size=size, font='Liberation Mono', spacing=0)
        y += max(height, 12)
    y += 10


draw('МОСКОВСКИЙ АВИАЦИОННЫЙ ИНСТИТУТ\n(НАЦИОНАЛЬНЫЙ ИССЛЕДОВАТЕЛЬСКИЙ УНИВЕРСИТЕТ)',
     54, 70, WIDTH - 108, size=14, align=Pango.Alignment.CENTER)
draw('Институт №8 «Компьютерные науки и прикладная математика»\nКафедра 806 «Вычислительная математика и программирование»',
     54, 132, WIDTH - 108, size=14, align=Pango.Alignment.CENTER)
draw('Лабораторная работа №8\nпо курсу «Дискретный анализ»\nна тему «Жадные алгоритмы»',
     54, 260, WIDTH - 108, size=14, bold=True, align=Pango.Alignment.CENTER)
labels = ['Выполнил:', 'Группа:', 'Преподаватель:', 'Проверил:', 'Дата:', 'Оценка:', 'Подпись:']
values = ['Н. В. Храмов', 'М8О-208БВ-24', 'Макаров Н.К.', 'Сахарин Н.А.', '26.10.2026', '', '']
for index, (label, value) in enumerate(zip(labels, values)):
    draw(label, 300, 572 + index * 18, 108, size=14, align=Pango.Alignment.RIGHT)
    draw(value, 420, 572 + index * 18, 135, size=14)
draw('Москва, 2026', LEFT, 792, WIDTH - LEFT - RIGHT, size=14, align=Pango.Alignment.CENTER)
new_page()

heading('Постановка задачи')
paragraph('Вариант 4. «Откорм бычков». Имеются M пищевых добавок, каждая из которых содержит N действующих веществ. Для добавки известны количества веществ a₁, …, aₙ и цена одного мешка. Воздействие добавки равно c₁a₁ + … + cₙaₙ, где коэффициенты cᵢ неизвестны и одинаковы для всех добавок.')
paragraph('Измерение воздействия одной добавки требует одного мешка. Необходимо выбрать набор добавок минимальной суммарной стоимости, измерения которого позволят однозначно определить все N коэффициентов. Если подходящего набора нет, следует сообщить об этом.')
heading('Формат ввода')
paragraph('В первой строке заданы M и N. Следующие M строк содержат N количеств веществ и цену мешка соответствующей добавки. Количества и цены — неотрицательные целые числа не больше 50. Номера добавок соответствуют порядку строк и начинаются с единицы.')
paragraph('В тексте условия указано M ≤ N, однако во втором примере M = 3, N = 2. Реализация допускает оба соотношения. При M < N определить все коэффициенты невозможно. Ограничение времени — 10 секунд, памяти — 1 ГБ.')
heading('Формат вывода')
paragraph('Если определить коэффициенты невозможно, выводится −1. Иначе выводятся номера выбранных добавок в порядке возрастания их номеров во входных данных. При нескольких наборах минимальной стоимости подходит любой из них.')
heading('Алгоритм решения задачи')
paragraph('Каждое измерение задаёт линейное уравнение относительно коэффициентов cᵢ. Они определяются однозначно тогда и только тогда, когда матрица выбранных составов имеет ранг N. Поэтому требуется найти базис строк минимального суммарного веса, где вес строки равен цене мешка.')
paragraph('Используется жадный алгоритм: добавки сортируются по возрастанию цены, затем очередная добавка включается в ответ только в том случае, если её вектор линейно независим от уже выбранных. Проверка выполняется последовательным исключением Гаусса. После выбора N независимых строк поиск завершается.')
new_page()

heading('Этапы решения')
paragraph('1. Чтение данных. Для каждой добавки сохраняются вектор количеств веществ, цена и исходный номер. Если добавок меньше, чем неизвестных коэффициентов, возвращается пустой результат, который main выводит как −1.')
paragraph('2. Сортировка. Метод Select упорядочивает добавки по цене. При равной цене порядок произвольный, поскольку задача допускает несколько оптимальных ответов.')
paragraph('3. Проверка независимости. Метод Add вычитает из очередного вектора кратные строки текущего базиса, обнуляя их опорные координаты. Если остаток нулевой, добавка зависима и пропускается. Иначе опорной выбирается координата с наибольшим модулем, строка нормируется и сохраняется в базисе.')
paragraph('4. Формирование ответа. Номер каждой принятой добавки записывается в результат. При достижении размера N номера сортируются по возрастанию. Если просмотрены все добавки, а N независимых строк не найдено, выводится −1.')
heading('Доказательство корректности')
paragraph('В точной арифметике вычитание линейных комбинаций строк базиса не меняет принадлежность вектора их линейной оболочке. После исключения всех опорных координат нулевой остаток означает зависимость. Ненулевой остаток содержит новую опорную координату и увеличивает ранг на единицу. Следовательно, алгоритм сохраняет независимость выбранных строк.')
paragraph('Докажем оптимальность жадного выбора обменным аргументом. Пусть уже выбранное множество S содержится в некотором оптимальном базисе B. На очередном шаге выбирается самая дешёвая из оставшихся строк v, независимых от S. Если v принадлежит B, свойство сохраняется.')
paragraph('Иначе представим v через строки B. Поскольку v не лежит в линейной оболочке S, в этом представлении есть строка u из B вне S с ненулевым коэффициентом. Замена u на v сохраняет базис. Строка u независима от S и не могла быть ранее пропущена как зависимая: зависимость сохраняется при расширении выбранного множества. Поэтому цена v не больше цены u. После замены получаем оптимальный базис, содержащий S и v.')
paragraph('Изначально пустое множество содержится в любом оптимальном базисе. По индукции после N успешных шагов выбранный базис имеет минимальную стоимость. Если ранг всех строк меньше N, однозначно определить коэффициенты невозможно. Поскольку цены неотрицательны, любой достаточный набор можно сократить до базиса без увеличения стоимости, поэтому поиск среди базисов решает исходную задачу.')
new_page()

heading('Анализ сложности')
paragraph('Сортировка требует O(M log M). При проверке одного вектора выполняется не более N исключений, каждое обрабатывает N координат, поэтому проверка стоит O(N²). Общая верхняя оценка времени — O(M log M + MN²). После нахождения базиса просмотр прекращается. Сортировка N номеров ответа укладывается в эту оценку при M ≥ N.')
paragraph('Исходные составы занимают O(MN) памяти, базис — O(N²), ответ — O(N). Общая оценка — O(MN + N²). На ветви решения при M ≥ N её можно записать как O(MN). При M < N программа возвращает невозможность сразу после чтения данных.')
paragraph('Векторы передаются в метод Add через std::move, поэтому отдельная копия состава для каждой проверки не создаётся. Для результата заранее резервируется N элементов. Цена хранится в int: суммарная стоимость в реализации не вычисляется, поскольку для жадного выбора достаточно сравнения отдельных цен.')
paragraph('Вычисления Гаусса используют long double и порог 10⁻¹² для нулевого остатка. Выбор максимальной по модулю опорной координаты уменьшает влияние деления на малые значения. Доказательство относится к точной арифметике; для плохо обусловленных матриц вычисления с плавающей точкой могут давать погрешность. По сообщению пользователя, реализация прошла все тесты проверяющей системы.')
heading('Структура программы')
paragraph('Supplement — структура данных одной добавки: substances хранит состав, price — цену, index — исходный номер. Она не содержит алгоритмических действий.')
paragraph('LinearBasis инкапсулирует математическую часть: размерность, строки базиса, опорные координаты и порог сравнения. Публичный метод Add проверяет независимость и при успехе расширяет базис. Класс ничего не знает о ценах, номерах добавок и потоках ввода и вывода.')
paragraph('SupplementSelector отвечает за бизнес-логику выбора: сортирует добавки по цене, обращается к LinearBasis и возвращает номера минимального достаточного набора. Метод Select объявлен const; данные для отдельного запуска поступают по значению, а из main передаются перемещением.')
paragraph('Функция main читает данные, вызывает Select и форматирует результат. Алгоритм не зависит от потоков и может вызываться отдельно. Такое разделение соответствует инкапсуляции и принципу единственной ответственности. Наследование и отдельные классы для простого ввода и вывода здесь не требуются.')
new_page()

heading('Исходный код')
code((ROOT / 'main.cpp').read_text().rstrip(), size=9)
new_page()

heading('Тесты')
paragraph('Приведённые результаты.')
test_cases = [
    ('Пример 1: недостаточный ранг', '3 3\n1 0 2 3\n1 0 2 4\n2 0 1 2\n'),
    ('Пример 2: дешёвая альтернатива', '3 2\n2 1 2\n1 2 9\n1 2 3\n'),
    ('Добавок меньше, чем веществ', '1 2\n1 1 5\n'),
    ('Нулевая строка и бесплатный базис', '3 2\n0 0 0\n1 0 0\n0 1 0\n'),
    ('Пропуск дешёвой зависимой строки', '4 2\n1 0 1\n2 0 2\n0 1 3\n1 1 10\n'),
    ('Равные цены', '3 2\n1 0 5\n0 1 5\n1 1 5\n'),
    ('Одно вещество', '3 1\n0 0\n2 7\n1 3\n'),
    ('Все составы нулевые', '2 2\n0 0 1\n0 0 2\n'),
]
build_directory = tempfile.TemporaryDirectory(prefix='task8-report-')
binary = str(Path(build_directory.name) / 'solution')
subprocess.run(['g++', '-std=c++20', '-O2', '-Wall', '-Wextra', '-Wpedantic',
                '-Wconversion', str(ROOT / 'main.cpp'), '-o', binary], check=True)


def exact_rank(rows, dimension):
    basis = {}
    for source in rows:
        row = list(map(Fraction, source[:dimension]))
        for column in range(dimension):
            if not row[column]:
                continue
            if column in basis:
                factor = row[column]
                row = [x - factor * y for x, y in zip(row, basis[column])]
            else:
                factor = row[column]
                basis[column] = [x / factor for x in row]
                break
    return len(basis)


def verify(data):
    values = list(map(int, data.split()))
    m, n = values[:2]
    rows = [values[2 + i * (n + 1):2 + (i + 1) * (n + 1)] for i in range(m)]
    optimum = min((sum(rows[i][-1] for i in ids)
                   for ids in itertools.combinations(range(m), n)
                   if exact_rank([rows[i] for i in ids], n) == n), default=None)
    result = subprocess.check_output([binary], input=data, text=True).strip()
    if optimum is None:
        assert result == '-1', (data, result)
    else:
        ids = [int(value) - 1 for value in result.split()]
        assert len(ids) == n and len(set(ids)) == n, (data, result)
        assert all(0 <= i < m for i in ids), (data, result)
        assert ids == sorted(ids), (data, result)
        assert exact_rank([rows[i] for i in ids], n) == n, (data, result)
        assert sum(rows[i][-1] for i in ids) == optimum, (data, result)
    return result


for index, (title, data) in enumerate(test_cases, 1):
    result = verify(data)
    if y + 190 > HEIGHT - BOTTOM:
        new_page()
    paragraph(f'Тест {index}. {title}', bold=True, gap=4, justify=False)
    code(f'Ввод:\n{data.rstrip()}\nВывод:\n{result}', size=10)

rng = random.Random(57)
for _ in range(500):
    n = rng.randint(1, 5)
    m = rng.randint(1, 8)
    rows = [[rng.randint(0, 5) for _ in range(n)] + [rng.randint(0, 50)]
            for _ in range(m)]
    data = f'{m} {n}\n' + ''.join(' '.join(map(str, row)) + '\n' for row in rows)
    verify(data)
new_page()

heading('Проверка решения')
paragraph('В первом примере второй столбец состоит из нулей, а первые две строки совпадают. Ранг меньше трёх, поэтому программа выводит −1. Во втором примере выбираются добавки 1 и 3 стоимостью 2 + 3 = 5; их определитель равен 3, поэтому оба коэффициента определяются однозначно.')
paragraph('В тесте с зависимыми строками добавка 2 дешевле добавки 3, но пропускается: её состав вдвое больше состава добавки 1 и не даёт нового независимого уравнения. Выбираются добавки 1 и 3. Нулевая строка не включается в базис даже при нулевой цене.')
paragraph('Генератор отчёта проверил восемь приведённых тестов и 500 случайных случаев с 1 ≤ N ≤ 5 и 1 ≤ M ≤ 8. Эталон перебирает все подмножества размера N, вычисляет ранг в точной рациональной арифметике Fraction и выбирает минимальную стоимость. Все результаты совпали с эталоном по возможности решения и минимальной стоимости.')
paragraph('Для каждого найденного ответа дополнительно проверялись количество и уникальность номеров, их допустимый диапазон, порядок возрастания и полный ранг выбранных составов. При равных ценах сравнивалась стоимость, а не конкретный набор номеров.')
paragraph('Проверочная сборка выполнена с параметрами g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wconversion. Для режима отправки (make) C++20 в проекте имеется Makefile, создающий исполняемый файл main. По сообщению пользователя, все тесты проверяющей системы пройдены. Локальный полный перебор подтверждает корректность на малых входах, но не является измерением производительности на максимальных размерах, которые в предоставленном условии не указаны.')
heading('Вывод')
paragraph('В ходе работы реализовано решение задачи «Откорм бычков» жадным алгоритмом построения базиса минимальной стоимости. Связь измерений с системой линейных уравнений позволяет свести достаточность набора к полному рангу, а последовательное исключение Гаусса — проверять независимость очередной добавки.')
paragraph('Оптимальность жадного выбора обоснована обменом строк базиса. Реализация имеет временную сложность O(M log M + MN²) и использует O(MN + N²) памяти. Математические операции, выбор набора и ввод-вывод разделены. Корректность подтверждена локальными проверками и прохождением тестов проверяющей системы; при оценке численной устойчивости учитывается использование long double.')
finish_page()
finish(surface)
build_directory.cleanup()
print(OUTPUT)
print(f'Pages: {page_number}')
print('Verified: 8 fixed tests and 500 random tests against exact exhaustive search')
