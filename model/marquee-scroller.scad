// (c) 2020 Gergo Torcsvari
// This code is licensed under MIT license (see LICENSE.txt for details)

//pael dimensions
panelZ = 13;
panelY = 33.4;
panelX = 128;

//for bottom layer config
layerHeight = 0.2;
minZWall =  layerHeight*4;

//for wall and additional dimensions
normalWallW = 2;
additionalSpacers = 25;
topSpace = 30;

//USB hole dimensions
usbW = 12; //7.2
usbH = 6; //4

//helper constants
holderW = 4;
err = 0.001;

//uncomment below to visualize the panel
//translate([additionalSpacers,normalWallW,minZWall]) color("red") cube([panelX, panelY, panelZ]);

//test print helper, uncommennt the cube and base/top for test renders
//uncommennt only the top/base to render it
intersection(){
//cube([30, 100, 100]);
base();
//top();
}

//////////////////////////////////////

module nute() {
    difference() {
        rotate([45,0,0])
           cube([2*additionalSpacers-2*normalWallW+panelX+err, sqrt(2)*holderW,  sqrt(2)*holderW]);
        translate([-err,-holderW, 0]) cube([2*additionalSpacers-normalWallW+panelX+3*err, 2*holderW,  holderW-err]);
    }
}

module top() {
  difference(){
    union(){
      translate([0, normalWallW, minZWall])
        cube([normalWallW, panelY, panelZ+topSpace]);
      translate([0, normalWallW+holderW+layerHeight, minZWall+panelZ+topSpace-holderW])
        cube([2*additionalSpacers+panelX-normalWallW-layerHeight, panelY-2*holderW-2*layerHeight, holderW]);
      translate([0,normalWallW-holderW+panelY-layerHeight,minZWall+panelZ+topSpace-2*holderW])
        nute();
      translate([0,normalWallW+holderW+layerHeight,minZWall+panelZ+topSpace-2*holderW])
        nute();
    }
    translate([-err,panelY/2 + normalWallW-usbW/2, minZWall+panelZ+topSpace-2*holderW-2*layerHeight-usbH/2+2]) cube([12+err,usbW,usbH]);  
  }
}

module base() {
    difference() {
      cube([panelX+2*additionalSpacers, panelY+2*normalWallW, minZWall + panelZ + topSpace]);
      translate([-err,normalWallW,minZWall]) 
        cube([additionalSpacers+panelX+err, panelY, panelZ]);
      translate([-err,normalWallW,minZWall+panelZ + holderW]) 
        cube([additionalSpacers+panelX+err, panelY, topSpace - 3*holderW]);
      translate([-err,normalWallW+holderW,minZWall]) 
        cube([2*additionalSpacers-normalWallW+panelX+err, panelY-2*holderW, panelZ+topSpace + err]);
      translate([-err+normalWallW,normalWallW+holderW,minZWall+panelZ-holderW]) 
        nute();
      translate([-err+normalWallW,normalWallW-holderW+panelY,minZWall+panelZ-holderW]) 
        nute();
      translate([-err+normalWallW,normalWallW+holderW,minZWall+panelZ+topSpace-2*holderW])
           nute();
      translate([-err+normalWallW,normalWallW-holderW+panelY,minZWall+panelZ+topSpace-2*holderW])
           nute();
       translate([-err+normalWallW,normalWallW+holderW,minZWall+panelZ+topSpace-3*holderW])
           nute();
      translate([-err+normalWallW,normalWallW-holderW+panelY,minZWall+panelZ+topSpace-3*holderW])
           nute();
      translate([-err, normalWallW, minZWall])
        cube([normalWallW+err, panelY, minZWall+panelZ+topSpace]);
    }
}
