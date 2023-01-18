/*
 * ECS7012 Music and Audio Programming
 *
 * This code runs on the Bela embedded audio platform (bela.io).
 * sounds.h code was based on a template code for pre-loading samples into buffers from:
 * drum machine code, by
 * Andrew McPherson, Becky Stewart and Victor Zappi
 * 2015-2020
 */

#ifndef _SOUNDS_H
#define _SOUNDS_H

#define NUMBER_OF_SOUNDS 8

/* Start playing a particular drum sound */
void startPlayingSound(int soundIndex);

#endif /* _SOUNDS_H */
