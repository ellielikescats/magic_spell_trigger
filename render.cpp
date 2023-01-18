/*
 * ECS7012 Music and Audio Programming
 *
 * Final Project - Eleanor Row
 *
 * This code runs on the Bela embedded audio platform (bela.io).
 */

#include <Bela.h>
#include <cmath>
#include "sounds.h"
#include <vector>
#include <iostream>
#include <random>

//Magic sound samples are pre-loaded in these buffers. Length of each
//buffer is given in gMagicSampleBufferLengths.
extern float *gMagicSampleBuffers[NUMBER_OF_SOUNDS];
extern int gMagicSampleBufferLengths[NUMBER_OF_SOUNDS];

// global variable for readPointers of no. of sounds
int gReadPointers[NUMBER_OF_SOUNDS];

// Global variable that determines whether there should be any sound from the Bela
// If set to false, output buffers will be filled with zeros
bool gIsPlaying = false;

// Looping variables
// The index of the sound in the gMagicSampleBuffer that we want to play
unsigned int gLoopSelection = 0;
unsigned int gLastLoopButtonValue;
bool gLoopState = false;
unsigned int gLoopReadPointer = 0;

// Bubble variables
// True if any sound from the bubbles should be written to the bela output buffer, false otherwise
bool gBubbleState = false;
unsigned int gLastBubbleButtonValue;

//Determine no. of slots to fill (change this alongside BubbleSoundSlot slots[])
int numberOfSoundsToPlaySimultaneously = 10;
// Data structure to hold the sound slot data
struct BubbleSoundSlot {
	public:
		bool isPlaying = false;
		unsigned int soundIndex = -1;
		unsigned int readPointer = 0;
		
		unsigned int getNextReadPointer(){
			readPointer++;
			return readPointer;
		}
		
		void reset(){
			isPlaying = false;
			soundIndex = -1;
			readPointer = 0;
		}
};

// Determine no. of slots to fill (change this alongside numberOfSoundsToPlaySimultaneously)
BubbleSoundSlot slots[10];


//global variables for digital I/O
const int kButtonPin1 = 0;
const int kButtonPin2 = 1;
const int kInfraRedPin = 2;
const int kLedPin = 3;

// counter variable that keeps track of each call to render
int counter = 0;

// Variables to generate random number for simulation of bubble detection
std::random_device rd;
std::mt19937 gen;
std::uniform_int_distribution<> randomSimulationInt;
std::uniform_int_distribution<> randomInt;

bool setup(BelaContext *context, void *userData)
{
	/* Step 2: initialise GPIO pins */
	pinMode(context, 0, kButtonPin1, INPUT); 
	pinMode(context, 0, kButtonPin2, INPUT); 
	pinMode(context, 0, kInfraRedPin, INPUT);
	pinMode(context, 0, kLedPin, OUTPUT);
	
	// Setup random number generator
    gen = std::mt19937(rd()); // seed the generator
    randomInt = std::uniform_int_distribution<>(1, 7); // define the range
    randomSimulationInt = std::uniform_int_distribution<>(1, 100000); // define the range
    
    for(int i = 0; i < numberOfSoundsToPlaySimultaneously; i++){
    	slots[i] = BubbleSoundSlot();
    }
        
	return true;
}


void render(BelaContext *context, void *userData)
{
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		// Read the value of button inputs
		int currentButtonInput1 = digitalRead(context, n, kButtonPin1);
		int currentButtonInput2 = digitalRead(context, n, kButtonPin2);
		
		// Read the value of potentiometer input
		int currentInfraRedInput = digitalRead(context, n, kInfraRedPin);
		
		// Loop button
		if(currentButtonInput1 == LOW && gLastLoopButtonValue == HIGH) {
			gLoopState = !gLoopState;
			// start loop from beginning every time button is pressed
			if(gLoopState == true)
				gLoopReadPointer = 0;
			//rt_printf("%i\n", gLoopState);
		}
		
		// Bubble button
		if(currentButtonInput2 == LOW && gLastBubbleButtonValue == HIGH) {
			gBubbleState = !gBubbleState;
			rt_printf("%i\n", gBubbleState);
		}
		
			// LED
		digitalWriteOnce(context, n, kLedPin, currentButtonInput1 == 1 ? LOW : HIGH);
		
		// Update last loop button value
		gLastLoopButtonValue = currentButtonInput1;
		gLastBubbleButtonValue = currentButtonInput2;
		
		// ===================== PLAYING LOGIC =====================
		// global "is playing" state is determined through logical or between 
		// sounds from the loop and sounds from the bubbles
		gIsPlaying = gLoopState || gBubbleState;
		
		// Silence
		if(!gIsPlaying){
			audioWrite(context, n, 0, 0.0f);
    		audioWrite(context, n, 1, 0.0f);
    		continue;
		}
		
		// ======= WE ARE PLAYING ========
		float in;
		float out = 0.0f;
		
		// If the loop button is activated
		if(gLoopState){
			in = gMagicSampleBuffers[gLoopSelection][gLoopReadPointer];
			// Add loop sample to out
			out += in;
        
        	// Iterate the read pointer for the looped sample
    		gLoopReadPointer++;
    		if(gLoopReadPointer > gMagicSampleBufferLengths[gLoopSelection]){
    			// Reset the read pointer
    			gLoopReadPointer = 0;
    		}
		} 
		
		
		// ================== BUBBLE LOGIC ===================
		// Below is the code for the intended bubble machine (bubble detection simulated without IR sensor),
		// and also the code for the IR sensor to detect a magic wand.
	
		
		// If the bubble state is triggered
		// choose between bubble simulation soundscape, or magic wand spell trigeer
		if(gBubbleState){
			bool hit = false;
			/*
			// Generates random bubbles without infrared 

			int randomNumber = randomSimulationInt(gen);
			if(randomNumber == 3){
				hit = true;
				//rt_printf("hittt\n");
			}
			*/
			
			// Infrared sensor code for bubble detection
			if(currentInfraRedInput == 0){
				counter++;
				if(counter == 10000){
					hit = true;
					counter = 0;
				}
			}
			else {
				counter = 0;
			}
			
			
			
			// If there is a bubble/wand detected, hit = true
			if(hit == true){
				// Start playing new random magic sound
				int randomSoundIndex = randomInt(gen);
				startPlayingSound(randomSoundIndex);
			}
			
			
			// For loop through all the filled up slots
			for(int i = 0; i < numberOfSoundsToPlaySimultaneously; i++){
				// if a particular slot is playing
				if(slots[i].isPlaying){
					// increment read pointer through the buffer assigned to that slot
					int readPointer = slots[i].getNextReadPointer();
					if(readPointer >= gMagicSampleBufferLengths[slots[i].soundIndex]){
						// reset read pointer when reaching end of buffer
						slots[i].reset();
						continue;
					}
					// add the sample to the mix
					float res = gMagicSampleBuffers[slots[i].soundIndex][readPointer];
					out += res;
				}
			}
		}
		
		// Attenuate output sample
		float multiplier = 1.0f;
		if(gLoopState && gBubbleState)
			multiplier = 0.5f;
		
		out *= multiplier;
		// Write the single or combined (loop + bubbles) output sample to the buffer
		audioWrite(context, n, 0, out);
		audioWrite(context, n, 1, out);
			
	}
		
}

/* Start playing a particular sound given by soundIndex, and store the sounds in an array of slots.
 */
void startPlayingSound(int soundIndex) {
	unsigned int slotIndex = 0;
		
	// Take first slot
	BubbleSoundSlot currentSlot = slots[slotIndex];
	
	bool found = false;
	for(int i = 0; i < numberOfSoundsToPlaySimultaneously; i++){
		found = !slots[i].isPlaying;
		if(found){
			slotIndex = i;
			break;
		}
	}
	if(!found){
		rt_printf("All slots taken... returning");
		return;
	}
	
	// Found a free slot
	slots[slotIndex].isPlaying = true;
	slots[slotIndex].soundIndex = soundIndex;
	slots[slotIndex].readPointer = 0;
	rt_printf("Found a free slot with slot index %i\n", slotIndex);
}

// cleanup_render() is called once at the end, after the audio has stopped.
// Release any resources that were allocated in initialise_render().

void cleanup(BelaContext *context, void *userData)
{

}
