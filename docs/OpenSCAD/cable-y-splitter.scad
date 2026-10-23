/* Parametric casing for a cable Y-splitter.

   Copyright 2024, CompuPhase, Thiadmer Riemersma
   This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License
   (https://creativecommons.org/licenses/by-sa/4.0/).
*/

// Diameter of the outer jacket of the central cable (including tolerance).
Central_Cable = 6.6;

// Diameter of the outer jacket of the split cables (including tolerance).
Split_Cable = 4.8;

// Height of the notches for strain relief, for all cables.
Strain_Relief = 0.8;

// Width of the casing.
Width = 35;

// Length of the casing.
Length = 40;

// Thickness of the casing.
Height = 10;

// Rounding of the three corners of the casing.
Fillet = 5;

// Chamfered edge between the top & bottom and the sides of the casing.
Chamfer = 1.5;

// Hole size for the self-tapping screws in the bottom shell. A value of 2.9 is good for 1/8 inch self-tappers.
Screw_Thread_Size = 2.9;

// Hole size for screws in the top shell; a value of 3.5 is good for 1/8 inch self-tappers.
Screw_Hole_Size = 3.5;

// The diameter of the head of the screw (a pan head is assumed).
Screw_Head = 5.5;

// Height of the screw head, or depth of the counterbored hole.
Counterbore = 2;

/* [Hidden] */
$fn = 100;
epsilon = 0.05;

module shape2D(height, inset) {
  linear_extrude(height) {
    offset(-inset) {
      hull() {
        translate([Fillet - Width/2, Length/2 - Fillet])
          circle(r = Fillet);
        translate([Width/2 - Fillet, Length/2- Fillet])
          circle(r = Fillet);
        translate([0, Fillet - Length/2])
          circle(r = Fillet);
      }
    }
  }
}

module cutout_cable(diameter, length) {
  translate([0, 0, Height/2]) {
    rotate([90, 0, 0]) {
      difference() {
        cylinder(d = diameter, h = length);
        translate([-diameter, -diameter/2, length - 5])
          cube([2*diameter, Strain_Relief, 2]);
        translate([-diameter, diameter/2 - Strain_Relief, length - 5])
          cube([2*diameter, Strain_Relief, 2]);
      }
    }
  }
}

module shape3D(top) {
  difference() {
    hull() {
      shape2D(Chamfer, Chamfer);
      translate([0, 0, Chamfer])
        shape2D(Height/2 - Chamfer, 0);
    }
    /* middle cavity (for a little extra space for the solder joints */
    hull() {
      translate([0, 0, Height/2])
        rotate([270, 0, 0])
          cylinder(d = Central_Cable, h = epsilon);
      translate([0, 0.2*Length - epsilon, Height/2])
        rotate([270, 0, 0]) scale([2, 1, 1])
          cylinder(d = Central_Cable, h = epsilon);
    }
    /* cable guides */
    cutout_cable(Central_Cable, Length/2);
    angle = atan2(Width - 3*Fillet, Length);
    len2 = Length/(2*cos(angle)) + (Split_Cable/2)*sin(angle);
    rotate([0, 0, 180 - angle])
      cutout_cable(Split_Cable, len2);
    rotate([0, 0, 180 + angle])
      cutout_cable(Split_Cable, len2);
    /* screw holes */
    edge = (Width/2 - Fillet)/2 + Fillet;
    xpos = (Central_Cable/2 + edge) / 2;
    zpos = top ? Counterbore + 0.2 : -epsilon;
    holesize = top ? Screw_Hole_Size : Screw_Thread_Size;
    translate([xpos, 0, zpos])
      cylinder(d = holesize, h = Height);
    translate([-xpos, 0, zpos])
      cylinder(d = holesize, h = Height);
    translate([0, Length/2 - Screw_Head/2 - Chamfer, zpos])
      cylinder(d = holesize, h = Height);
    if (top) {
      translate([xpos, 0, -epsilon])
        cylinder(d = Screw_Head, h = Counterbore);
      translate([-xpos, 0, -epsilon])
        cylinder(d = Screw_Head, h = Counterbore);
      translate([0, Length/2 - Screw_Head/2 - Chamfer, -epsilon])
        cylinder(d = Screw_Head, h = Counterbore);
    }
  }
}

translate([-Width/2 - 4, 0, 0])
  shape3D(false);
translate([Width/2 + 4, 0, 0])
  shape3D(true);
