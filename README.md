# Notes on the Tardis Implementation

Design notes, technical documentation, and code comments that grew too large.

[TOC]: Notes.md " "


## Software Details

### Class Heirarchy

If it were up to me, I'd implement a monolithic class to serve the API. However, the API expects a
`KeyScanner` and `LedDriver` class. I use these only to group the related pieces of data together.
The code primarily lives in the `Tardis` class.

### The Bit Pattern of a Command Code

The simplicity of the hardware is obscured by the unfortunate fact that there are five different
addressing schemes!

1. Logical address — scheme convenient for software Since we are free to choose (a), I have chosen
   it to be a linearization of (c).
2. Physical circuit address — traditional "row" and "column" of the key matrix. Determined by the
   original PCB designer and is unrelated to the physical location of the keys. Has a simple linear
   relationship to (c).
3. Physical pin address — pin number on PCB ribbon cable. Has a simple linear relationship to (b).
4. Physical (button) layout address The representation that we care about, but unrelated to the
   others! We encode this in a lookup table defined in `TARDIS_COORD_TABLE` and stored in
   `TardisKeyScanner.coord_table_` (`TardisKeyScanner.hpp`).
5. How the (c) is encoded when written out to the registers. Determined by the construction of the
   hardware driver circuits. Has no nice relationship to anything else, but is easy to determined
   from the table below.

```
Reg1
1: Inhibit Demux A - Always low
2: Inhibit Demux B = ~Bit3
3: Inhibit Demux C = ~Bit2
4: Inhibit Demux D = Not connected (but always low)
5: DemuxA SelectA \
6: DemuxA SelectB  \
7: DemuxA SelectC  /`— Binary encoding of row addresses 0-7 (pins 0-7) of key matrix.
8: DemuxA SelectD /    (Note that SelectD is always low.)

Reg2
9: DemuxB SelectA \
10: DemuxB SelectB  \
11: DemuxB SelectC  /`— Binary encoding of col addresses 0-15 (pins 27-42) of key matrix.
12: DemuxB SelectD /
13: DemuxC SelectA \
14: DemuxC SelectB  \
15: DemuxC SelectC  /`— Binary encoding of col address mod 16 for col addresses 16-32
16: DemuxC SelectD /    (pins 43-58) of key matrix.

Reg3
17: Driver1 In1 <-> LED Pin57 <-> LED Row 17
18: Driver1 In2 <-> LED Pin58 <-> LED Row 18
19: Driver1 In3 <-> LED Pin59 <-> LED Row 19
20: Driver1 In4 <-> LED Pin60 <-> LED Row 20
21: Driver1 In5 <-> GND = Always low
22: Driver1 In6 <-> GND = Always low
23: Driver1 In7 <-> GND = Always low
24: Driver1 In8 <-> GND = Always low

Reg4
25: Driver2 In1 <-> LED Pin49 <-> LED Row 9
26: Driver2 In2 <-> LED Pin50 <-> LED Row 10
27: Driver2 In3 <-> LED Pin51 <-> LED Row 11
28: Driver2 In4 <-> LED Pin52 <-> LED Row 12
29: Driver2 In5 <-> LED Pin53 <-> LED Row 13
30: Driver2 In6 <-> LED Pin54 <-> LED Row 14
31: Driver2 In7 <-> LED Pin55 <-> LED Row 15
32: Driver2 In8 <-> LED Pin56 <-> LED Row 16

Reg5
33: Driver3_In1 <-> LED Pin41 <-> LED Row 1
34: Driver3_In2 <-> LED Pin42 <-> LED Row 2
35: Driver3_In3 <-> LED Pin43 <-> LED Row 3
36: Driver3_In4 <-> LED Pin44 <-> LED Row 4
37: Driver3_In5 <-> LED Pin45 <-> LED Row 5
38: Driver3_In6 <-> LED Pin46 <-> LED Row 6
39: Driver3_In7 <-> LED Pin47 <-> LED Row 7
40: Driver3_In8 <-> LED Pin48 <-> LED Row 8

Reg6
41: Driver4_In1 <-> Physical LED Cols/Pins 1  9 17 25 33 <-> Logical LED Col 1
42: Driver4_In2 <-> Physical LED Cols/Pins 2 10 18 26 34 <-> Logical LED Col 2
43: Driver4_In3 <-> Physical LED Cols/Pins 3 11 19 27 35 <-> Logical LED Col 3
44: Driver4_In4 <-> Physical LED Cols/Pins 4 12 20 28 36 <-> Logical LED Col 4
45: Driver4_In5 <-> Physical LED Cols/Pins 5 13 21 29 37 <-> Logical LED Col 5
46: Driver4_In6 <-> Physical LED Cols/Pins 6 14 22 30 38 <-> Logical LED Col 6
47: Driver4_In7 <-> Physical LED Cols/Pins 7 15 23 31 39 <-> Logical LED Col 7
48: Driver4_In8 <-> Physical LED Cols/Pins 8 16 24 32 40 <-> Logical LED Col 8
```


### Meeting Ordering/Timing Requirements

The hardware implementation of SPI on the STM32F1 is broken: There is no hardware handling of NSS.
It turns out to be just as easy, then, to implement SPI in software. We (ab)use the `sev` assembly
instruction to output the clock signal.

The data pin needs to be high/low on the upward transition of SCK. The MOSI pin tansitions ~5 cycles
after the write instruction is executed. On the other hand, `sev` strobes SCK instantly. The SCK
strobe must be delayed by two bits to be in time with the data on MOSI. Consequently, to achieve
correct timing, several no-ops need to be inserted at the begining and end of each transmission. It
is possible to insert other useful instructions in place of no-ops, but it probably isn't worth it.

Once we set the latch, the voltage on the key read pin will decay exponentially if it was high or
increase like 1-exp(t) if it was previously low and is being brought up, so we don't want to read it
immediately. In fact, immediately before setting the latch to "activate" the new bit pattern is the
moment when the voltage on the input pin has had the most time to rise to its maximum or decay to
its minimum level from the last time we set the latch. This is when we finally read it.

OPEN QUESTION: Is there a significant delay to read a pin?

### Setting and Resetting GPIO Pins

Two distinct techniques for changing the value of a GPIO pin are used for setting and resetting.
Believe it or not, they happen to be the choices that optimize transition speed—but not necessarily
latency.

## Details of Hardware Interface

The software communicates with the hardware by shifting out control commands, which are specific bit
patterns described in these notes, to a shift register.


### Computing the Bit Pattern


The high byte encodes which column is being scanned. For columns 0-15, the first four bits encode
the column address, and the last four bits are DON'T CAREs. For columns 16-31, the first four bits
are DON'T CAREs, and the last four bits are the column address mod 32. high_byte = (current_column
<< 4) | current_column;

The lower byte's highest four bits are 0-7, giving which row is being read. The lower four bits are
XCBA (MSB to LSB), where X=DON'T CARE, C=Inhibit Mutex C, B=Inhibit Mutex B, and A=Inhibit Mutex A.
We set A=X=0, and B=~C.

Observe the effect of shifting right by 4:

         x = 0b0000000011111iiiU
    x << 4 = 0b00011111iii00000U,

where `reverse_bits` acts on `uint16_t`'s (but see below). Given the above, convince yourself that
the magic incantation to decode j = `key_index` into the hardware shift register bit pattern is:

```cpp
hword = reverse_bits((j << 4) | 0b0100U);                       // For columns 0-15
hword = reverse_bits((j << 4) & 0x00ffU | 0b0010U | (j << 9));  // For columns 16-31
```

The actual code is a bit uglier, because the `rbit` ARM assembly instruction which implements
`reverse_bits` only works on 32-bit words, not 16-bit half words. Luckily, it turns out we can use a
`uint32_t` for the hword value in the first place, making it much less ugly, but still ugly, like
that present day Bookface photo of your high school crush who you haven't seen in 25 years. You
really dodged that bullet.


### Shifting Out the Bit Pattern

This algorithm shifts out each byte LSB first. The `out_buffer` must be initialized with most
significant byte first but each byte bit-reversed. What's more, STM32 computes 32 bits at a time,
whereas NSS strobes every 48 bits. There are three solutions to the problem this raises:

1. Alternate strobing NSS according to the pattern:

```
     a)  DOWN     hword      hword
     b)  hword    UP-DOWN    hword
     c)  hword    hword      UP
d) = a)  DOWN     hword      hword
e) = b)  hword    UP-DOWN    hword ...
```

2. Do 64 bits (two `uint32_t`s) each cycle.
3. Compute the 48 bits on the fly and just write 48 bits out every loop.

The first solution trades time for space; the second trades space for time. It's not clear what the
performance profile of #3 is. It's possible we could squeeze it into the no-ops. The bytes
controlling the LEDs are pre- computed and incur no runtime overhead.

### Performance of `readMatrix`

Possible options for improving performance of `readMatrix` (should it be necessary):

1. Put the loop in its own thread. I.e. transfer the data asynchronously, and have it continue
   looping through a callback/interrupt. **Edit: Broken on STM32F1.**
2. Instead of calling high-level `SPI.dma*` functions, which enable and disable DMA/SPI every call
   as well as have function call overhead, inline the code, moving the enable/disable code to
   outside the loop. **Edit: Apparently extremely fragile. Can't get it to work at all!**
3. Use async while computing the next `out_data_` or acting on previous scan, then wait for
   completion when ready to send.
4. Aggressively unroll the loop. **Edit: Very little performance benefit, as loop overhead is tiny
   relative to body of the loop.**
5. Use (fast) digitalWrite and the bit band instead of DMA+SPI. Unfortunately, we have to toggle the
   latch on the shift registers every 6 bytes, so we have no choice but to fall out of SPI/DMA every
   6 bytes. The overhead of SPI/DMA might exceed the advantage of using it. **Edit: This seems to
   work best and is what we implement.**
6. Avoid recomputing entire `out_data_` each loop, since col only changes once every 8 iterations.
   **Edit: Very little performance benefit. We save maybe 6 cycles out of hundreds.**
7. Just write the assembly. It is pretty simple code to write, wouldn't be hard. **Edit: It's not
   hard at all, but it's slightly easier to embed only a few instructions in assembly, as the
   translation of the C++ code is trivial on the hot path.**

[Shifting Out the Bit-Pattern]: #shifting-out-the-bit-pattern
[Details of Hardware Interface]: #details-of-hardware-interface


