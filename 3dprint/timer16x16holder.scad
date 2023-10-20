// a holder to mount256px display
// print 2 of those, insert between display and PCB
// works together with "timer16x16box" model


// from timer16x16box.scad
SIZE_256PX = 64; // module is square
MNT_256PX_Y_OFFS = 6; // hole offset Y, from side

// defined here
SLOP = 1;
WIDTH = 5;
THICKNESS = 4;

difference() {
    cube([WIDTH, SIZE_256PX - SLOP, THICKNESS], center=true);
    for (oy=[-1, 1]) 
      scale([1, oy, 1]) {
        // two holes in the middle are attaching holder to display
        // they are for making install easier only
        translate([0, MNT_256PX_Y_OFFS, 0])
        cylinder(d=1.8, h=10, center=true, $fn=10);
        
        // two holes in the sides are important ones, they accept
        // mounting screws
        translate([0, SIZE_256PX/2 - MNT_256PX_Y_OFFS, 0])
        cylinder(d=1.8, h=10, center=true, $fn=10);
    }
}    


