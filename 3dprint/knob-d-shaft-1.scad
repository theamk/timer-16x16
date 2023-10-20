// a knob for unnamed encoder
// with D-shaped 6 mm shaft

SHAFT_D = 6 + 0.5; // diameter
// height of part at the end with flat
SHAFT_CUTOUT_HEIGHT = 8; 
// flat diameter (to ther end of shaft, as measured by calipers)
SHAFT_CUTOUT_DEPTH = 4.5; 

KNOB_D = 20;
KNOB_INNER_D = 17; // hide encoder body
KNOB_HEIGHT = 17; // including TOP_WALL

TOP_WALL = -0.1; 

difference() {
    // knob body, intentionally non-circular
    cylinder(d=KNOB_D, h=KNOB_HEIGHT, $fn=8);
    
    // at the bottom, there is a wide cutout
    translate([0, 0, -0.1])
    cylinder(d=KNOB_INNER_D, h=KNOB_HEIGHT - SHAFT_CUTOUT_HEIGHT - TOP_WALL + 0.1);
    
    difference() {
        // shaft hole
        cylinder(d=SHAFT_D, h=KNOB_HEIGHT - TOP_WALL, $fn=20);
        
        // "D" cutout
        translate([-10, SHAFT_CUTOUT_DEPTH - SHAFT_D/2, 0])
        cube([20, 20, 40]);
    }
}