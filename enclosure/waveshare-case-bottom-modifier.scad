// waveshare case for 2.8 display from JazzzzX
// https://www.printables.com/model/1253887-waveshare-esp32s3-28inch-display-case
// added sink holes for screws

// 2025 uliuc@gmx.net
// licensed under the Creative Commons - Attribution license
// https://creativecommons.org/licenses/by/4.0/
// Creative Commons CC BY 4.0

difference() {
     rotate([90, 0, 0])
        import("waveshare-case-bottom.stl");
        
     translate([-27, -111, 0.0])   
     linear_extrude(1.5)
        import("waveshare-case-bottom-screwterminals.svg");   
 }