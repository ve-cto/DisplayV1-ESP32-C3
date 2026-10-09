# PROJECT-TITLE
> yes, i know, i just haven't decided yet :sob:

Controls an HV5812 which is driving an old 10 Digit, 9 Segment VFD, with a XIAO ESP32-C3.
> !physically untested... *for now*
## Devlog
### Backstory
When my family inherited a business many years ago, the deal included one of these old cash registers.
<br><img src="https://ve-cto.github.io/portfolio/display_register.png" alt="" width="250"><br>
When we eventually gave up on it because of numerous issues, and bought a new one, it was destined for landfill. So, making the best of the moment, I scrapped it for parts. (Who wouldn't?!)
One of the fruits of my labor? This old Futaba VFD (and driving board) that was on the turret - the bit that faced the client and showed them the total price and whatnot. It had this really long JST-like connector that led down into the mainboard, which actually drove the thing. The board had traces on only one side (thankfully), and consisted of two diodes, two small ceramic capacitors, a couple jumpers, and a *mystery chip oOOOOooohh*.
Aformentioned chip was completely obscured by the VFD ontop of it, and I didn't really fancy desoldering the fragile mess, so I left it.

I started to probe the board for connections. Finding ground was pretty easy, on either side of the VFD is where the filaments connect, the thin wires that the shoot out electrons(..? it's witchcraft.). One side is grounded, the other loaded with low voltage. Looking at how the diodes were connected, V+ was on the right, GND on the left (as seen from the bottom side of the PCB).
That... is how far I got up to before having to know what this chip is. Desoldering the VFD was a minor pain, a lot less than I thought it would be, but still tedious.
Now being able to see the chip, it was an HV5812, made by Microchip Technology.
<br><img src="https://ve-cto.github.io/portfolio/display_VFDPCBfront.jpg" alt="" width="100">
<img src="https://ve-cto.github.io/portfolio/display_VFDPCBback.jpg" alt="" width="100"><br>
The HV is a glorified 20 bit latching shift register, with the latched bits being connected to a dedicated high voltage line (VPP) responsible for powering the displays' segments. You clock in data bit by bit on the DIN pin, and then when driving the STOBE pin high, all the bits become latched. There's a BLANK pin which resets all the bits, and a VDD pin for supplying the chip with 5V power.

Now knowing the chip, I could figure out how the display was connected by visual inspection and probing the board with a multimeter on continuity.
<br><img src="https://ve-cto.github.io/portfolio/display_chip-pinout.png" alt="" width="200"><br>
Aaaaand that's what I came up with.
Each "S" is one of the display segments, and each "L" is a Lllll....digit. The segments were labled A to H (+DP) on the boards silkscreen, so why change that? I also mapped the JST connector-thing, where JST1 is the leftmost pin as viewed from the bottom of the board.

Next step: Prototyping!

### Prototyping
I didn't.

### PCB Design
PCB Design time!
I settled on using a XIAO ESP32-C3 because I really like the form factor of the XIAO series, and having WiFi is great because I eventually want connect it to the interwebs. The HV uses 5V logic, and the XIAO is 3.3V logic, and the four channels on a 74HCT125 ***quad non-inverting buffer and line driver*** is just enough to interface completely with the HV. Putting a 10k resistor across BLK and 5V ensures that the display is blanked while the board starts, or when the pin is otherwise floating. The VFD needs upwards of 20 volts on its' channels, but at really low current, so an MT3608 boost module would provide enough power taking 5V input from the XIAO. Instead of reconstructing the modules' circuit, I just whacked the entire module on the board. Easy! Putting a row of PTH's at 2.54mm pitch allows for connection to existing display PCB through some headers. Sprinkle on a couple WS2812B addressable LED's for the glamour, completely forgetting the 100uF capacitors that they need, and we're done!
<br><img src="https://ve-cto.github.io/portfolio/display_pcb1.png" alt="" width="500">
<br><img src="https://ve-cto.github.io/portfolio/display_schematic1.png" alt="" width="500"><br>
> I'll put the production files on here later, once I know that everything works.

### Programming
Whilst waiting for JLCPCB to make and ship out my boards, I started working on programming the XIAO. 
I couldn't find any libraries that interface with the HV5812, so we're just diving into it.

The XIAO connects to the HV through four data pins, DIN, CLK, STR, and BLK which are D0, D1, D2, and D3 respectively. The HV5812 holds 20 bits, or as I call it, one *packet* of data, so when we push new data to the display it needs to be 20 bits so all the data ends up in the right spot along the shift register.
Each of those 20 bits correspond to the HVout*X* pins on the HV5812, which are connected to each segment and digit of the display. To draw a *pattern*, we need to clock in the corresponding digit and segment/s. Since the segments are shared between each digit, we need to procedually display each digit individually else we end up with ghosting.<br>
Each segment is made into a mask, so combining them together to make one 20 bit pattern is as simple as OR'ing all the ones you want active together. When we end up writing it to the display, we OR the digits' bit in. This leaves the patterns open for use with whatever digit we want.

When writing to the chip, we pull BLK high which clears the display. We clock in one 0 bit, which corresponds to the one display segment that wasn't wired up (HVout20).
> A clock cycle is where we set the DIN pin to some value, HIGH or LOW, and then write the CLK pin HIGH then LOW. This saves the value of the DIN pin into the first bit of the shift register, and moves all the other bits along the register. (For example, if the register was 0100 and we clocked, it would become 0010).

We then iterate through the remaining 19 bits, writing their value to DIN and clocking it into the register. Pulling STR HIGH then LOW latches the values, and then pulling BLK LOW reactivates the display, putting power through those pins we just activated.

Being able to communicate via Serial allows easy enough testing without having the reflash the board each and every time. In this case the host sends 3 bytes to the XIAO, which is enough to set a single digits' buffer.
The first four bits of the first byte corresponds to the digits' index, 0 thru 10. The remaining bits and bytes are the *pattern* that is to be assigned to that digit.
So, say we sent the following bytes to the device:<br>
> `BBBBAAAA DDDDCCCC FFFFEEEE`<br>`01010011 00101001 10010000`

We mask the first four bits of the first byte for the digit, so `0011`, or index 3.
We then make a 32 bit value based on the remaining bits, like so:<br>
> `00000000 0000BBBB DDDDCCCC FFFFEEEE`<br>`00000000 00000101 00101001 10010000`

### Tags
To aid in the discoverability of this for those that salvage identical or similar displays and also wish to repurpose them, the following terms have been collected...
- TURRET PCB(10 DIGIT)
- MODEL: ER-550/650/650R/5200
- CODE NO.: JK41-105471 V1.0
- VFD: FUTABA
- DATE: 2002 11
- Futaba 10-LT-50QNK B55689 318 VFD