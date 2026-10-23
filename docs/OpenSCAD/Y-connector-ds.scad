/* Parametric casing for a parallel-input Y-splitter.
   Fixed cable channels and added engraved label.
*/

// --- Diameters of the cables ---
Central_Cable = 6.6;     // Output cable
Split_Cable = 4.8;       // Input cables
Cable_Spacing = 12.0;    // Center-to-center distance between the two input cables

// --- Casing Dimensions ---
Width = 35;
Length = 40;
Height = 10;
Fillet = 5;
Chamfer = 1.5;

// --- Hardware (Screws) ---
Screw_Thread_Size = 2.9;
Screw_Hole_Size = 3.5;
Screw_Head = 5.5;
Counterbore = 2;

// --- Label Settings ---
Label_Text = "DFC";      // Up to 3 characters
Label_Size = 8;          // Font size
Label_Y_Offset = -6;     // Shift text towards the output cable
Label_Depth = 0.8;       // Engraving depth

/* [Hidden] */
$fn = 100;
epsilon = 0.05;

module shape2D(height, inset) {
  linear_extrude(height) {
    offset(-inset) {
      hull() {
        translate([Fillet - Width/2, Length/2 - Fillet]) circle(r = Fillet);
        translate([Width/2 - Fillet, Length/2- Fillet]) circle(r = Fillet);
        translate([0, Fillet - Length/2]) circle(r = Fillet);
      }
    }
  }
}

// Универсальный модуль для вырезания канала
module channel(d, len) {
  translate([0, 0, Height/2])
  rotate([90, 0, 0])
  difference() {
    cylinder(d=d, h=len, center=true);
    // Насечки (strain relief)
    for (y = [-len/2 + 3, len/2 - 3]) {
      translate([-d/2, y, 0])
        cube([d, 1.5, d], center=true);
    }
  }
}

module shape3D(top) {
  difference() {
    // Внешний корпус
    hull() {
      shape2D(Chamfer, Chamfer);
      translate([0, 0, Chamfer]) shape2D(Height/2 - Chamfer, 0);
    }
    
    // Внутренняя полость для пайки
    hull() {
      translate([0, -Length/4, Height/2])
        rotate([90, 0, 0]) cylinder(d = Central_Cable, h = epsilon, center=true);
      translate([0, Length/4, Height/2])
        rotate([90, 0, 0]) scale([(Cable_Spacing + Split_Cable)/Central_Cable, 1, 1]) 
        cylinder(d = Central_Cable, h = epsilon, center=true);
    }
    
    // --- КАНАЛЫ ДЛЯ КАБЕЛЕЙ ---
    // 1. Выходной кабель (снизу, строго по центру)
    translate([0, -Length/4, 0])
      channel(Central_Cable, Length/2 + 5);
      
    // 2. Входные кабели (сверху, строго параллельно)
    translate([-Cable_Spacing/2, Length/4, 0])
      channel(Split_Cable, Length/2 + 5);
    translate([Cable_Spacing/2, Length/4, 0])
      channel(Split_Cable, Length/2 + 5);
    
    // --- ОТВЕРСТИЯ ПОД ВИНТЫ ---
    edge = (Width/2 - Fillet)/2 + Fillet;
    xpos = (Central_Cable/2 + edge) / 2;
    zpos = top ? Counterbore + 0.2 : -epsilon;
    holesize = top ? Screw_Hole_Size : Screw_Thread_Size;
    
    translate([xpos, 0, zpos]) cylinder(d = holesize, h = Height);
    translate([-xpos, 0, zpos]) cylinder(d = holesize, h = Height);
    translate([0, Length/2 - Screw_Head/2 - Chamfer, zpos]) cylinder(d = holesize, h = Height);
    
    if (top) {
      // Зенковка для шляпок винтов
      translate([xpos, 0, -epsilon]) cylinder(d = Screw_Head, h = Counterbore);
      translate([-xpos, 0, -epsilon]) cylinder(d = Screw_Head, h = Counterbore);
      translate([0, Length/2 - Screw_Head/2 - Chamfer, -epsilon]) cylinder(d = Screw_Head, h = Counterbore);
      
      // --- ГРАВИРОВКА НАДПИСИ ---
      translate([0, Label_Y_Offset, Height/2 - Label_Depth])
        linear_extrude(height = Label_Depth + 0.1)
        text(Label_Text, size = Label_Size, font = "Arial:style=Bold", halign = "center", valign = "center");
    }
  }
}

// Рендер двух половинок
translate([-Width/2 - 4, 0, 0]) color("LightBlue") shape3D(false);
translate([Width/2 + 4, 0, 0]) color("LightGreen") shape3D(true);