# 3D Printing

Unless otherwise specified, parts were printed on a Bambu A1 Mini using the `0.20mm Standard @BBL A1M` preset, or modifications of that preset.

## Filaments

For the following profiles, I changed it in Bambu Studio; your slicer may vary. Always check the labeling on the actual filament spool or package to confirm that the settings sense.

|Short Name|Manufacturer|Name|Print Profile|Notes|
|:--- |:--- |:--- |:--- |:--- |
|Gold|Sunlu|[PLA Plus Gold](https://www.amazon.com/dp/B0CJLT7Z9M)|*Sunlu PLA+*  preset already in Bambu Studio |Not really like gold or brass but is any filament?
|Silver|Elegoo|[PLA Silk Silver Grey](https://www.amazon.com/dp/B0C6QFDQWT)| See below
|Copper|Elegoo|[PLA Metal Bronze](https://www.amazon.com/dp/B0DFPKMPHB)|See below|Says bronze in listing, but what I got says copper filled|
|Black|?|Black ABS||These were sent off to a service to print, but PLA or PETG on a home printer should probably be fine|



### Copper filament profile

Bambu Studio export in this folder as `ELEGOO PLA Copper 1-2606-0619.bbsflmt` and is specific to Specific to Bambu Lab A1 mini 0.4 nozzle. Or you can follow these steps

1. Create new profile based on Generic PLA in the filament dropdown
2. Change these values:
	*  Nozzle: 215°C first layer, 210°C other layers
	*  Bed: 55°C (textured PEI)
	*  Max volumetric speed: 8 mm³/s. This is the key one for metal-filled PLA, which flows worse than plain PLA.

### Silver filament profile

Bambu Studio  export in this folder as `ELEGOO PLA Silk Silver Grey 1-2606-0612.bbsflmt` and is specific to Specific to Bambu Lab A1 mini 0.4 nozzle. Or you can follow these steps

1. Create new profile based on Generic PLA Silk
2. Change these values:
	* Nozzle: 225°C first layer, 220°C other layers
	* Bed: 55°C
	* Max volumetric speed: 7.5 mm³/s
	* Retraction -> Length: 0.5mm


## Myth Made Models

The items below were printed unmodified from the Myth Made CAD model. You can get them and Jamie's other alethiometer plans [here](https://mythmade.gumroad.com/l/trhgos). They are available for free, but please consider chipping in what you can afford like I did to support her wonderful work! This build would not be possible without her.

You will need to import the `.f3z` into [Autodesk Fusion](https://www.autodesk.com/products/fusion-360/personal), which is free for personal use. From there, you can right click on a specific component, and export to `.step` or Save As mesh to .stl or .3mf. Be sure to use High refinement if you are exporting to `.stl` or `.3mf`. If the part in question is a body and not a component, you will need to right click on the body and convert it into a component first.


|Name in model| Description| Filament |Notes|
|:--- |:--- |:--- |:--- |
|symbol holder|Holds the symbol tiles|Gold | Supports on base plate only|
|outer silver ring thing|Sits on top of the motor plate and holds the symbol holder|Silver | Supports on base plate only|
|Gears -> Middle Gear||Black||
|Gears -> Bodies -> Body11||Black||
|Gears -> Bodies -> Body1||Black||
|Gears -> Left gear||Black||
|Gears -> Middle Gear||Black||
|Gears -> Bodies -> Body2||Black||

### Drilling
Any gears above should have their axle hole drilled out to 3.5mm so it will spin freely on a M3 screw


## Models in this Repository

These models are a mix of new creations and modifications of models from Myth Made. `.step` files are in this directory and can be imported by slicers for printing or CAD programs to modify

If there is a `.py` file with the same name as a CAD file, that `.py` contains Fusion API python code that when run as a script in Fusion, creates the model. This parametric code is hypothetically easier for LLMs to work with and allows for easier modification via parameter changes. Fusion API functions are black boxes, but having the python code gives some clues as to how to recreate the model.


## lower_case

### Description

This is the outer case of the alethiometer in which everything lives. There is no upper case for this build. This case contains a window cutout for the touch sensor, and a cutout for plugging a USB cable to the Nano. There are also narrower slit cutouts for the three hand dials used to turn the gears inside. The gear train will move the three set pointers. 

### Printing

Default Bambu Studio settings for the Bambu A1 mini were used except for below

* **Filament:** Copper
* **Supports:** Off


### Assembly Notes
You will want to wait until you have the motor plate support wall before you do anything

## Motor Plate Support wall

### Description
This is a wall that fits inside the lower case. The wall runs along the inside of the lower case and designed to prop up the motor plate above the electronics below. This case contains a window cutout for the touch sensor, and a cutout for plugging a USB cable to the Nano. These cutouts should match the equivalent from the lower case.

### Printing

Default Bambu Studio settings for the Bambu A1 mini were used except for below

* **Filament:** Copper
* **Supports:** Off

Since this is not seen from the outside, any color or material should do


### Assembly Notes

1. Place this inside the lower case before you attach anything to the case.
2.  Make sure the windows line up. 
3. Plug the Nano into the half size breadboard so that the USB port is on the very outside edge. 
4. Note: There is a sticker you can peel off under the breadobard to reveal some glue that will hold the breadboard in place. 
5. The USB-port window is on the side with no hand wheel cutout. 
6. Place the breadboard with Nano onto the bottom of the lower case, with the USB port facing this window.

For the touch sensor, I used blu-tack to stick it against the touch sensor window and hold it in place. The touch sensor board has screw holes also that you can probably use if you want to drill through the motor plate support wall and/or lower case.

## Motor Plate
### Description
The motor plate holds the x27.168 motor and requires some modification after printing (see Assembly below). It will hold the motor, the gears, and dial faces.

### Printing

Default Bambu Studio settings for the Bambu A1 mini were used except for below

* **Filament:** Copper
* **Supports:** Off


### Assembly Notes



#### Drilling
Because 3D printed holes can be off in size, they were deliberately printed smaller than actual, so that the correct size could be made using a drill bit. I used a hand held manual pin vise drill for all of the below.

##### Motor
These are the two rectangular cutouts inside the inner circular wall near the center of the plate. These are intended for the wires of the x27.168 motor to go through to the bottom and plug into the breadboard.

There are also two circular holes that serve to hold the motor in place. The two pegs on the x27.168 slot into these. These need to be drilled out before use. The smaller hole should be drilled to a diameter of **2.5 mm**, and the larger hole is **3.8 mm**.

##### Axle Holes
There are 8 holes at the very bottom of the motor plate, outside of the circular wall, that serve as the holes for inserting a heat set screw thread. The screw threads will hold a screw that serves as the axle for the gears. These holes have a raised rim around them. I drilled these open to **4 mm**, but it might depend on which heat set inserts you use (see below).


##### Dial face fasteners
There are two "peninsulas" with holes inside the inner ring, and four holes closer to the outer, higher areas of the motor plate. These are intended for heat set screw thread inserts that will hold screws that fasten the dial faces onto the motor plate.

I did NOT drill them out further.


#### Heat Set Screw Thread Inserts

Heat set threaded inserts are designed so that you can

1. Place the heat set insert the right way into the hole
2. Use soldering iron with an adapter and special tip to heat the insert, which melts/softens the plastic and allows you to push down and put the insert into place

Most of the holes need to have a screw thread in order for the screws serving as the fasteners and axles to stay in place. I bought variety packs in M2 and M3, meaning each pack had a variety of thread insert lengths.

Here are some examples of Heat Set Inserts, ideally you should get one that comes with a soldering iron tip that fits the inserts:

* [Nefuree Threaded Inserts for Plastic, Heat Set, sizes M2 through M6 with tips and adapter](https://www.amazon.com/dp/B0GYD133M5)
* [Kadrick Heat Set Inserts Kit, M3, with tip](https://www.amazon.com/dp/B0FD88XTV4)
* [Ruthex M2x4 Threaded Inserts](https://www.amazon.com/dp/B088QJG676)
* [Ruthex Soldering Iron Tips](https://www.amazon.com/dp/B0DP51DDF7)


#### Axle Holes
Use M3 threaded inserts

#### Dial Face Fasteners
Use M2 threaded inserts



## Inner Gears, with integrated set pointer
These are three separate files, named
* `INN-T1.step`
* `INN-T2.step`
* `INN-T3.step`

These are the stacking inner gears that go around the inner ring of the motor plate. In the Myth Made build, a cut piece of flat wire was heated, then inserted into a groove on the inner gear. Then a CNC'd metal pointer was glued on top of that. 

I had a lot of trouble doing this, and ended up with deformed gears, or the flat wire just came off after it got attached. I also don't have access to a CNC. So I decided to go a different way, to print the set pointers directly attached to the inner gears.


### Printing

Default Bambu Studio settings for the Bambu A1 mini were used except for below

* **Filament:** First copper, then a pause as added in the slicer right before the set pointer, so you can switch the filament to switch to silver. You may not need this if you can print in multiple colors already.
* **Supports:** On, base support only

The connection between the gear and the set pointer is very thin and fragile, so you have to take great care when handling to not break them off. This print also require supports for the set pointer. 

I recommend when taking the gears off the plate, to first carefully scrape the supports off before taking off the entire gear. I recommend when taking off the supports, to put the gear upside down and pushing down on the gear ring so the thin connector is held in place and not pulled upon, and pulling the supports away from the ring towards the center

### Assembly Notes

1. Stack all the inner gears together BEFORE placing it in the motor plate.
2. Each set pointer is restricted to its own third circle on the dial face by the symbol holder, which is just as well, because the set pointers on these inner gears cannot slide by each other. Attempting to do so may break off the set pointers.

## Hand Dial
### Description
The three hand dials stick out of the motor plate in the cutouts. Turning these will turn the gears attached and turn the inner gear, which will move the set pointer around to the symbol of your choice. These are modified from Myth Made to make the divot on the bottom of the hand dial deeper, because I found the hand dial needed to seat lower for clearance.

### Printing

Default Bambu Studio settings for the Bambu A1 mini were used except for below

* **Filament:** Silver
* **Supports:** On, base support only

You might not need supports. If you use supports, you will need to tear out the supports at the bottom center of the hand dial. I used fine tweezers to do so.

### Drilling
As with all the gears, drill out the center hole with a **3.5 mm** diameter drill bit to so that it will spin freely around the screw without catching.

### Assembling
Hand dials are placed on the outermost gear positions. As with the other gears, you will need to use an M3 screw to serve as an axle for the gear to turn around. The head type is flat and counter sunk because it needs to seat low for proper clearance for other parts.

## Inner Plate
### Description
The inner plate in Myth Made consisted of two brass plates. I switched to 3D printing due to lack of CNC access. The inner plate here consists of one piece


### Printing

Default Bambu Studio settings for the Bambu A1 mini were used except for below

* **Filament:** Gold
* **Supports:** Off

### Assembling
Use silver colored M2 screws to screw to the "peninsula" thread inserts. Two of the screw holes are one size, and a third is different. You may have to trial and error rotate so that the screw holes line up with the peninsula.

This should be placed after the motor is in place, but before inner gears are installed

## Outer Dial Face

### Description
The outer dial face in Myth Made consisted of two aluminum plates, and is much more intricate. I switched to 3D printing a simpler face due to lack of CNC access.


### Printing

Default Bambu Studio settings for the Bambu A1 mini were used except for below

* **Filament:** Silver
* **Supports:** Off

### Assembling
This should be the second to last last item installed on the alethiometer. Screw into holes using silver colored M2 screws







