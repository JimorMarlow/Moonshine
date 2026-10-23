// ============================================================
// Переходник для датчика потока SEN-HZ06D -> ПВХ шланг 8x12 мм
// ============================================================

// --- Основные размеры (мм) ---
LENGTH = 20;                // Общая длина переходника
SENSOR_SIDE_D = 6.0;        // Внутренний диаметр со стороны датчика
HOSE_SIDE_D = 3.5;          // Внутренний диаметр выходного отверстия
OUTER_D = 8.4;              // Наружный диаметр под шланг

// --- Параметры уплотнительного кольца ---
GASKET_D = 6.0;             // Диаметр резиновой шайбы
GASKET_THICKNESS = 2.0;     // Толщина резиновой шайбы
GASKET_CLEARANCE = 1.0;     // Запас для сжатия/расплющивания прокладки
GASKET_SEAT_DEPTH = GASKET_THICKNESS - GASKET_CLEARANCE;

// --- Параметры насечки (рифления) ---
BARB_COUNT = 3;             // Количество насечек
BARB_WIDTH = 1.0;           // Ширина канавки
BARB_START_OFFSET = 4;      // Отступ от края шланга до первой канавки
GROOVE_INNER_D = 7.0;       // Внутренний диаметр канавки (оставляет стенку целой)

// --- Параметры конуса для облегчения надевания шланга ---
TAPER_LENGTH = 2;           // Длина конусной части
TAPER_REDUCTION = 1;        // Уменьшение диаметра на конце (мм)

// --- Параметры печати и рендера ---
$fn = 100;                  // Детализация окружностей
OVERCUT = 0.01;             // Выступание отверстий за пределы детали (для чистого рендера)

// ============================================================
// Модули
// ============================================================

module main_body() {
    // Внешний цилиндр с насечкой и конусом на конце
    difference() {
        union() {
            // Основная прямая часть
            cylinder(d=OUTER_D, h=LENGTH - TAPER_LENGTH, center=false);
            // Конусное сужение на выходе под шланг
            translate([0, 0, LENGTH - TAPER_LENGTH])
                cylinder(d1=OUTER_D, d2=OUTER_D - TAPER_REDUCTION, h=TAPER_LENGTH, center=false);
        }
        
        // Вычитаем насечки (кольцевые канавки)
        for (i = [0 : BARB_COUNT - 1]) {
            translate([0, 0, LENGTH - BARB_START_OFFSET - i * (BARB_WIDTH * 2)])
                difference() {
                    // Внешний контур канавки (с запасом)
                    cylinder(d=OUTER_D + 0.1, h=BARB_WIDTH, center=false);
                    // Внутренний контур канавки (оставляет стенку целой)
                    // Увеличиваем высоту на OVERCUT, чтобы грани тоже не совпадали
                    translate([0, 0, -OVERCUT/2])
                        cylinder(d=GROOVE_INNER_D, h=BARB_WIDTH + OVERCUT, center=false);
                }
        }
    }
}

module inner_channel() {
    // Внутренний канал: от датчика (6 мм) до выхода (3.5 мм)
    
    // Сторона датчика (вход) - 6 мм. Выдвигаем вниз на OVERCUT
    translate([0, 0, -OVERCUT])
        cylinder(d=SENSOR_SIDE_D, h=LENGTH * 0.6 + OVERCUT, center=false);
    
    // Сторона шланга (выход) - 3.5 мм. Выдвигаем вверх на OVERCUT
    translate([0, 0, LENGTH * 0.6])
        cylinder(d=HOSE_SIDE_D, h=LENGTH * 0.4 + OVERCUT, center=false);
    
    // Плавный переход (конус) между диаметрами
    translate([0, 0, LENGTH * 0.4])
        cylinder(d1=SENSOR_SIDE_D, d2=HOSE_SIDE_D, h=LENGTH * 0.2, center=false);
}

module gasket_seat() {
    // Посадочное место под резиновую шайбу со стороны датчика
    // Выдвигаем вниз на OVERCUT для чистого рендера
    translate([0, 0, -OVERCUT])
        cylinder(d=GASKET_D + GASKET_CLEARANCE, h=GASKET_SEAT_DEPTH + OVERCUT, center=false);
}

// ============================================================
// Сборка
// ============================================================

module adapter() {
    difference() {
        main_body();
        inner_channel();
        gasket_seat();
    }
}

// --- Рендер ---
adapter();