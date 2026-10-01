# Alethiometer Electronics

The electronics and Arduino code for the divination needle of a working alethiometer replica from His Dark Materials, based on Myth Made's [YouTube build](https://www.youtube.com/watch?v=rpL9xWBeznU). The Arduino sketch (code) is adapted from Myth Made's original code. Touch the sensor and the needle swings to a random sequence of symbols, pausing and sometimes quivering on each one.

## Folder layout

```
electronics/
├── README.md                        this file: usage, wiring and theory
└── arduino_alethiometer/
    └── arduino_alethiometer.ino     the Arduino sketch
```

The `.ino` file is the Arduino code file and can be opened with any text editor. The Arduino IDE needs the sketch folder and the `.ino` file to share a name, so keep `arduino_alethiometer/arduino_alethiometer.ino` together. No extra libraries are needed.

## Hardware

### Supplies

| Part | Description | Example Link |
| --- | --- | --- |
| Breadboard (half size) | Needs to fit in the case | [Breadboard and Jumper Wire Bundle](https://www.amazon.com/BOJACK-Values-Solderless-Breadboard-Flexible/dp/B08Y59P6D1) |
| Jumper wires (male to male)| One end is stripped and soldered to the motor pins, the other plugged into the breadboard. | Included in above |
|Dupont jumper wires (male to female)| If the sensor module has headers already attached, you may not need to solder and can use these to connect to the header pins|[40 pack wires](https://www.amazon.com/SinLoon-Breadboard-Arduino-Circuit-40-Pack/dp/B08M3QLL3Q)|
| Arduino Nano microcontroller board | I used a Nano clone, not an original Arduino Nano. Mine had headers already soldered in. You will need to follow the [Elegoo instructions](https://www.elegoo.com/blogs/arduino-projects/elegoo-arduino-nano-board-ch340-usb-driver) to install the right driver on your computer for the Arduino IDE to work with it. | [Elegoo Presoldered Nano Board USB-C](https://www.amazon.com/ELEGOO-Presoldered-Compatible-Arduino-Microcontroller/dp/B0F6Y7GS4Q) |
| Motor | X27.168 automotive gauge stepper motor. For 360° movement, you will need to open the motor and cut off the small bump on the big gear that acts as an end stop. I used a razor blade to cut and scrape, and followed up with a file. Open it carefully: the parts can spring out and are easy to lose. I highly advise taking photos as you open it up and take things out, so you can reconstruct | [Adafruit x27.168 stepper motor](https://www.adafruit.com/product/2424) |
| Touch sensor | HiLetgo TTP223B capacitive touch sensor module, default momentary mode. The mode matters for the code to work; on some modules you can change it by bridging solder pads. | [HiLetgo TTP223B capacitive touch sensor module](https://www.amazon.com/HiLetgo-TTP223B-Capacitive-Digital-Raspberry/dp/B00HFQEFWQ) |
|Clock hand| For final assembly, I used a brass colored clock hand with a hub that fit through the inner plate hole and onto the motor shaft. None of the clock hands had the right length, so I used a wirecutter to cut down to length and make a point|[Paraor Set of 20 clock hands](https://www.amazon.com/dp/B0CQ2FL2MS)|

#### Notes
* You could skip the breadboard completely by soldering wires directly from the Nano to the motor and touch sensor.
* For the Elegoo Nano board, use the cable that came with it for best results. USB-C to USB-C cables didn't work for me; only USB-A to USB-C did. (Many budget USB-C boards leave out the resistors that tell a USB-C port to supply power.) I ended up connecting: computer USB-C port -> USB-C to USB-A adapter -> USB-A end of cable -> USB-C end of cable -> Nano USB-C port.
* The motor is driven directly from the Nano's pins, with no driver chip. That's fine for the X27.168, which draws roughly 20 mA per coil at 5 V. If you substitute a different gauge motor or a 3.3 V board, check its datasheet first; a motor that draws more current needs a driver chip.
* In the Arduino IDE, choose Tools > Board > Arduino Nano. If uploading to a clone fails with a "not responding" error, set Tools > Processor to "ATmega328P (Old Bootloader)".


## Pre-build Testing

1. Wire up the Arduino Nano board, x27.168 motor and touch sensor on a half-sized breadboard, as in the wiring section below. I recommend placing the Nano with its USB connector right at the edge of the breadboard, since it will eventually need to sit right against a box wall.
2. Put the supplied pointer or a piece of tape on the motor shaft, so you can see the movement.
3. Connect the Nano to your computer.
4. Open `arduino_alethiometer/arduino_alethiometer.ino` in the Arduino IDE, select the Nano board and port, and upload.
5. Touch the sensor. The needle performs a reading, and the motor coils switch off when it finishes.

## Using it

Once the dial is fitted, rest the needle on the center of any symbol before powering on. Every move is a whole number of symbols (20 motor steps each), so if the needle starts centered and the motor completes every step, every stop lands on a symbol center. Nothing checks the needle's actual position, so a skipped step, a knock or a needle slipping on its shaft will throw it off until you re-center it.

When the coils first switch on, the needle may shift slightly, up to about 1°, as the motor settles into its first step position. That's normal, and nudging it back won't stick, because the motor pulls it back to the same spot next time.

If the needle gets bumped off-center, nudge it back onto the middle of a symbol before the next reading. Turn it slowly and gently: the motor datasheet warns that knocks or sudden spins of the pointer can damage its internal gears.


## Wiring

Looking down on the motor with the shaft to the north:

| From | To Arduino Pin | Note |
| --- | --- | --- |
| Motor NE pin | D4 | one coil |
| Motor SE pin | D5 | same coil as NE |
| Motor NW pin | D6 | the other coil |
| Motor SW pin | D7 | same coil as NW |
| Touch Sensor SIG | D2 | HIGH while touched |
| Touch Sensor VCC | 5V | |
| Touch Sensor GND | GND | |

The only hard rule is that each coil's two pins go to D4/D5 or to D6/D7. Swapping the two wires within one pair just reverses the direction of rotation.


## The touch sensor

The touch sensor measures the electrical capacitance of the pad. When your finger touches the pad, the capacitance increases, and when it rises past a threshold, the sensor switches its output HIGH. When you lift your finger, the output goes back to LOW. (The TTP223B can also be set to a toggle mode, where each touch flips the output and it stays that way, but this project uses the default momentary mode.) That HIGH signal goes to the Nano. The code on the Nano runs in a loop, and if it sees the HIGH signal on any pass through the loop, it starts the needle moving.

## How the Nano drives the motor

Very simplified, the motor has a small permanent magnet and two electromagnets. The permanent magnet spins the motor shaft through a series of gears. The Arduino code directs the Nano to set four pins high or low in a repeating pattern that flips the direction of current in the electromagnets. Each flip turns their combined magnetic field a quarter turn; the permanent magnet follows it like a compass, turning the gear train, and thus the needle.


## Serial Monitor bench tools

Optional, for testing with the Nano plugged into a computer. In the Arduino IDE open Tools, then Serial Monitor, set 115200 baud and the line ending to Newline, type a letter and press Enter. Opening the Serial Monitor resets the Nano, so you'll see the startup message.

| Command | What it does |
| --- | --- |
| `r` | Run a reading, same as touching the sensor |
| `t` | Tile tour: a full turn, one tile at a time. Every stop should be centered and it should finish where it started |
| `s` | Spin test: 3 turns one way, 3 back. It should land where it started; if not, look for skipped steps, rubbing or a slipping needle |
| `b` | Backlash test: steps backwards slowly so you can estimate the gear slack. Set `BACKLASH_STEPS` in the sketch to the step number where the needle first visibly moves (erring a step high is safe) |
| `x` | Switch the coils off |
| `?` | Show the command list |

Start `t`, `s` and `b` with the needle centered on a symbol.
