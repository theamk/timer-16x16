// box for 16x16 timer
//  top side (closed) is where displays are
//  bottom side is closed with lid, made out of PCB

// rev 2 ("yellow") comments:
//  - Printed with 10mm brim, "high detail" mode. This was very slow,
//    but came out great.
//  - 256PX hole supports were so thick I had to switch to 12mm screws
//    which I only have in 2.6 mm dia.. so had to drill out the holes
//  - power button hole wrong (too tall) - not a real problem
//  - USB hole wrong (shifted by 1mm) - had to file it
//  - both power button and USB connection stick out of board.. so any time
//    board is installed/removed, there is a pressure on power button. 
//    It works so far, but I suspect button might fail eventually
//  - 7-segment display was a bit tricky to install due to control board
//    extra base overhanging.. but managed eventually. Maybe shorten
//    that extra base a bit.
//  - cutting corners of controls board was a pain. Maybe reduce size of PCB
//    mounting block?

WALL = 3;

// "Lip" on top, to conceal sides of red film
// walls are reduced thickness, as they are not structural
// NOTE: lip sounded like a great idea but my printer can't print it
// at h=1. So setting it to almost 0 to disable
LIP_HEIGHT = 0.001; 
LIP_WALL = 3;

// bottom lip height. walls are full thickness
// (this is offset from PCB mounting blocks to bottom of the box,
// if this is = to PCB thickness, the bottom would be even)
BOTTOM_LIP_HEIGHT = 3;

// inner X/Y is defined by PCB size, which is used as a bottom cover
INNER_X = 70 + 1; // don't forget some slop
INNER_Y = 90 + 1;
INNER_Z = 30;

// PCB mounting block hole offset from board edge
PCB_HOLE_X1 = 3.5;
PCB_HOLE_X2 = 2.5;
PCB_HOLE_Y = 3.5;
PCB_BLOCK_Z = 5;  // mounting block Z thickness
PCB_BLOCK_XY = 7; // mounting block X/Y extra 

// 7-segment (for time)
// mounting PCB is non-symmetric, but we don't bother to mode it fully,
// and instead increase sizes so that both sides fit, assuming display is centered
// (real one is smaller on one side)
PCB_Y_7SEG = 25; // height of PCB, _if_ it were symmetric
PCB_X_7SEG = 45; // width of PCB, _if_ it were symmetric
// center Y, Z offset
OFFSET_Y_7SEG = PCB_Y_7SEG / 2 - 0.1;
// top of PCB to top of display
HEIGHT_7SEG = 7.5;

// no overlap with Y- wall (except tiny one to avoid rounding errors)
assert((OFFSET_Y_7SEG - PCB_Y_7SEG/2) >= -0.1);

// 16x16 module, made out of quad 8x8
SIZE_256PX = 64; // module is square
SLOP_256PX = 0.5; // evenly spread across both sizes
THICKNESS_256PX = 13.5; // front panel to back of PCB
OFFSET_Y_256PX = INNER_Y - SIZE_256PX / 2 - 2; // Y of centerpoint of display
MNT_256PX_X_OFFS = 3; // hole offset X, from side
MNT_256PX_Y_OFFS = 6; // hole offset Y, from side
MNT_256PX_SIZE = 6;  // Y axis is twice as much

// overlap with 7-segment display is not too big (small one is OK)
assert((OFFSET_Y_7SEG + PCB_Y_7SEG/2 - 8) <= 
       (OFFSET_Y_256PX - SIZE_256PX/2));

CONTROLS_CENTER_Z = INNER_Z / 2 + 2; // slightly shifted to front
       
// Charger hole, size - need to fit "most" usb cables,
// and there is a power switch underneath too
CHARGER_HOLE_SIZE_Y = 9;
CHARGER_HOLE_SIZE_Z = 4;
CHARGER_HOLE_PCB_Y = 21;
CHARGER_HOLE_PCB_Z = 2;
// and center offset
CHARGER_HOLE_OFFSET_Y = 26.5; // PCB side to center of hole
CHARGER_HOLE_OFFSET_Z = 0.5; // PCB inner surface to bottomhole
       
// power switch - which is 7x7 button with right angle mount
POWER_HOLE_SIZE_Y = 8;
POWER_HOLE_SIZE_Z = 8;
// and center offset
POWER_HOLE_OFFSET_Y = 29; // PCB side to center of switch
POWER_HOLE_OFFSET_Z = 1.5; // PCB inner surface to bottom of button hole

/*
// battery box (which is a single AA holder with 14500 lipo inside)
// (bare battery: 53x14x14, holder: 59x17x17)
BBOX_SIZE_X = INNER_X + 0.1;
BBOX_SIZE_Y = 14;
BBOX_SIZE_Z = 14;
BBOX_WALL = 2;
*/
     
difference() {   
    // outer walls
    // Zmin = INNER_Z + BOTOMLIP_HEIGHT
    // Zmax = WALL + LIP_HEIGHT
    translate([-WALL, -WALL, -INNER_Z - BOTTOM_LIP_HEIGHT])
    cube([INNER_X + 2*WALL, INNER_Y + 2*WALL, 
          INNER_Z + WALL + BOTTOM_LIP_HEIGHT + LIP_HEIGHT]);
    
    difference() {
        // main volume for PCB
        translate([0, 0, -(INNER_Z + 100) - 0.1])
        cube([INNER_X, INNER_Y, INNER_Z + 100]);
        
        // extra base for 7-segment digits
        translate([INNER_X / 2, OFFSET_Y_7SEG, 0])
        cube([PCB_X_7SEG, PCB_Y_7SEG, HEIGHT_7SEG], center=true);
        
        // extra base control buttons backplane 
        translate([INNER_X / 2 - 65/2, -0.1, -CONTROLS_CENTER_Z - 20/2])
        cube([65, 2, 20]);

        /*
        // battery box
        translate([INNER_X / 2 - BBOX_SIZE_X / 2 - BBOX_WALL, 
                   INNER_Y - BBOX_SIZE_Y - BBOX_WALL - 0.1,
                   -INNER_Z])
        cube([BBOX_SIZE_X + BBOX_WALL * 2, 
              BBOX_SIZE_Y + BBOX_WALL, 
              BBOX_SIZE_Z + BBOX_WALL]);
        */
    }

    /*
    // battery box cutout
    translate([INNER_X / 2 - BBOX_SIZE_X / 2, 
               INNER_Y - BBOX_SIZE_Y,
               -INNER_Z - 0.1])
    cube([BBOX_SIZE_X, BBOX_SIZE_Y, BBOX_SIZE_Z + 0.1]);
    */

    // Top lip cutout
    translate([LIP_WALL-WALL, LIP_WALL-WALL, WALL])
    cube([INNER_X - 2*(LIP_WALL-WALL), INNER_Y - 2*(LIP_WALL-WALL), 100]);
        
    // cutout for 256PX (16x16) display
    translate([INNER_X / 2, OFFSET_Y_256PX, 0])
    cube([SIZE_256PX + SLOP_256PX, SIZE_256PX + SLOP_256PX, THICKNESS_256PX],
         center=true);
    
    // cutout for 7-segment digits
    // (we don't bother with mouting holes, they are drilled in place)
    translate([INNER_X / 2, OFFSET_Y_7SEG, 0])
    cube([31, 15, 100], center=true);
    
    /*
    // cutout for encoder, 25LB10Q model
    translate([INNER_X / 2, 1, -INNER_Z / 2])
    rotate([90, 0, 0]) {
        cylinder(d=8.5, h=10);  // main shft
        // anti-rotatino hole
        translate([0, -10, 0]) cylinder(d=4, h=10);
        // body - a slight dip (note contacts are 2 more mm)        
        cube([22, 22, 4], center=true);        
    }
    */
    
    // cutout for encoder, unnamed PCB-mount model
    translate([INNER_X / 2, 1, -CONTROLS_CENTER_Z]) {
        // main body
        cube([13, 20, 13], center=true);
        // expand top/bottom side for contacts
        cube([13, 3.5, 17], center=true);
    }
    
    // cutout for control buttons
    for (ox=[-1, 1])
    translate([INNER_X / 2 + ox * 8 * 2.54, 1, -CONTROLS_CENTER_Z]) {
        cube([12.5, 10, 12.5], center=true);
        // extra for light connectors
        cube([15.5, 10, 2], center=true);
    }
    
    // holes for control panel mount
    for (ox=[-1, 1])
    translate([INNER_X / 2 + ox * 4 * 2.54, 
                1, 
                -CONTROLS_CENTER_Z + ox * 1.5 * 2.54])
        rotate([90, 0, 0]) {
        // hole
        cylinder(d=2.5, h=30, center=true, $fn=10);
        // concealed head
        translate([0, 0, WALL - 1])
        cylinder(d=5, h=10, $fn=10);
        }
            
           
    // cutout for power button - a latching, lighted model
    //translate([INNER_X * 0.75, INNER_Y - 1, -INNER_Z / 2])
    //rotate([-90, 0, 0])
    //cylinder(d=12, h=10);
    
    // port for USB connection
    translate([INNER_X - 1,
               INNER_Y - CHARGER_HOLE_OFFSET_Y - CHARGER_HOLE_SIZE_Y / 2, 
               -INNER_Z + CHARGER_HOLE_OFFSET_Z])
    cube([10, CHARGER_HOLE_SIZE_Y, CHARGER_HOLE_SIZE_Z]);
    
    // a small cutout for PCB that USB connector is mounted on
    translate([INNER_X - 0.1,
               INNER_Y - CHARGER_HOLE_OFFSET_Y - CHARGER_HOLE_PCB_Y / 2, 
               -INNER_Z])
    cube([WALL - 1, CHARGER_HOLE_PCB_Y, CHARGER_HOLE_PCB_Z]);
    
    translate([-10,
               INNER_Y - POWER_HOLE_OFFSET_Y - POWER_HOLE_SIZE_Y/2, 
               -INNER_Z + POWER_HOLE_OFFSET_Z])
    cube([20, POWER_HOLE_SIZE_Y, POWER_HOLE_SIZE_Z]);
}

// 256PX mounting blocks
mount_block_hole_x = (INNER_X - SIZE_256PX) / 2 + MNT_256PX_X_OFFS;
for (ox=[-1, 1]) for (oy=[-1, 1])
    translate([INNER_X/2 - ox * INNER_X / 2, 
               oy * (SIZE_256PX - 2*MNT_256PX_Y_OFFS) / 2 + OFFSET_Y_256PX, 
               -THICKNESS_256PX - WALL])
    scale([ox, oy, 1])
    difference() {
        translate([-0.1, 
                   -(MNT_256PX_SIZE + 0.2)/2, 
                   0])
        cube([mount_block_hole_x + MNT_256PX_SIZE / 2 + 0.1, 
              2 * MNT_256PX_SIZE+0.2, 
              MNT_256PX_SIZE]);
        
        translate([mount_block_hole_x, 0, -1])
        cylinder(d=3, h=30, $fn=10);
}


// PCB mounting blocks
for (ox=[0, 1]) for (oy=[0, 1])
    translate([ox * INNER_X, oy * INNER_Y, -INNER_Z])
    scale([ (ox ? -1 : 1), (oy ? -1 : 1), 1])
    difference() {
        hole_x = (ox ? PCB_HOLE_X2 : PCB_HOLE_X1);
        translate([-0.1, 
                  PCB_HOLE_Y -PCB_BLOCK_XY / 2 - 0.1, 
                  0])
        cube([hole_x + PCB_BLOCK_XY / 2 + 0.1, 
              PCB_BLOCK_XY + 0.1, 
              PCB_BLOCK_Z]);
        
        translate([hole_x, PCB_HOLE_Y, -5])
        cylinder(d=2, h=30, $fn=10);
    }

