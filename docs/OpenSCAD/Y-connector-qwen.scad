/* Parametric rectangular casing for a cable Y-splitter with IP67 connectors.
   
   Copyright 2024
   This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License
   (https://creativecommons.org/licenses/by-sa/4.0/).
*/

// ==================== CABLE PARAMETERS ====================
// Diameter of the outer jacket of the central cable (top, 1 cable)
Central_Cable_Diameter = 13;  // IP67 connector diameter

// Diameter of the outer jacket of the split cables (bottom, 2 cables)
Split_Cable_Diameter_1 = 13;  // Left connector diameter
Split_Cable_Diameter_2 = 13;  // Right connector diameter

// Length of cable insertion into casing
Cable_Insertion_Length = 20;

// ==================== CASING DIMENSIONS ====================
// Overall width of the rectangular casing
Casing_Width = 55;

// Overall length of the casing
Casing_Length = 70;

// Height of the casing (top to bottom)
Casing_Height = 18;

// Wall thickness
Wall_Thickness = 3;

// Corner radius for rounded corners
Corner_Radius = 8;

// ==================== INTERNAL CAVITY ====================
// Additional space inside for soldering and heat shrink
Cavity_Extra_Width = 8;
Cavity_Extra_Height = 6;

// ==================== MOUNTING HOLE ====================
// Diameter of mounting hole in top section
Mounting_Hole_Diameter = 5;
// Position from top edge
Mounting_Hole_Offset = 8;

// ==================== CABLE ENTRY POSITIONING ====================
// Distance between the two bottom cable entries (center to center)
Bottom_Cable_Spacing = 22;
// Length of parallel section before cables merge
Parallel_Section_Length = 15;

// ==================== STRAIN RELIEF ====================
// Height of the notches for strain relief
Strain_Relief_Depth = 1.5;
Strain_Relief_Width = 3;

// ==================== SCREW PARAMETERS ====================
// Hole size for self-tapping screws in bottom shell
Screw_Thread_Size = 2.9;
// Hole size for screws in top shell
Screw_Hole_Size = 3.5;
// Diameter of screw head
Screw_Head_Diameter = 5.5;
// Height of screw head / counterbore depth
Screw_Head_Height = 2;

// ==================== TEXT LABEL ====================
// Text label (max 3 characters)
TextLabel = "Y-S";
// Text height
Text_Height = 4;
// Text depth (engraved)
Text_Depth = 0.8;

// ==================== CHAMFER/FILLET ====================
// Edge chamfer
Edge_Chamfer = 1.5;

/* [Hidden] */
$fn = 50;
epsilon = 0.1;

// ==================== MODULES ====================

module rounded_rectangle_2d(width, length, radius) {
    hull() {
        translate([radius - width/2, radius - length/2])
            circle(r = radius);
        translate([width/2 - radius, radius - length/2])
            circle(r = radius);
        translate([width/2 - radius, length/2 - radius])
            circle(r = radius);
        translate([radius - width/2, length/2 - radius])
            circle(r = radius);
    }
}

module casing_outline(width, length, height, corner_radius, chamfer) {
    difference() {
        hull() {
            // Bottom section
            linear_extrude(chamfer) {
                offset(-chamfer) {
                    rounded_rectangle_2d(width, length, corner_radius);
                }
            }
            // Top section
            translate([0, 0, height/2 - chamfer])
                linear_extrude(chamfer) {
                    rounded_rectangle_2d(width, length, corner_radius);
                }
        }
    }
}

module cable_cutout(diameter, length, angle, x_offset, y_offset, strain_relief) {
    rotate([0, 0, angle]) {
        translate([x_offset, y_offset, 0]) {
            rotate([90, 0, 0]) {
                difference() {
                    cylinder(d = diameter, h = length);
                    // Strain relief notches
                    translate([-diameter, -diameter/2, length - 5])
                        cube([2*diameter, strain_relief, 2]);
                    translate([-diameter, diameter/2 - strain_relief, length - 5])
                        cube([2*diameter, strain_relief, 2]);
                }
            }
        }
    }
}

module internal_cavity() {
    cavity_width = Casing_Width - 2*Wall_Thickness + Cavity_Extra_Width;
    cavity_length = Casing_Length - 2*Wall_Thickness;
    cavity_height = Casing_Height - 2*Wall_Thickness + Cavity_Extra_Height;
    cavity_radius = max(0, Corner_Radius - Wall_Thickness);
    
    translate([0, 0, Wall_Thickness]) {
        linear_extrude(cavity_height) {
            rounded_rectangle_2d(cavity_width, cavity_length, cavity_radius);
        }
    }
}

module mounting_hole() {
    // Mounting hole at top
    translate([0, Casing_Length/2 - Mounting_Hole_Offset, Casing_Height/2]) {
        rotate([90, 0, 0]) {
            cylinder(d = Mounting_Hole_Diameter, h = Casing_Width, center = true);
        }
        // Counterbore for screw head
        rotate([90, 0, 0]) {
            cylinder(d = Screw_Head_Diameter, h = 3, center = true);
        }
    }
}

module text_label() {
    // Text on top surface
    translate([0, 0, Casing_Height/2 + 0.1]) {
        rotate([90, 0, 0]) {
            rotate([0, 0, 90]) {
                linear_extrude(Text_Depth) {
                    text(TextLabel, 
                         size = Text_Height, 
                         halign = "center", 
                         valign = "center",
                         font = "Arial:style=Bold");
                }
            }
        }
    }
}

module screw_holes(top) {
    // Position screws symmetrically
    screw_x = (Casing_Width/2 - Corner_Radius)/2;
    screw_y_bottom = -Casing_Length/4;
    screw_y_top = Casing_Length/4;
    
    hole_size = top ? Screw_Hole_Size : Screw_Thread_Size;
    
    if (top) {
        // Top shell: through holes with counterbore
        positions = [[screw_x, screw_y_bottom], 
                    [-screw_x, screw_y_bottom],
                    [0, screw_y_top]];
        
        for (pos = positions) {
            translate([pos[0], pos[1], -epsilon]) {
                cylinder(d = Screw_Head_Diameter, h = Screw_Head_Height);
            }
            translate([pos[0], pos[1], -epsilon]) {
                cylinder(d = hole_size, h = Casing_Height);
            }
        }
    } else {
        // Bottom shell: threaded holes
        positions = [[screw_x, screw_y_bottom], 
                    [-screw_x, screw_y_bottom],
                    [0, screw_y_top]];
        
        for (pos = positions) {
            translate([pos[0], pos[1], -epsilon]) {
                cylinder(d = hole_size, h = Casing_Height);
            }
        }
    }
}

module bottom_shell() {
    difference() {
        casing_outline(Casing_Width, Casing_Length, Casing_Height, Corner_Radius, Edge_Chamfer);
        
        // Internal cavity
        internal_cavity();
        
        // Central cable entry (top) - parallel section
        translate([0, Casing_Length/2 - Parallel_Section_Length/2, 0]) {
            cable_cutout(Central_Cable_Diameter, 
                        Cable_Insertion_Length + Parallel_Section_Length, 
                        0, 0, 0, 
                        Strain_Relief_Depth);
        }
        
        // Bottom cable entries - parallel then merge
        // Left cable
        translate([0, -Casing_Length/2 + Parallel_Section_Length/2, 0]) {
            cable_cutout(Split_Cable_Diameter_1, 
                        Cable_Insertion_Length, 
                        0, 
                        -Bottom_Cable_Spacing/2, 0, 
                        Strain_Relief_Depth);
        }
        
        // Right cable
        translate([0, -Casing_Length/2 + Parallel_Section_Length/2, 0]) {
            cable_cutout(Split_Cable_Diameter_2, 
                        Cable_Insertion_Length, 
                        0, 
                        Bottom_Cable_Spacing/2, 0, 
                        Strain_Relief_Depth);
        }
        
        // Merge cavity for Y-junction
        translate([0, -Casing_Length/6, Wall_Thickness]) {
            cube([Casing_Width - 2*Wall_Thickness, 
                  Parallel_Section_Length + 10, 
                  Casing_Height - 2*Wall_Thickness + Cavity_Extra_Height], 
                 center = true);
        }
        
        // Mounting hole
        mounting_hole();
        
        // Screw holes
        screw_holes(false);
    }
}

module top_shell() {
    difference() {
        casing_outline(Casing_Width, Casing_Length, Casing_Height, Corner_Radius, Edge_Chamfer);
        
        // Internal cavity (slightly larger for fit)
        hull() {
            internal_cavity();
            translate([0, 0, 0.2])
                internal_cavity();
        }
        
        // Cable passages (openings)
        // Top cable
        translate([0, Casing_Length/2 - Parallel_Section_Length/2, 0]) {
            cable_cutout(Central_Cable_Diameter + 0.5, 
                        5, 
                        0, 0, 0, 
                        0);
        }
        
        // Bottom cables
        translate([0, -Casing_Length/2 + Parallel_Section_Length/2, 0]) {
            cable_cutout(Split_Cable_Diameter_1 + 0.5, 
                        5, 
                        0, 
                        -Bottom_Cable_Spacing/2, 0, 
                        0);
        }
        
        translate([0, -Casing_Length/2 + Parallel_Section_Length/2, 0]) {
            cable_cutout(Split_Cable_Diameter_2 + 0.5, 
                        5, 
                        0, 
                        Bottom_Cable_Spacing/2, 0, 
                        0);
        }
        
        // Mounting hole through
        mounting_hole();
        
        // Screw holes
        screw_holes(true);
        
        // Text label (engraved)
        text_label();
    }
}

// ==================== RENDERING ====================

// Bottom shell (left)
translate([-Casing_Width/2 - 10, 0, 0])
    bottom_shell();

// Top shell (right)
translate([Casing_Width/2 + 10, 0, 0])
    top_shell();