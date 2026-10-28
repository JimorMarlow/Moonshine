// ==========================================
// Заглушка для алюминиевого профиля 20x10
// ==========================================

// --- Основные размеры (мм) ---
profile_w = 20.3;      // Ширина профиля (ваш замер)
profile_h = 10.3;      // Высота профиля (ваш замер)
wall_thickness = 3.0;  // Толщина стенок и дна
cap_length = 50.0;     // Общая длина заглушки

// --- Настройки посадки ---
clearance = 0.6;       // Зазор для легкой усадки

// --- Настройки эстетики и удобства ---
outer_r = 3.0;         // Радиус скругления внешних вертикальных углов
chamfer = 2.0;         // Размер верхней фаски
base_fillet = 2.0;     // Радиус скругления нижнего внешнего края

// --- Настройки рендера ---
z_overlap = 0.01;      // Выступ для предотвращения Z-fighting (артефактов рендера)

// --- Вычисляемые переменные ---
inner_w = profile_w + clearance;
inner_h = profile_h + clearance;
outer_w = inner_w + 2 * wall_thickness;
outer_h = inner_h + 2 * wall_thickness;
eps = 0.001;           // Микро-размер для 2D-профилей

$fn = 60;

// Модуль внешнего прямоугольника со скруглениями
module rounded_rect(w, h, r) {
    hull() {
        for (x = [-w/2 + r, w/2 - r]) {
            for (y = [-h/2 + r, h/2 - r]) {
                translate([x, y]) circle(r = r);
            }
        }
    }
}

// Внешний корпус
module outer_shell() {
    union() {
        // Основная часть со скругленным основанием
        intersection() {
            linear_extrude(cap_length - chamfer)
                rounded_rect(outer_w, outer_h, outer_r);
            
            minkowski() {
                linear_extrude(cap_length - chamfer - base_fillet)
                    rounded_rect(outer_w - 2*base_fillet, outer_h - 2*base_fillet, max(0.1, outer_r - base_fillet));
                sphere(r=base_fillet);
            }
        }
        
        // Верхняя внешняя фаска
        hull() {
            translate([0, 0, cap_length - chamfer])
                linear_extrude(eps)
                    rounded_rect(outer_w, outer_h, outer_r);
            translate([0, 0, cap_length])
                linear_extrude(eps)
                    rounded_rect(outer_w - 2*chamfer, outer_h - 2*chamfer, max(0.1, outer_r - chamfer));
        }
    }
}

// Внутренняя полость
module inner_cavity() {
    // Вычитаем z_overlap снизу, чтобы пробить грань дна
    translate([0, 0, wall_thickness - z_overlap]) {
        // 1. Основной внутренний канал (строго прямоугольный)
        linear_extrude(cap_length - chamfer - wall_thickness + 2*z_overlap)
            square([inner_w, inner_h], center=true);
            
        // 2. Внутренняя фаска для облегчения надевания
        translate([0, 0, cap_length - chamfer - wall_thickness + z_overlap])
        hull() {
            linear_extrude(eps)
                square([inner_w, inner_h], center=true);
            translate([0, 0, chamfer + z_overlap])
                linear_extrude(eps)
                    square([inner_w + 2*chamfer, inner_h + 2*chamfer], center=true);
        }
    }
}

// Сборка модели
module end_cap() {
    difference() {
        outer_shell();
        inner_cavity();
    }
}

// Рендер
color("Silver") end_cap();