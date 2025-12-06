// waveshare case for 2.8 display from JazzzzX
// https://www.printables.com/model/1253887-waveshare-esp32s3-28inch-display-case

// gamepad based on the great work of tahtvjd1998 (gamepad version for two joystick)
// https://www.thingiverse.com/thing:4070026
// modified to fit a popular joystick module

// 2025 uliuc@gmx.net
// licensed under the Creative Commons - Attribution license
// https://creativecommons.org/licenses/by/4.0/
// Creative Commons CC BY 4.0

walls=3; // wall thickness
wall_wiggle=0.5;
epsilon=0.01;
big_epsilon=1;
js_yoff=-30; // offset of the object with joystick holes
js_to_wall_yoff=4; // forward offset
js_radius=27.5/2; // radius of the circles
js_board_side=60; // size of the rectangular base with rounded edges
dist_between_js=35; // distance between joystick holes
box_zoff=2;
heights=[3,21.5,3];
dist_between_dpad=0; // front part of the joystick for buttons if needed
dpad_yoff=-28; // move the front part for the buttons
dist_between_handles=95; // distance between handles
handles_yoff=-41.0; // handles along Y
handles_zoff=30; // handles along Z
handle_length=65; // handle length
handle_xrot=25; // handles rotation along X
handle_zrot=18; // handle rotation
$fn=80;

module handle(dir=1)
{
    rotate(handle_zrot*dir)
        scale([1,1,0.75])
            hull()
            {
                big=30;
                small=20;
                translate([0,handle_length/2,0])
                    sphere(r=big);
                translate([0,-handle_length/2,0])
                    sphere(r=small);
            }
}

module handles()
{
    difference()
    {
        rotate([handle_xrot,0,0])
            translate([0,0,handles_zoff])
            {
                translate([dist_between_handles/2,handles_yoff,0])
                    handle(1);
                translate([-dist_between_handles/2,handles_yoff,0])
                    handle(-1);
            }
        top(0);
    }
}

module insides()
{
    hull()
    {
        real_dist_between_js=dist_between_js+js_radius*2-30;
        w=js_board_side;
        translate([-(real_dist_between_js+w)/2,js_yoff-w/2+js_to_wall_yoff])
            square(size=[w,w]);
        translate([(real_dist_between_js-w)/2,js_yoff-w/2+js_to_wall_yoff])
            square(size=[w,w]);
        translate([dist_between_dpad/2,dpad_yoff])
            circle(r=25);
        translate([-dist_between_dpad/2,dpad_yoff])
            circle(r=25);
    }
}

module top_holes()
{
    // joystick
    import("joystick_template.svg");
}

module middle(hollow,h_override=0,z_override=0,inset=0)
{
    translate([0,0,z_override])
        linear_extrude(height=heights[1]+h_override)
            difference()
            {
                difference()
                {
                    offset(walls-inset)
                        insides();
                    if(hollow!=0)
                        offset(inset)
                            insides();
                }
            }
}

module screw_terminal(height, top=0)
{
   if (top == 0) {
     
     difference() {
        cylinder(r = 3, h = height);
        cylinder(r = 1, h = height-10); 
     }
   } else if (top == 1) {
   
       translate([0, 0, -3])
        cylinder(r = 1, h = height);
       translate([0, 0, -3])
        cylinder(r = 2, h = 2);
       
   }
}

module screw_terminals(height, top=0)
{
    if (top == 0) {
        translate([-42.9,-53.1,5])
            screw_terminal(height, top);
    } else {
 
         translate([-43,-52,-8])
           screw_terminal(height+13, top);    
    }
    translate([42.9,0.9,5])
        screw_terminal(height, top);
}

module face(holes)
{
        difference()
        {
               linear_extrude(height=heights[2])
                    offset(walls)
                        insides();
            
            if(holes!=0)
            {
                    giant_epsilon=2;
                    
                    translate([-40, -46, -giant_epsilon])
                        linear_extrude(height=heights[2]+giant_epsilon*2)
                            top_holes();
               
            }
        }
        
}

module top(hollow=1)
{
    translate([0,0,box_zoff])
        difference()
        {
            union()
            {
                face(0);
                middle(hollow);
                translate([0,0,heights[1]-epsilon])
                        face(hollow);
            }
        }
}

module gamepad(part=0)
{
    if(part==0)
    {
      union() {
        difference()
        {
            union()
            {
                handles();
                top();
            }
            middle(0,30,-31,walls+epsilon);
            cutw=250;
            cuth=200;
            cutz=100;
            translate([-cutw/2,-cuth/2,heights[1]+heights[2]])
                cube(size=[cutw,cuth,cutz]);
        }
        screw_terminals(19, 0);
     }
    }
    else if(part==1)
    {
        difference()
        {
            intersection()
            {
                union()
                {
                    handles();
                    top();
                }
                middle(0,30,-31,walls+epsilon+wall_wiggle);
            }
          screw_terminals(3, 1); 
        }
    }
}

module gamepad_display() {

    difference() {
    
        translate([0, 33.6, 35])
             rotate([90, 180, 90]) import("waveshare-case-top.stl");
        
        // cable hole
        translate([+20, 0, 19.4])
            cube([6, 20, 4]);
    }

}

module gamepad_solo() {

  difference() {
 
    union() {
    
        gamepad(0);
        
          // stege
          difference() {
             translate([-40, -46-1.25, 15.5])
                   linear_extrude(height=8)
                         import("joystick_terminals.svg");
              translate([-38.0, -44.0-1.25, 15.5])
                   linear_extrude(height=8)
                         import("joystick_terminals_holes.svg");
          }
    }
     
    // cable hole
    translate([+20, 0, 19.4])
         cube([6, 20, 4]);
  }
}

module gamepad_lid() {
    gamepad(1);
}

//gamepad_display();
gamepad_solo();
//gamepad_lid();

