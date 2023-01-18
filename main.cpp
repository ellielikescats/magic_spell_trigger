/*
 * ECS7012 Music and Audio Programming
 *
 * This code runs on the Bela embedded audio platform (bela.io).
 * main.cpp code was based on a template code for pre-loading samples into buffers from:
 * drum machine code, by
 * Andrew McPherson, Becky Stewart and Victor Zappi
 * 2015-2020
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <signal.h>
#include <getopt.h>
#include <libraries/sndfile/sndfile.h>
#include <Bela.h>
#include "sounds.h"

using namespace std;


/* Magic samples are pre-loaded in these buffers. Length of each
 * buffer is given in gDrumSampleBufferLengths.
 */
float *gMagicSampleBuffers[NUMBER_OF_SOUNDS];
int gMagicSampleBufferLengths[NUMBER_OF_SOUNDS];

// Handle Ctrl-C by requesting that the audio rendering stop
void interrupt_handler(int var)
{
	gShouldStop = true;
}

// Print usage information
void usage(const char * processName)
{
	cerr << "Usage: " << processName << " [options]" << endl;

	Bela_usage();

	cerr << "   --help [-h]:                Print this menu\n";
}

int initSounds() {
	/* Load magic sounds from WAV files */
	SNDFILE *sndfile ;
	SF_INFO sfinfo ;
	char filename[64];

	for(int i = 0; i < NUMBER_OF_SOUNDS; i++) {
		snprintf(filename, 64, "./magic%d.wav", i);

		if (!(sndfile = sf_open (filename, SFM_READ, &sfinfo))) {
			printf("Couldn't open file %s\n", filename);

			/* Free already loaded sounds */
			for(int j = 0; j < i; j++)
				free(gMagicSampleBuffers[j]);
			return 1;
		}

		if (sfinfo.channels != 1) {
			printf("Error: %s is not a mono file\n", filename);

			/* Free already loaded sounds */
			for(int j = 0; j < i; j++)
				free(gMagicSampleBuffers[j]);
			return 1;
		}

		gMagicSampleBufferLengths[i] = sfinfo.frames;
		gMagicSampleBuffers[i] = (float *)malloc(gMagicSampleBufferLengths[i] * sizeof(float));
		if(gMagicSampleBuffers[i] == NULL) {
			printf("Error: couldn't allocate buffer for %s\n", filename);

			/* Free already loaded sounds */
			for(int j = 0; j < i; j++)
				free(gMagicSampleBuffers[j]);
			return 1;
		}

		int subformat = sfinfo.format & SF_FORMAT_SUBMASK;
		int readcount = sf_read_float(sndfile, gMagicSampleBuffers[i], gMagicSampleBufferLengths[i]);

		/* Pad with zeros in case whole file couldn't be read */
		for(int k = readcount; k < gMagicSampleBufferLengths[i]; k++)
			gMagicSampleBuffers[i][k] = 0;

		if (subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE) {
			double	scale ;
			int 	m ;

			sf_command (sndfile, SFC_CALC_SIGNAL_MAX, &scale, sizeof (scale)) ;
			if (scale < 1e-10)
				scale = 1.0 ;
			else
				scale = 32700.0 / scale ;
			printf("Scale = %f\n", scale);

			for (m = 0; m < gMagicSampleBufferLengths[i]; m++)
				gMagicSampleBuffers[i][m] *= scale;
		}

		sf_close(sndfile);
	}

	return 0;
}

void cleanupSounds() {
	for(int i = 0; i < NUMBER_OF_SOUNDS; i++)
		free(gMagicSampleBuffers[i]);
}

int main(int argc, char *argv[])
{
	BelaInitSettings  settings;	// Standard audio settings

	struct option customOptions[] =
	{
		{"help", 0, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	// Set default settings
	Bela_defaultSettings(&settings);
	settings.setup = setup;
	settings.render = render;
	settings.cleanup = cleanup;

	// Parse command-line arguments
	while (1) {
		int c;
		if ((c = Bela_getopt_long(argc, argv, "hf:", customOptions, &settings)) < 0)
				break;
		switch (c) {
		case 'h':
				usage(basename(argv[0]));
				exit(0);
		case '?':
		default:
				usage(basename(argv[0]));
				exit(1);
		}
	}

	// Load the magic sounds
    if(initSounds()) {
    	printf("Unable to load magic sounds. Check that you have all the WAV files!\n");
    	return -1;
    }

	// Initialise the PRU audio device
	if(Bela_initAudio(&settings, 0) != 0) {
		cout << "Error: unable to initialise audio" << endl;
		return -1;
	}


	// Start the audio device running
	if(Bela_startAudio()) {
		cout << "Error: unable to start real-time audio" << endl;
		return -1;
	}

	// Set up interrupt handler to catch Control-C
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);

	// Run until told to stop
	while(!gShouldStop) {
		usleep(100000);
	}

	// Stop the audio device and sensor thread
	Bela_stopAudio();

	// Clean up any resources allocated for audio
	Bela_cleanupAudio();

	// Clean up the magic sounds
	cleanupSounds();

	// All done!
	return 0;
}
