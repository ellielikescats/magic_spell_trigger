# Magic Spell Trigger and Bubble Simulation Instrument

Initially, the project was designed as a bubble instrument, where magic sounds were triggered from the rate of flow of the bubbles with each call to render(). 
The intention was to create a magical and immersive soundscape, that in some way provided room for thought and reflection. 
The idea for a bubble instrument was inspired by the use of ultrasonic sensors to detect the flow of bubbles with Arduino. When trying to determine which sensor would be best to use to detect the presence of bubbles and bubble flow, I decided to try using an IR sensor after reading a paper on bubble detection in crude oil (https://www.mdpi.com/1424-8220/17/6/1278). 
However, it was found that using an IR sensor was not sensitive enough to detect bubbles and the design of the instrument then evolved into a Magic Spell Trigger.


## Design and IO


The instrument is designed with four main I/O components (IR sensor, LED light, 2 buttons) and these are specifically laid out on the breadboard for ease of access. An LED light is set up at the back (bottom right-hand side) of the breadboard to indicate if a looping sample buffer is playing, and in front of the LED light are two buttons; one to turn on/off a looping sample of a magical bell soundscape, and the other to turn on/off the magic effects triggered by an IR sensor.
An IR sensor is also attached from a distance to the breadboard, to enable flexibility when moving the IR sensor around the area away from the breadboard. This also enables the user to hold the sensor cables and allows for more interaction with the sensor.
Eight sound samples were pre-loaded into buffers1, using the a modified version of the code provided with QMUL Music and Audio Programming module Assignment 2. The looping sample of a magical bell soundscape was added to the design to create a more immersive atmosphere. This can be turned on and off. The sound samples are all downloaded from www.freesound.com

The two main design features of the code are the users choice between the interactive magic spell trigger, and the passive (but equally as enchanting) bubble simulation instrument. In the bubble simulation instrument, the code randomly generates ‘bubbles’ or numbers, and each time a specific number is detected, a ‘hit’ is recorded and an instance of the bubble slot is called to and filled with a magic sample sound from a pre-loaded buffer. A for() loop then goes through all the slots repeatedly, and iterates read pointer through any slots that have filled. The random sound allocated to this slot then plays. Up to 10 sounds are able to be added to the final sound mix.
The magic spell trigger builds upon this concept, but a ‘hit’ is recorded when the input from the IR sensor has been tallied to a specfic number after each call to render(). When a hit is recorded, the slot instances work in the same way as described about, and this will be explored in more detail in the implementation section below.
