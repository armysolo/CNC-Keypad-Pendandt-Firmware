// CNC pendant interface to Duet
// D Crocker, started 2020-05-04

/* This Arduino sketch can be run on either Arduino Nano or Arduino Pro Micro. 
 * It should alo work on an Arduino Uno (using the same wiring scheme as for the Nano) or Arduino Leonardo (using the same wiring scheme as for the Pro Micro).
 * The recommended board is the Arduino Pro Micro because the passthrough works without any modificatoins to the Arduino. 

Nano     Connected to:   
VCC      PanelDue side port 5V
GND      PanelDue side port 0V
GND      LED - Terminal, Rotary Encoder Ground, Mini Switch Middle Terminal, Emergency Stop
5v       Rotary Encoder + Terminal, LED + Terminal-Make sure you use the correct resistor to reduce the votage from the Arduino. 120-220 ohm should be sufficient.
A0       Emergency Stop
A1       NC
A2       Rotary Encoder Pin A
A3       Rotary Encoder Pin B
A4       X Switch Top Lug
A5       Y Switch Top Lug
A6       Z Switch Top Lug
A7       NC
D2       Keypad Membrane
D3       Keypad Membrane
D4       Keypad Membrane
D5       Keypad Membrane
D6       Keypad Membrane
D7       Keypad Membrane
D8       Keypad Membrane
D9       Keypad Membrane
D10      X1 Switch Top Lug
D11      X10 Switch Top Lug
D12      X100 Switch Top Lug
D13      NC
TX1      Through 6K8 resistor to Duet URXD0
RX0      Paneldue Dout Side Port

On the Arduino Nano is necessary to replace the 1K resistor between the USB interface chip by a 10K resistor so that PanelDiue can override the USB chip.
On Arduino Nano clones with CH340G chip, it is also necessary to remove the RxD LED or its series resistor.

*/

// Configuration constants
#include <Keypad.h>

const int PinA = A2;
const int PinB = A3;
const int PinX = A4;
const int PinY = A5;
const int PinZ = A6;
const int PinStop = A0;
const int PinTimes1 = 10;
const int PinTimes10 = 11;
const int PinTimes100 = 12;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//Installed Upright/Normal
char hexaKeys[ROWS][COLS] = {
       {'1','2','3','A'},
       {'4','5','6','B'},
       {'7','8','9','C'},
       {'*','0','#','D'}
    };

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};


Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

const unsigned long BaudRate = 57600;
const int PulsesPerClick = 4;
const unsigned long MinCommandInterval = 20;

// Table of commands we send, one entry for each axis
const char* const MoveCommands[] =
{
  "G91 G0 F6000 X",     // X axis
  "G91 G0 F6000 Y",     // Y axis
  "G91 G0 F600 Z",      // Z axis
};

#include "RotaryEncoder.h"
#include "GCodeSerial.h"
#include "PassThrough.h"

RotaryEncoder encoder(PinA, PinB, PulsesPerClick);
PassThrough passThrough;

int serialBufferSize;
int distanceMultiplier;
int axis;
uint32_t whenLastCommandSent = 0;

const int axisPins[] = { PinX, PinY, PinZ, };
const int feedAmountPins[] = { PinTimes1, PinTimes10, PinTimes100 };

# define UartSerial   Serial

GCodeSerial output(UartSerial);

void setup()
{
  pinMode(PinA, INPUT_PULLUP);
  pinMode(PinB, INPUT_PULLUP);
  pinMode(PinX, INPUT_PULLUP);
  pinMode(PinY, INPUT_PULLUP);
  pinMode(PinZ, INPUT_PULLUP);
  pinMode(PinTimes1, INPUT_PULLUP);
  pinMode(PinTimes10, INPUT_PULLUP);
  pinMode(PinTimes100, INPUT_PULLUP);
  pinMode(PinStop, INPUT_PULLUP);
  output.begin(BaudRate);
  serialBufferSize = output.availableForWrite();
  Serial.begin(57600);
  keypad.addEventListener(keypadEvent);  //Keypad 1
}

// Check for received data from PanelDue, store it in the pass through buffer, and send it if we have a complete command
void checkPassThrough()
{
  unsigned int commandLength = passThrough.Check(UartSerial);
  if (commandLength != 0 && UartSerial.availableForWrite() == serialBufferSize)
  {
    output.write(passThrough.GetCommand(), commandLength);
  }
}

void loop()
{
  // 0. Poll the encoder. Ideally we would do this in the tick ISR, but after all these years the Arduino core STILL doesn't let us hook it.
  // We could possibly use interrupts instead, but if the encoder suffers from contact bounce then that isn't a good idea.
  // In practice this loop executes fast enough that polling it here works well enough
  encoder.poll();

  // 1. Check for emergency stop
  if (digitalRead(PinStop) == LOW)
  {
    // Send emergency stop command every 2 seconds
    do
    {
      output.write("M112 ;" "\xF0" "\x0F" "\n");
      uint16_t now = (uint16_t)millis();
      while (digitalRead(PinStop) == HIGH && (uint16_t)millis() - now < 2000)
      {
        checkPassThrough();
      }
      encoder.getChange();      // ignore any movement
    } while (digitalRead(PinStop) == HIGH);

    output.write("M999\n");
  }
  char key = keypad.getKey();  //Keypad 1

  // 2. Poll the feed amount switch
  distanceMultiplier = 0;
  int localDistanceMultiplier = 1;
  for (int pin : feedAmountPins)
  {
    if (digitalRead(pin) == LOW)
    {
      distanceMultiplier = localDistanceMultiplier;
      break;
    }
    localDistanceMultiplier *= 10;
  }

  // 3. Poll the axis selector switch
  axis = -1;
  int localAxis = 0;
  for (int pin : axisPins)
  {
    if (digitalRead(pin) == LOW)
    {
      axis = localAxis;
      break;
    }
    ++localAxis;    
  }
  
  // 5. If the serial output buffer is empty, send a G0 command for the accumulated encoder motion.
  if (output.availableForWrite() == serialBufferSize)
  {
    const uint32_t now = millis();
    if (now - whenLastCommandSent >= MinCommandInterval)
    {
      int distance = encoder.getChange() * distanceMultiplier;
      if (axis >= 0 && distance != 0)
      {
        whenLastCommandSent = now;
        output.write(MoveCommands[axis]);
        if (distance < 0)
        {
          output.write('-');
          distance = -distance;
        }
        output.print(distance/10);
        output.write('.');
        output.print(distance % 10);
        output.write('\n');
      }
    }
  }

  checkPassThrough();
}

void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()) {
    
    /* About getState: the PRESSED state occurs first, right when the users presses a key.
     * Following PRESSED the next state can be either HOLD or RELEASED.
     * If the key is pressed longer than 500 milliseconds (default) you will reach the HOLD state.  
     * However, if the key was pressed and released in less than 500 milliseconds then the next state is RELEASED.
     * The RELEASED state occurs after a PRESSED or HOLD state and only after the user lets go of the key.
    */
    
    case PRESSED:

        //row1
             if (key == '1') { output.print("G28\n"); } // Home all 
        else if (key == '2') { output.print("G32\n"); } // Bed Leveling
        else if (key == '3') { output.print("M120\n M83\n G1\n E10\n F900\n M121\n"); } //Extrude 10mm of filament at 15mm/s
        else if (key == 'A') { output.print("M120\n M83\n G1\n E-10\n F900\n M121\n"); } //Extrude 10mm of filament at 15mm/s
        //row2
        else if (key == '4') { output.print("T0 P0\n G10 P0 S200\n"); }  // Enable hotend and set temp to 200c
        else if (key == '5') { output.print("T-1 P0\n G10 P0S-273.15 R-273.15\n"); }  // Turn hotend off
        else if (key == '6') { output.print("M140 S60\n"); }  // Heatbed to 60c
        else if (key == 'B') { output.print("M140 S0\n M140 S-273.15\n"); } // Turn heatbed off
        //row3
        else if (key == '7') { output.print("M106 S0.90\n"); } // Turn fan on to 90%
        else if (key == '8') { output.print("M106 S0\n"); } // Turn fan on off
        else if (key == '9') { output.print("M112\n"); } // Emergency Stop
        else if (key == 'C') { output.print("M112+M999\n"); } // Emergency stop and reset board
        //row4
        else if (key == '*') { output.print("M558 P9\n G31 P500 Z1.0\n"); } // Set Z Probe to BLTouch and probe the bed at current position
        else if (key == '0') { output.print("G92 Z10\n"); } // Set Current Z Height to 10
        else if (key == '#') { output.print("G92 Z0\n"); } // Set Current Z Height to 0
        else if (key == 'D') { output.print("G30 S-1\n"); } // Probe the bed at the current position
        break;

    case RELEASED:        
        break;

    case HOLD:
        break;
  }
}
// End
