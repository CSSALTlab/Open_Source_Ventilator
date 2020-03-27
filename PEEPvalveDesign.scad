// PEEP valve
// R Benedict  3/27/2020
// PEEP valve 
// closed when pipe not pressurized
// requires very low (0-3cm) positive pressure  to open
// flat poppet valve with elastomer washer seat
// fits in center tap of 1" PVC T
// prototypes printed with PETG on a Prusa Reasearh MK3s
// uses 6-32 screws and an 6-32 nuts
// can use silicon calk to adhere body into to T
// Estimated open pressure differential:
// 
//area mm2 poppet g nuts	nuts g	total g	  Pa	 cm H2O
// 176.72	1.74	 0   	  0     1.74    96.49	0.98
// 176.72	1.74	 1    	0.83	2.57	142.71	1.46
// 176.72	1.74	 2   	1.67	3.41	188.92	1.93
// 176.72	1.74	 3   	2.50	4.24	235.14	2.40


vr1=25/2;  // washer OD
vr2=15/2;  // washer ID 
// note washer 3.1 mm thick
vth=8;    // valve seat thickness
pth=4;  // poppet thickness
sr=2.5;  // spindle radius
sh=9; // spindle height
tht=10;  // valve seat sleeve in T
to=41/2; // T OD 41.0
so=to+2; // seat outside radius
ti=33.4/2; // T id 33.4
sb=3.7/2; // 6-32 clearance hole, adjust for other screws
st=2.8/2;  // 3 mm 50% tap hole, adjust for other screws 
kth=4;  // keeper thickness
kw=10;  // keeper width
klen=2*so;  // keeper length

$fn=180;

module poppet(hh, rr1, sr, sh){  // poppet valve
    union(){
    cylinder(h = hh, r = rr1);
    translate([0,0,pth])  // spindle
    cylinder(h=sh, r=sr);
    translate([0,0,pth])    // section to thread nut to
    cylinder(h=sh+8, r=st);
    }   
}

// poppet(pth, vr1-1, sr, sh);

module ring(ro, ri, th) {  // utility
    difference(){
        cylinder(h=th, r=ro);
        cylinder(h=th, r=ri);
    }
}


module seat(ssr){  // main body
    difference(){
        union(){
            cylinder(h=vth, r=so);
            translate([0,0,vth])
            cylinder(h=tht, r=ti);
        }
        
        translate([0,0,3])  // washer seat, note undercut to retain washer
        rotate([0,180,0])
        cylinder(h=vth-2, r1=vr1, r2=vr1-.5); // pocket 3mm deep
        
        translate([0,0,vth]) // make interior
        cylinder(h=tht, r=ti-2);  
     
        cylinder(h=20, r=vr2); // throat
        
        translate([ti+2,0,0])  // keeper screw holes
        cylinder(r=ssr,h=vth);        
        translate([-ti-2,0,0])  // keeper screw holes
        cylinder(r=ssr,h=vth); 
        
        rotate([0,0,90])
        translate([ti+2,0,0])  // assemby screw holes
        cylinder(r=ssr,h=vth);
        rotate([0,0,90])        
        translate([-ti-2,0,0])  // assembly screw holes
        cylinder(r=ssr,h=vth);        
//   cube(30);   // slice to view X-section, comment out to print
    }
}


//seat(sb);

module seat_top(ssr){
    difference(){
        seat(ssr);
        translate([0,0,3])
        cylinder(h=25,r=so+1);  // uncomment to make top
    }   
}


//seat_top(sb);


module seat_bottom(ssr){  //  make bottom
    difference(){
       seat(ssr);
       cylinder(h=3,r=so+1); 
    }   
}


//seat_bottom(st);

module keeper(){  // poppet keeper
    difference(){
        union(){
        translate([-so,-kw/2,0])
        cube([klen,kw,kth]);  // base
 //       translate([0,00,0])        
 //       cylinder(r=sr+2,h=kth+2);  // center nub
        translate([ti+2,0,0])
        cylinder(r=sb+2,h=kth+8);   // side nub   
        translate([-ti-2,0,0])
        cylinder(r=sb+2,h=kth+8);   // side nub                             
        }
    translate([0,00,-1])       // center hole for spindle 
    cylinder(r=sr*1.07,h=10); 
        
    translate([ti+2,0,-1])  // screw hole
    cylinder(r=sb,h=15);     
        
    translate([-ti-2,0,-1])   // screw hole
    cylinder(r=sb,h=15);             
    }   
}



//keeper();

module assembly(){
    rotate([0,180,0])
    poppet(pth, vr1-1, sr, sh); 
    seat_top();   
    seat_bottom();   
    translate([0,0,-12])
    keeper();    
}

assembly();