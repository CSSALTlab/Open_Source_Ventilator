// OP and PEEP valves, V 3
// R benedict 3/29/2020
// closed when pipe not pressurized
// requires positive pressure  to open
// flat poppet valve with elastomer washer seat
// fits in center tap of 1" PVC T
// prototypes printed with PETG on a Prusa Reasearh MK3s
// printed in PETG with .2 mm nozzle, 3 perimiters, brim on screw
// used 3.36 m filament, 10.26 g
// uses 6-32 screws and an 6-32 nuts, 3/8 and 1 1/4
// spring od 8, id 6.8, thin wire
// used 10 mm length fot PEEP
// used 15 mm length for Over Pressure
// ** threaded version - screws into 1" PVC pipe thread **
// print seat bottom, seat top, keeper, poppet
// uncomment module name to generate geometry
// uncomment "assembly" to see system - may be slow rendering
to=41/2; // T OD 41.0
so=to+2; // seat outside radius
ti=33.4/2; // T id 33.4
sb=3.7/2; // 6-32 clearance hole, adjust for other screws
st=2.8/2;  // 3 mm 50% tap hole, adjust for other screws 
vr1=25/2;  // washer OD
vr2=15/2;  // washer ID 
// note washer 3.1 mm thick
vth=8.2;  // valve seat thickness, thickened slightly to use 3/8" screw
tth=3;  // top slice thickness, should be about washer thickness
bth=vth-tth; // bottom slice thickness
tht=12;  // valve seat sleeve in T, works for Ts I have
d=32.6;  // 1" NPT thread OD
Tang=30; // T thread angle
di=d/2-4; // threaded part ID 
pitch=2.21;  // 1" NTP thread pitch
vtravel=5;  // poppet travel
klen=2*so; // length keeper bar
kw=16;  // width keeper bar
kth=6;  // tickness keeper bar
scd=10;  // screw diameter
sl=4*vtravel;  // screw length
step=3; // screw thread step
sang=45; // screw thread angle
pth=3;  // poppet thickness
kc=2*vtravel+pth+3;   // keeper clearance
sh=kc+kth+vtravel/2; // spindle height 
sr=2.5;  // spindle radius
sclear=1.03; // clearance

include <Poor_mans_openscad_screw_library/polyScrewThread.scad>
PI=3.141592;
$fn=90; // number of fragments, increase for smoother curves

/* Example 01.
 * Just a 100mm long threaded rod.
 *
 * screw_thread(15,   // Outer diameter of the thread
 *               4,   // Step, traveling length per turn, also, tooth height, whatever...
 *              55,   // Degrees for the shape of the tooth 
 *                       (XY plane = 0, Z = 90, btw, 0 and 90 will/should not work...)
 *             100,   // Length (Z) of the tread
 *            PI/2,   // Resolution, one face each "PI/2" mm of the perimeter, 
 *               0);  // Countersink style:
 *                         -2 - Not even flat ends
 *                         -1 - Bottom (countersink'd and top flat)
 *                          0 - None (top and bottom flat)
 *                          1 - Top (bottom flat)
 *                          2 - Both (countersink'd)
 */
// screw_thread(15,4,55,100,PI/2,1);


module seat(ssr){  // main body
    difference(){
        union(){
            cylinder(h=vth, r=so);
            translate([0,0,vth])
            screw_thread(d,pitch,Tang,tht,PI/2,1);  
        }
        
        translate([0,0,3])  // washer seat, note undercut to retain washer
        rotate([0,180,0])
        cylinder(h=vth-2, r1=vr1, r2=vr1-.5); // pocket 3mm deep
        
        translate([0,0,vth]) // make interior
        cylinder(h=tht+2, r=di);  
     
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
        translate([0,0,tth])
        cylinder(h=50,r=so+1);  // cut off bottom
    }   
}


//seat_top(sb);


module seat_bottom(ssr){  //  make bottom
    difference(){
       seat(ssr);
       cylinder(h=tth,r=so+1); // cut off top
    }   
}


//seat_bottom(st);

module keeper(){  // poppet keeper, flip over to print
    difference(){
    union(){
         translate([0,0,kc])    
            union(){
                difference(){
                translate([-klen/2, -kw/2,0])
                cube([klen,kw,kth]);
                cylinder(h=kth, r=scd/2+2);
                    }
            hex_nut(15,kth,step,sang,scd*sclear,0.5);
        
            }
             translate([ti+2,0,0])
        cylinder(r=sb+2,h=kc);   // side nub   
        translate([-ti-2,0,0])
        cylinder(r=sb+2,h=kc);   // side nub                                    

    } 
 //   translate([0,00,-1])       // center hole for spindle 
 //  cylinder(r=sr*1.07,h=10); 
        
    translate([ti+2,0,-1])  // screw hole
    cylinder(r=sb,h=kth+kc+2);     
        
    translate([-ti-2,0,-1])   // screw hole
    cylinder(r=sb,h=kth+kc+2);             
    }   
}



//keeper(); // note - flip over in slicer to print without supports

module screw(){
    difference(){
        union(){
            cylinder(r=scd/2+1,h=1);
          translate([0,0,1])  
        screw_thread(scd,step,sang,sl,PI/4,1);
  
        }
        cylinder(r=sr*1.05,h=klen);
    }
}  
//screw();

module poppet(hh, rr1, sr, sh){  // poppet valve
    union(){
    cylinder(h = hh, r = rr1);
    translate([0,0,pth])  // spindle
    cylinder(h=sh, r=sr);
    }   
}

//poppet(pth, vr1-1, sr, sh);

module assembly(){ // visualize whole system

    rotate([0, 180,0])
    seat();
    translate([0,0,vtravel+pth])    
    screw();
    keeper();
    poppet(pth, vr1-1, sr, sh);
}

// assembly();
