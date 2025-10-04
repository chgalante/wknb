// =============================================================================
// Sketch (in mm)
// =============================================================================

BOARD_WIDTH=79;
BOARD_HEIGHT=58;
BOARD_THICKNESS=2;
CLEARANCE_BOTTOM=10;
CLEARANCE_TOP=10;
CLEARANCE_XY=2;

ENCLOSURE_THICKNESS=2;
ENCLOSURE_BOARD_CLEARANCE_XY=2;
ENCLOSURE_WIDTH= BOARD_WIDTH + (ENCLOSURE_BOARD_CLEARANCE_XY*2);
ENCLOSURE_HEIGHT= BOARD_HEIGHT + (ENCLOSURE_BOARD_CLEARANCE_XY*2);

HOLE_DIAMETER=3;
HOLE_DIST_HEIGHT=50;
HOLE_DIST_WIDTH=71;

KNOB_HOLE_DIAMETER=7;
KNOB_HOLE_CLEARANCE=1;
KNOB_HOLE_OFFSET_X=39;
KNOB_HOLE_OFFSET_Y=28;

USB_PORT_HEIGHT=3;
USB_PORT_WIDTH=8;
USB_HEIGHT_OFFSET=5;
USB_WIDTH_OFFSET=15;

module main() {
  sketch();
}
main();

module sketch() {

  $fn=32;

  // enclosure
  translate([-ENCLOSURE_BOARD_CLEARANCE_XY, -ENCLOSURE_BOARD_CLEARANCE_XY, 0]) {
    color("red") {
      square([ENCLOSURE_WIDTH,ENCLOSURE_HEIGHT], false);
    }
  }

  // board 
  square([BOARD_WIDTH,BOARD_HEIGHT], false);

  // USB OPENING
  translate([-USB_WIDTH_OFFSET, 0, 0]) {
    translate([-ENCLOSURE_BOARD_CLEARANCE_XY, -ENCLOSURE_BOARD_CLEARANCE_XY, 0]) {
      translate([ENCLOSURE_WIDTH, ENCLOSURE_HEIGHT, 0]) {
        translate([-USB_PORT_WIDTH, -2, 0]) {
          color("yellow"){
            square([USB_PORT_WIDTH, 2], false);
          }
        }
      }
    }
  }

  // KNOB HOLE
  translate([KNOB_HOLE_OFFSET_X, KNOB_HOLE_OFFSET_Y, 0]) {
    color("yellow") {
      circle(d=KNOB_HOLE_DIAMETER+KNOB_HOLE_CLEARANCE);
    }
  }

  // MOUNTING HOLES
  EDGE_OFFSET=1;

  // bottom left
  BOTTOM_LEFT_HOLE_OFFSET_X= (HOLE_DIAMETER/2) + EDGE_OFFSET;
  BOTTOM_LEFT_HOLE_OFFSET_Y= BOTTOM_LEFT_HOLE_OFFSET_X;
  translate([BOTTOM_LEFT_HOLE_OFFSET_X, BOTTOM_LEFT_HOLE_OFFSET_Y, 0]) {
    color("pink") {
      circle(d=HOLE_DIAMETER);
    }
  }

  //bottom right
  BOTTOM_RIGHT_HOLE_OFFSET_X= BOTTOM_LEFT_HOLE_OFFSET_X + HOLE_DIST_WIDTH + HOLE_DIAMETER;
  BOTTOM_RIGHT_HOLE_OFFSET_Y= BOTTOM_LEFT_HOLE_OFFSET_Y;
  translate([BOTTOM_RIGHT_HOLE_OFFSET_X, BOTTOM_RIGHT_HOLE_OFFSET_Y, 0]) {
    color("pink") {
      circle(d=HOLE_DIAMETER);
    }
  }

  //top right
  TOP_RIGHT_HOLE_OFFSET_X= BOTTOM_RIGHT_HOLE_OFFSET_X;
  TOP_RIGHT_HOLE_OFFSET_Y= BOTTOM_RIGHT_HOLE_OFFSET_Y + HOLE_DIST_HEIGHT + HOLE_DIAMETER;
  translate([TOP_RIGHT_HOLE_OFFSET_X, TOP_RIGHT_HOLE_OFFSET_Y, 0]) {
    color("pink") {
      circle(d=HOLE_DIAMETER);
    }
  }

  //top left 
  TOP_LEFT_HOLE_OFFSET_X= BOTTOM_LEFT_HOLE_OFFSET_X;
  TOP_LEFT_HOLE_OFFSET_Y= TOP_RIGHT_HOLE_OFFSET_Y;
  translate([TOP_LEFT_HOLE_OFFSET_X, TOP_LEFT_HOLE_OFFSET_Y, 0]) {
    color("pink") {
      circle(d=HOLE_DIAMETER);
    }
  }

}


