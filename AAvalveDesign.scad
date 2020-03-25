// AA valve
// R Benedict  3/24/2020
// Anti-Asphyxia valve 
// closed when pipe pressurized
// requires very low (0-3cm) pressure  to open
// flat poppet valve with elastomer washer seat
// fits in center tap of 1" PVC T
// prototypes printed with PETG on a Prusa Reasearh MK3s
// uses 2 8mm long M3 screws and an M3 nut
// can use silicon calk to adhere body into to T
// Estimated open pressure differential:
//(poppet+retaining nut)/exposed area=120 Pa, 1.22 CM H2O *neglects friction

vr1=25/2;  // washer OD
vr2=15/2;  // washer ID 
// note washer 3.1 mm thick
vth=5;    // valve seat thickness
sr=2;  // spindle radius
sh=14; // spindle height
tht=10;  // valve seat sleeve in T
to=41/2; // T OD 41.0
ti=33.4/2; // T id 33.4
sb=3.3/2; // 3 mm clearance hole, adjust for other screws
st=2.6/2;  // 3 mm 50% tap hole, adjust for other screws 
kth=2;  // keeper thickness
kw=8;  // keeper width
klen=2*to;  // keeper length

$fn=180;

module poppet(hh, rr1, sr, sh){  // poppet valve
    union(){
    cylinder(h = hh, r = rr1);
    translate([0,0,vth])  // spindle
    cylinder(h=sh, r=sr);
    translate([0,0,vth])    // section to thread nut to
    cylinder(h=sh+3, r=st);
    }   
}

//poppet(vth, vr1-1, sr, sh);

module ring(ro, ri, th) {  // utility
    difference(){
        cylinder(h=th, r=ro);
        cylinder(h=th, r=ri);
    }
}


module seat(){  // main body
    difference(){
        union(){
            cylinder(h=vth, r=to);
            translate([0,0,vth])
            cylinder(h=tht, r=ti);
        }
        
        translate([0,0,2])  // washer seat, note undercut to retain washer
        cylinder(h=vth-2, r1=vr1, r2=vr1-.5); // pocket 3mm deep
        
        translate([0,0,vth]) // make interior
        cylinder(h=tht, r=ti-2);  
     
        cylinder(h=20, r=vr2); // throat
        
        translate([ti,0,-2])  // keeper screw holes
        cylinder(r=st,h=5);
        
        translate([-ti,0,-2])  // keeper screw holes
        cylinder(r=st,h=5); 
        
//   cube(30);   // slice to view X-section
    }

}

//seat();

module keeper(){  // poppet keeper
    difference(){
        union(){
        translate([-to,-kw/2,0])
        cube([klen,kw,kth]);  // base
        translate([0,00,0])        
        cylinder(r=sr+2,h=kth+2);  // center nub
        translate([ti,0,0])
        cylinder(r=sb+2,h=kth+2);   // side nub   
        translate([-ti,0,0])
        cylinder(r=sb+2,h=kth+2);   // side nub                             
        }
    translate([0,00,-1])       // center hole for spindle 
     cylinder(r=sr*1.05,h=10); 
        
    translate([ti,0,-kth])  // screw hole
    cylinder(r=sb,h=9);     
        
    translate([-ti,0,-kth])   // screw hole
    cylinder(r=sb,h=9);             
    }   
}

//translate([0,0,-4])
keeper();