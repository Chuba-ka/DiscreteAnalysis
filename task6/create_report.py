from pathlib import Path
import subprocess
import tempfile
import ctypes as ct
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
OUTPUT = ROOT / 'Отчет_по_ЛР_№6_Динамическое_программирование.pdf'
WIDTH, HEIGHT = 595.276, 841.890
LEFT, RIGHT, TOP, BOTTOM = 72, 54, 65, 60
surface = pdf_create(str(OUTPUT).encode(), WIDTH, HEIGHT)
metadata(surface, 0, 'Лабораторная работа №6. Динамическое программирование'.encode())
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
draw('Лабораторная работа №6\nпо курсу «Дискретный анализ»\nна тему «Динамическое программирование»',
     54, 260, WIDTH - 108, size=14, bold=True, align=Pango.Alignment.CENTER)
labels = ['Выполнил:', 'Группа:', 'Преподаватель:', 'Проверил:', 'Дата:', 'Оценка:', 'Подпись:']
values = ['Н. В. Храмов', 'М8О-208БВ-24', 'Макаров Н.К.', 'Сахарин Н.А.', '02.10.2026', '', '']
for index, (label, value) in enumerate(zip(labels, values)):
    draw(label, 300, 572 + index * 18, 108, size=14, align=Pango.Alignment.RIGHT)
    draw(value, 420, 572 + index * 18, 135, size=14)
draw('Москва, 2026', LEFT, 792, WIDTH - LEFT - RIGHT, size=14, align=Pango.Alignment.CENTER)
new_page()

heading('Постановка задачи')
paragraph('Дано натуральное число n. За один ход разрешается вычесть единицу, разделить число на два или разделить его на три. Деление допускается только нацело. Стоимость каждой операции равна текущему значению числа до её выполнения.')
paragraph('Необходимо преобразовать число n в единицу так, чтобы суммарная стоимость всех операций была минимальной, и вывести одну из оптимальных последовательностей операций.')
heading('Формат ввода')
paragraph('В первой строке задано целое число n, 2 ≤ n ≤ 10⁷.')
heading('Формат вывода')
paragraph('В первой строке выводится минимальная стоимость преобразования. Во второй строке — последовательность операций, разделённых пробелами: -1, /2 или /3. При наличии нескольких оптимальных последовательностей подходит любая из них.')
heading('Алгоритм решения задачи')
paragraph('Решение основано на динамическом программировании. Обозначим через dp[i] минимальную стоимость преобразования числа i в единицу. Для единицы операции не нужны, поэтому dp[1] = 0.')
paragraph('Для каждого i от 2 до n рассматриваются все допустимые первые операции. Их стоимость одинакова и равна i, поэтому достаточно выбрать переход с наименьшей стоимостью оставшегося преобразования.')
paragraph('dp[i] = i + min { dp[j] : j ∈ S(i) },\nS(i) = {i − 1} ∪ {i / 2, если i кратно 2}\n                         ∪ {i / 3, если i кратно 3}.', justify=False)
new_page()

heading('Этапы решения')
paragraph('1. Заполнение таблицы. Значения dp вычисляются по возрастанию i. Для каждой позиции в массиве op сохраняется операция, которая обеспечивает минимальную стоимость. При равенстве стоимостей остаётся ранее выбранная операция: сначала -1, затем /2, затем /3.')
paragraph('2. Восстановление ответа. Начиная с n, программа добавляет op[n] в последовательность и переходит к числу после этой операции. Процесс повторяется до достижения единицы. Обращать последовательность не требуется: операции уже записываются в порядке выполнения.')
heading('Доказательство корректности')
paragraph('Докажем по индукции, что dp[i] хранит минимальную стоимость. База: dp[1] = 0, поскольку исходное число уже равно единице.')
paragraph('Пусть утверждение верно для всех чисел меньше i. Любое преобразование i начинается с одной из допустимых операций и приводит к числу j < i. Стоимость первой операции равна i. По предположению индукции оптимальная стоимость оставшейся части равна dp[j]. Следовательно, любое решение стоит не меньше i + min dp[j] по допустимым j.')
paragraph('Алгоритм перебирает все такие переходы и выбирает минимум. Выбранная операция вместе с оптимальным преобразованием j достигает этой стоимости. Значит, вычисленное dp[i] оптимально.')
paragraph('Массив op сохраняет переход, соответствующий найденному минимуму. Каждый переход строго уменьшает число и не опускает его ниже единицы. Поэтому восстановление завершается в единице, а сумма стоимостей выбранных операций равна dp[n].')
new_page()

heading('Анализ сложности')
paragraph('Временная сложность — O(n): для каждого из n − 1 состояний проверяются не более трёх переходов. Восстановление и вывод L операций требуют O(L), где L ≤ n − 1, поэтому общая оценка остаётся O(n).')
paragraph('Пространственная сложность — O(n): таблица dp содержит n + 1 значений long long, таблица op — n + 1 значений перечисления с базовым типом char. Дополнительно хранятся L операций результата и строка вывода.')
paragraph('На использованной платформе основные таблицы занимают примерно 9(n + 1) байт. При n = 10⁷ это около 85,8 МиБ без учёта остальных объектов и служебных расходов. В предыдущем запуске максимальный расход памяти процесса составил около 89 МиБ. Соответствие ограничению памяти зависит от лимита проверяющей системы, который в предоставленном условии не указан.')
paragraph('Для стоимости выбран тип long long. Допустимый путь из одних вычитаний имеет стоимость n(n + 1) / 2 − 1. При максимальном n эта величина равна 50 000 004 999 999 и помещается в 64-битное целое число. Следовательно, минимальная стоимость также представима этим типом.')
heading('Структура программы')
paragraph('MinCostTransformer отвечает за вычисление оптимальной стоимости и восстановление пути. Его вспомогательные методы apply и restorePath скрыты в private. Все таблицы создаются внутри transform, поэтому отдельные запуски не зависят друг от друга.')
paragraph('Operation задаёт допустимые виды операций, TransformationResult объединяет стоимость и последовательность. OperationFormatter переводит операции в требуемый текстовый формат. InputReader и ResultWriter работают с потоками ввода и вывода, переданными через конструкторы.')
paragraph('Application координирует чтение, вычисление и вывод. Такое разделение обязанностей изолирует алгоритм от представления результата и соответствует принципам инкапсуляции и единственной ответственности. Наследование для решения этой задачи не требуется.')
new_page()

heading('Исходный код')
code((ROOT / 'main.cpp').read_text().rstrip(), size=9)
new_page()

heading('Тесты')
paragraph('Результаты ниже получены запуском проверяемой программы. В каждом тесте стоимость равна сумме значений числа перед операциями. При равных стоимостях приведён путь, выбранный текущей реализацией.')
test_values = [2, 3, 4, 5, 6, 10, 27, 82]
build_directory = tempfile.TemporaryDirectory(prefix='task6-report-')
binary = str(Path(build_directory.name) / 'solution')
subprocess.run(['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Wpedantic',
                '-Wconversion', str(ROOT / 'main.cpp'), '-o', binary], check=True)
for index, n in enumerate(test_values, 1):
    if y + 100 > HEIGHT - BOTTOM:
        new_page()
    paragraph(f'Тест {index}. n = {n}', bold=True, gap=4, justify=False)
    result = subprocess.check_output([binary], input=f'{n}\n', text=True).strip()
    code(f'Ввод:\n{n}\nВывод:\n{result}', size=10)
new_page()

heading('Проверка решения')
paragraph('В примере n = 82 программа выводит стоимость 202 и последовательность -1 /3 /3 /3 /3. Числа перед операциями: 82, 81, 27, 9, 3; их сумма равна 202.')
paragraph('Для n = 2 выбирается -1, хотя /2 имеет ту же стоимость. Для n = 4 последовательность /2 -1 стоит 4 + 2 = 6. Это подтверждает, что при равенстве цен выбор операции не нарушает оптимальность.')
paragraph('Все входы от 2 до 1000 были сопоставлены с независимым решением через алгоритм Дейкстры: вершины графа — значения числа, рёбра — допустимые операции с весом, равным исходному числу. Во всех случаях минимальные стоимости совпали. Для каждого ответа дополнительно проверены допустимость делений, достижение единицы, сумма стоимости и двухстрочный формат вывода.')
paragraph('Проверены входы 9 999 999 и 10 000 000. Для максимального значения программа получила стоимость 20 079 033; выданная последовательность допустима, заканчивается в единице и имеет указанную стоимость. Эти большие тесты проверяют восстановление пути и работу на верхней границе; независимое сравнение оптимальности выполнено для диапазона 2…1000.')
paragraph('Сборка: g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wconversion main.cpp. Компилятор не выдал предупреждений. Измеренный ранее запуск для n = 10⁷ занял около 0,07 с; это результат на текущем окружении, а не гарантированное время на другой машине.')
heading('Вывод')
paragraph('В ходе работы реализовано решение задачи минимизации стоимости преобразования натурального числа в единицу методом динамического программирования. Таблица оптимальных стоимостей позволяет учитывать все допустимые первые операции, а массив выбранных переходов — восстановить одну из оптимальных последовательностей.')
paragraph('Корректность решения обоснована индукцией по значению числа и подтверждена сравнением с независимым алгоритмом на малых входах. Реализация соответствует формату ввода и вывода и имеет линейную сложность по времени и памяти. Разделение вычислений, ввода и вывода между классами обеспечивает ясную структуру программы.')
finish_page()
finish(surface)
build_directory.cleanup()
print(OUTPUT)
print(f'Pages: {page_number}')
