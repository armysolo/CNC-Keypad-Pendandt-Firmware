# CNC-Pendant-Firmware

Modified CNC Pendant Firmware with Arduino Nano Clone with CH340G microchip. I didn't want to spend $80(USD) on a pendant that limited the functionality to only jogging moves. My branch keeps the functionality of jogging using a rotary encoder and common micro SPDT 1P2T switches but also incorporates a 16 button keypad where you can assign custom G/M codes or even macros. This also maintains Panel Due functionality by passing TX from the Arduino to the Duet board and RX from the Arudino to the Panel Due.

*NOTE: This should work with other Arduino boards like the Arduino Micro Pro. However my testing was done only with a Nano clone so all wiring and sketch notes are based on the Nano. There's residual info/configs from the original fork I didn't want to get rid of.

The Arduino Nano I use come un-soldered. The printed parts I designed are based off Arduino Nano clones without the pins. The parts could still work if modified but for now you'd probably have some fitment issues.

```
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
```

**Parts list** These are the links for reference of the parts I puchased:

**Arduino Nano Clone** These boards are so inexepensive and have so many uses I buy them 10 at a time un-soldered. With them unsoldered it's also easier to desolder the resistor on the front leading to the LED and desolder the 1k resistor on the rear and replace it with a 10k resistor. I used a little hot glue from a hot glue gun to secure the resistor to the back of the board to protect it.

**10 Pack un-soldered**
https://www.amazon.com/Longruner-ATmega328P-Controller-Module-Arduino/dp/B01MSYWE6B/ref=sxts_sxwds-bia-wc-p13n2_0?cv_ct_cx=Arduino+nano&dchild=1&keywords=Arduino+nano&pd_rd_i=B01MSYWE6B&pd_rd_r=591b0af8-b947-419b-8434-9e79dfdbbbc6&pd_rd_w=AnJxk&pd_rd_wg=Pw5cq&pf_rd_p=1da5beeb-8f71-435c-b5c5-3279a6171294&pf_rd_r=6WRDT0RYW70J8K0M9YS9&psc=1&qid=1594168206&sr=1-2-70f7c15d-07d8-466a-b325-4be35d7258cc

**3 Pack un-soldered**
https://www.amazon.com/WYPH-ATmega328P-Microcontroller-Development-Not-soldered/dp/B07KCH534K/ref=sr_1_6?dchild=1&keywords=Arduino+nano&qid=1594168206&sr=8-6

**2 Position Mini switch** If it's a 3 prong switch the middle lug is the common lug. When the switch is in the bottom position, the circuit for the bottom lug and middle lug are closed while the top lug is open. When the switch is in the top position, the circuit for the top lug and middle lug are closed while the bottom lug is open.
https://www.amazon.com/uxcell-Position-Switch-Solder-SS12F1-G3/dp/B007QAJMHO/ref=sr_1_14?dchild=1&keywords=2+Position+SPDT+1P2T+Mini+Panel+Slide+Switch&qid=1594083563&sr=8-14

**Keypad membrane** 4x4 16 key keypad membrane
https://www.amazon.com/WINGONEER-Universial-Switch-Keyboard-Arduino/dp/B06XHFC19G/ref=pd_sbs_328_2/133-1063505-5236503?_encoding=UTF8&pd_rd_i=B06XHFC19G&pd_rd_r=67e9bbfe-5eec-46e6-a852-5baba54dd59c&pd_rd_w=SBiDd&pd_rd_wg=edIjZ&pf_rd_p=d28ef93e-22cf-4527-b60a-90c984b5663d&pf_rd_r=3M5CRSQBKZ5DJ6H25J6K&psc=1&refRID=3M5CRSQBKZ5DJ6H25J6K

**KY-040 Rotary Encoder** I desoldered the pins and soldered the wires directly to the board. The push button terminal(SW) is not connected to the Arudino.
https://www.amazon.com/Cylewet-Encoder-15%C3%9716-5-Arduino-CYT1062/dp/B06XQTHDRR/ref=sr_1_3?crid=2URJ7FOBKV0DA&dchild=1&keywords=rotary+encoder&qid=1594146946&sprefix=rotary+e%2Caps%2C183&sr=8-3

**6 x 6mm Momentary Tactile Push Button Switch Micro NO/NC** These are 4 pin momentary push button switches. Depending which lugs you solder, the switch can be used as NO(switch circuit is open until pressed) OR NC(switch is closed until pressed). Currently used for Emergency Stop/Reset Button. Can also be used for to assign additional G/M codes.
https://www.amazon.com/gp/product/B071KX71SV/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
**Note: Coding to support additional push button switches in prgoress.

**5mm LEDs**
https://www.amazon.com/MCIGICM-Circuit-Assorted-Science-Experiment/dp/B07PG84V17/ref=sr_1_3?dchild=1&keywords=5mm+led&qid=1594167835&s=industrial&sr=1-3
