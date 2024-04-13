# Building a Eurorack synth

This is a collection of notes and resources for building a Eurorack synth. 
I'm not an expert, but I'm learning as I go and I'm happy to share what I've learned.

## Disclaimer

I am not responsible for any damage to any of your expensive other modules. The modules
within this repository have been tested and built by myself to use ONLY 
with the other modules in this system. There is a chance they work well with other modules, 
but I do not own any, so I can not test it out. If you proceed to copy or use any file within 
this repository, you are doing this at your own risk.

These modules are also not perfect 1v/oct or temperature controlled. This project should be 
seen as a fun hobby project and not as professional advice on how to build synthesisers.

## The story

Since I was young, I have always been interested in 2 things: Music & Electronics. 
I started drumming when I was 6 years old, and I was always fascinated by the recording equipment
my dad had for his band. 

I played drums for some years, tried guitar and piano but settled on producing electronic music with
software. I DJ'ed a while during my college years but stopped when I moved to Amsterdam. In 2022, while visiting
my dad in the states, we visited the MOOG factory. I purchased the MFOS book that explained how to build
your very own analogue synth. I must have read that book 3 or 4 times, but it took another 2 years of thinking and watching
youTube to finally make the decision to try it out.

## What do you need?

I didn't have much electronic equipment, so I started saving and purchasing as I got the money:

- Osciloscope
- Dual Bench Power Supply
- Hakko Solder Station
- Precision measuring tool

I already had most of the other small tools, but here is a list of my most used tools:

- KNIPEX 78 61 125 Precision Pliers
- KNIPEX 12 62 180 Automatic Wire Stripper
- KNIPEX 92 28 69 ESD Tweezers (don't get cheap ones)
- de-soldering pump (It's not easy to use, but has saved my PCB's countless times)
- breadboards (get enough if you plan to keep your builds for debugging)

The next step was to start sourcing components, because I did not want to just order kits.

I started by watching lots of YouTube and getting schematics from the internet. That gave me a
good idea of the main things I would need. Here are the most used components I have. 
The quantity is just a guideline for how many I got to get started. 


### Resistors

I started with a big set of lots of different values to start with, and ordered in bulk when I needed them.
Here is the list of my most used resistor values:

- 1M (50x)
- 100K (200x)
- 68K (100x)
- 47K (100x)
- 33K (50x)
- 22K (50x)
- 14K (50x)
- 10K (100x)
- 4.7K (100x)
- 1K (100x)
- 470R (50x)
- 200R (50x)
- 100R (100x)
- 10R (100x)

### Potentiometers

I have been looking for cheap (but good) ones that fit on PCB's vertically for a long time.
The best I can found are the ALPHA 9mm (I buy them on thonk.co.uk). You can find some on digiKey, 
but the ALPHA's are my recomendation. I bought some with a D-shaft, and others just as a long shaft.

- A1M (10x)
- B250K (10x)
- A250K (10x)
- B100K (50x)
- A100K (10x)
- B50K (10x)

### Diodes

- 1N1418 (200x) Used for general-purpose direction of current
- 2N.... (50x) Used for power

### Capacitors

It's also good to have some of every value. I got a box of electrolytic and film

- 1uF MKS
- 0.1uF MKS
- 2.2nf MKS
- 47uF electrolytic
- 0.1uf electrolytic

### Transistors

There are some general NPN and PNP transistors you can get, I mainly got some variety that I found on videos and blogs:

- BC558 (PNP, 25x)
- BC548 (NPN, 50X)
- 2N2904 (PNP, 25x)

### IC's

- TL072 (25x) [operational amplifier]
- TL074 (50x) [operational amplifier]
- CD40106 (10x) [Hex Schmidt trigger]
- CD4016 [decade counter]
- CD.... [binary counter]


## Schematics & PCB creation

I use kiCad, open-source software to design all my schematics. It's pretty easy to get a hang
of it and also has a built in PCB creator. You can also design custom footprints for components they
don't have in their (pretty big) library. I have included some of my footprints/symbols in this repo.


### Tips

#### Testing Points

For my first PCB, I had some testing points and that made it easier to debug. I forgot them for all the other
ones and had a really bad time. I had to keep prodding resistor legs to understand what was going on. Always add
testing points to your schematic & PCB.

#### Measure

Always measure 20 times when creating a footprint. It's really annoying when you have ordered your new
PCB and the component just does not fit. I tried to make a SPDT switch footprint, and they just did not fit well.

#### PIN numbers

Make sure when you are giving the pins a number, the correspond to the correct number on the whole. I had the issue
with my SPDT switch, that they just didn't work. I got the holes mixed up and ended with the wrong pin out.


#### LED's

When using an LED, never connect it directly to an IC to drive it. Always use a transistor to drive the LED.
This is important because you don't want to burn your IC because there is too much current flowing over your LED.

[INSERT IMAGE OF LED DRIVER SCHEMATIC]

#### Header pins

If you are following my schematics, you will see I use header/socket pins to connect my boards.
I first started with the 2.00mm pins. I could not easily find tall sockets, so I ended having to
always stack 2 on top of eachother, this was not super stable in the end, and I had to solder 
all the cap's to the back of the board because I didn't want to use 3 stacked socket pins.

#### Mounting holes

Make sure you place 4 mounting holes on your board, and that they match the coordinates of the
top board. I got simple 3mm & 2mm spacers from nylon.



## Modules

### Voltage Controlled Oscillators

- [VIXEN 3000.014](modules/voltage-controlled-oscillators/1-vixen/README.md)

### Voltage Controlled Amplifiers

- [Sparkle](modules/voltage-controlled-oscillators/1-vixen/README.md)

### Sequencer

- [Sensei](modules/voltage-controlled-oscillators/1-vixen/README.md)

### Envelope Generator

- [Scribble](modules/voltage-controlled-oscillators/1-vixen/README.md)

### Active Mixer

- [Distorted Daisy](modules/voltage-controlled-oscillators/1-vixen/README.md)


## Upcoming

- [VCO: CEM 3340](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [LFO](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [Digital Sequencer](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [Filter: CEM 3320](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [Filter: Ladder](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [Digital Quantizer](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [Midi to CV](modules/voltage-controlled-oscillators/1-vixen/README.md)


- [Kick](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [Snare/clap](modules/voltage-controlled-oscillators/1-vixen/README.md)
- [HiHat](modules/voltage-controlled-oscillators/1-vixen/README.md)

