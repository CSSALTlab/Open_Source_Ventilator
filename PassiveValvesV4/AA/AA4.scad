// AA valve
// R Benedict  3/24/2020
// Anti-Asphyxia valve 
// closed when pipe pressurized
// requires very low (0-3cm) pressure  to open
// flat poppet valve with elastomer washer seat
// fits in center tap of 3/4" PVC T
// prototypes printed with PETG on a Prusa Reasearh MK3s
// uses 2 8mm long 6-32 screws and an 6-32 nut
// Estimated open pressure differential:
//(poppet+retaining nut)/exposed area=120 Pa, 1.22 CM H2O *neglects friction
// ** threaded version - screws into 3/4" PVC pipe thread **
// print seat, keeper, poppet
// uncomment module name to generate geometry
// uncomment "assembly" to see system - may be slow rendering
d=25.6;  // 3/4" NPT thread OD
Tang=30; // T thread angle
ir=d/2-3; // threaded part IR
pitch=1.81;  // 1" NTP thread pitch
vr1=25/2;  // washer OD
vr2=15/2;  // washer ID 
// note washer 3.1 mm thick
vth=5;    // valve seat thickness
vtb=4; // valve seat bottom
pth=2;   // poppet thickness
sr=3;  // spindle radius
sh=14; // spindle height
tht=10;  // valve seat sleeve in T
to=20;  //41/2; // T OD 41.0
ti=33.4/2; // T id 33.4
trh=6;
sb=3.7/2; // 6-32 clearance hole, adjust for other screws
st=2.8/2;  // 6-32 50% tap hole, adjust for other screws 
kth=2;  // keeper thickness
kw=10;  // keeper width
klen=2*to+2;  // keeper length

include <Poor_mans_openscad_screw_library/polyScrewThread.scad>
PI=3.141592;
$fn=90; // number of fragments, increase for smoother curves


module poppet(hh, rr1, sr, sh){  // poppet valve
    union(){
    cylinder(h = hh, r = rr1); // base
    translate([0,0,pth])  // spindle
    cylinder(h=sh, r=sr);
    translate([0,0,pth])    // section to thread nut to
    cylinder(h=sh+3, r=st);
    }   
}

//poppet(pth, vr1-1, sr, sh);


module seatTop(){  // valve seat top 
    difference(){
            cylinder(h=vth, r=to);

        
        translate([0,0,2]) //washer seat, note undercut to retain washer
        cylinder(h=vth-2, r1=vr1, r2=vr1-.5); // pocket 3mm deep
           
        cylinder(h=20, r=vr2); // throat
        
        rotate([0,0,90])
        translate([ti,0,-2])  // assembly  screw holes
        cylinder(r=sb,h=9);
 
        rotate([0,0,90])       
        translate([-ti,0,-2])  // asssembly  screw holes
        cylinder(r=sb,h=9); 
        
        translate([ti,0,-2])  // keeper screw holes
        cylinder(r=sb,h=9);
        
        translate([-ti,0,-2])  // keeper screw holes
        cylinder(r=sb,h=9); 
        
//   cube(30);   // slice to view X-section
    }
}

seatTop();

module seatBot() {
    difference(){
        union(){
            cylinder(h=vtb, r=to); // flange
            translate([0,0,vtb])
            cylinder(r1=ti, r2=d/2, h=trh);  // flare
            translate([0,0,vtb+trh])
            screw_thread(d,pitch,Tang,tht,PI/2,1);   // thread
        }
        cylinder(h=vth+1, r=vr1+1);  // poppet pocket
        translate([0,0,vth+1])
        cylinder(r1=vr1+1, r2=ir, h=4);   // flare
        cylinder(h=20, r=ir);  // threaded part inside
    
        rotate([0,0,90])
        translate([ti,0,0])  // assembly  screw holes
        cylinder(r=st,h=9);
 
        rotate([0,0,90])       
        translate([-ti,0,0])  // asssembly  screw holes
        cylinder(r=st,h=9); 
        
        translate([ti,0,0])  // keeper screw holes
        cylinder(r=st,h=9);
        
        translate([-ti,0,0])  // keeper screw holes
        cylinder(r=st,h=9);     
        
//        cube(30);   // slice to view X-section
    }    
}
//seatBot();

module keeper(){  // poppet keeper
    difference(){
        union(){
        translate([-klen/2+1,-kw/2+1,0])
            minkowski()   /// top bar, round corners
                {
                  cube([klen-2,kw-2,kth-1]);
                  cylinder(r=1,h=1);
                } 
        cylinder(r=sr+2,h=kth+2);  // center nub
        translate([ti,0,0])
        cylinder(r=sb+2,h=kth+2);   // side nub   
        translate([-ti,0,0])
        cylinder(r=sb+2,h=kth+2); // side nub                             
        }
    translate([0,00,-1])       // center hole for spindle 
     cylinder(r=sr*1.05,h=10); 
        
    translate([ti,0,-kth])  // screw hole
    cylinder(r=sb,h=9);     
        
    translate([-ti,0,-kth])   // screw hole
    cylinder(r=sb,h=9);             
    }   
}


//keeper();

module assembly(){
    translate([0,0,vth])
    rotate([180,0,0])
    seatTop(); 
    rotate([180,0,0]) 
  seatBot();
    translate([0,0,-pth])
    poppet(pth, vr1-1, sr, sh);
    translate([0,0,vth+kth+2])
    rotate([0,180,0])    
     keeper();
}

//assembly();