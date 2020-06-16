$fn = 100;
// ABS: environ 1mm de retrait
// longueur_b = 4.10 * 25.4;
longueur_b = 113.8 + 0.9;
// largeur_b = 2.60 * 25.4;
largeur_b = 83.5 + 0.9;
hauteur_b = 60;
hauteur_b_droite = 7;
angle=4.9;
conge=4;
base_ep = 2;
murs_ep = 1.5;

hole_l = 22;
hole_x = 18;
hole_z = 24;

module hole() {
	translate([-murs_ep*2, 0, 0]) rotate([0,90,0]) union () {
		cylinder(d = hole_l, h=murs_ep * 4, $fn = 500);
		translate([0, -hole_l/2, 0]) cube([hole_z, hole_l, murs_ep * 4]);
	};
}

module coupe_conge(r) {
	translate([-(r - r * sin(angle)), -r, 0]) difference() {
		polygon(points = [
		    [0, 0],
		    [0, r],
		    [(r - r * sin(angle)), r],
		    [r, 0]
		    ]);
		circle(r = r, $fn = 500);
	};
}

module trapeze_conge(longueur, hauteur, r) {
	difference() {
	    polygon(points = [
		[-longueur/2, 0],
		[-(longueur/2 - hauteur * sin(angle)), hauteur],
		[(longueur/2 - hauteur * sin(angle)), hauteur],
		[longueur/2, 0]
		]);
	    translate([-(longueur/2 - hauteur * sin(angle)), hauteur, 0])
		rotate([0,180,0]) coupe_conge(r = r);
	    translate([(longueur/2 - hauteur * sin(angle)), hauteur, 0])
		coupe_conge(r = r);
	};
}

module pyramidal_1(longueur, largeur, hauteur, r) {
	rotate([90,0,0]) linear_extrude(height=largeur, center=true, convexity=10, twist=0)
	    trapeze_conge(longueur=longueur, hauteur=hauteur, r = r);
}

module pyramidal(longueur, largeur, hauteur, r) {
	intersection() {
		pyramidal_1(longueur, largeur, hauteur, r);
		rotate([0,0,90]) pyramidal_1(largeur, longueur, hauteur, r);
	}
}

module base(longueur, largeur, hauteur, r) {
	union() {
		translate([0,0,hauteur_b_droite / 2]) cube([longueur, largeur, hauteur_b_droite], center = true);
		translate([0,0,hauteur_b_droite]) pyramidal(longueur, largeur, hauteur_b - hauteur_b_droite, r);
	}
}
	

module box() {
	rotate([180,0,0]) difference() {
		translate([0,0,base_ep]) base(longueur_b+murs_ep*2, largeur_b+murs_ep*2, hauteur_b+base_ep, 0);
		base(longueur_b, largeur_b, hauteur_b, 4);
		translate([longueur_b/2, -largeur_b/2+hole_x, hole_z]) hole();
	};
}

box();

// difference() {
//  	box();
//  	translate([0,0,-hauteur_b / 2 - 20]) cube([longueur_b+10, largeur_b+10, hauteur_b], center=true);
// };
// hole();

// projection(cut=true) translate([0,0, -0.1]) box();
