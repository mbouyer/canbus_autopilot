$fa = 6;
$fs = 0.5;
$fn = 100;

difference() {
    rotate_extrude(angle = 360, convexity = 100) {
	import("bouton-3mm.dxf");
    };
    translate([0,0,7]) rotate([90,0,0]) cylinder(d=4, h=15);
}
