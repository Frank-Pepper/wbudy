int const Notes[8][12] = {
	{ 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62 },
	{ 65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123 },
	{ 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247 },
	{ 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494 },
	{ 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988 },
	{ 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976 },
	{ 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951 },
	{ 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902 }
};

#define NOTE_C 0
#define NOTE_CS 1
#define NOTE_D 2
#define NOTE_DS 3
#define NOTE_E 4
#define NOTE_F 5
#define NOTE_FS 6
#define NOTE_G 7
#define NOTE_GS 8
#define NOTE_A 9
#define NOTE_AS 10
#define NOTE_B 11

// https://pianoletternotes.blogspot.com/2018/06/doom-theme.html TODO notes

inline void noteDoomBase(int octave, int speed) {
	playNote(octave - 1, NOTE_E, speed / 2);
	delay(speed / 2);
	playNote(octave - 1, NOTE_E, speed);
}

static uint32_t Doom[] = {
    NOTE_DS, 3, NOTE_DS, 3, NOTE_DS, 4, NOTE_DS, 3, NOTE_DS, 3, NOTE_CS, 4, NOTE_DS, NOTE_DS, 3, NOTE_B, 3, NOTE_DS, 3, NOTE_DS, 3, , NOTE_A, 3,
    NOTE_DS, 3,NOTE_AS, 3,NOTE_B, 3,
    NOTE_DS, 3, NOTE_DS, 3, NOTE_DS, 4, NOTE_DS, 3, NOTE_DS, 3, NOTE_CS, 4, NOTE_DS, NOTE_DS, 3, NOTE_B, 3, NOTE_DS, 3, NOTE_DS, 3, , NOTE_A, 3,

    NOTE_DS, 3,NOTE_A,
    NOTE_DS, 3, NOTE_DS, 3, NOTE_DS, 4, NOTE_DS, 3, NOTE_DS, 3, NOTE_CS, 4, NOTE_DS, NOTE_DS, 3, NOTE_B, 3, NOTE_DS, 3, NOTE_DS, 3, , NOTE_A, 3,
    NOTE_DS, 3,NOTE_A, NOTE_DS, 3, NOTE_DS, 3, NOTE_AS, 3, NOTE_B, 3, NOTE_DS, 3, NOTE_DS, 3, NOTE_DS, 4, NOTE_DS, 3
    
};

void loop()
{
	int octave = 3;

	// Fast part
	int speed = 64;

	playNote(octave, NOTE_B, speed);
	playNote(octave, NOTE_G, speed);
	playNote(octave, NOTE_E, speed);
	playNote(octave, NOTE_C, speed);

	playNote(octave, NOTE_E, speed);
	playNote(octave, NOTE_G, speed);
	playNote(octave, NOTE_B, speed);
	playNote(octave, NOTE_G, speed);

	playNote(octave, NOTE_B, speed);
	playNote(octave, NOTE_G, speed);
	playNote(octave, NOTE_E, speed);
	playNote(octave, NOTE_G, speed);

	playNote(octave, NOTE_B, speed);
	playNote(octave, NOTE_G, speed);
	playNote(octave, NOTE_B, speed);
	playNote(octave + 1, NOTE_E, speed);

	// Main theme
	speed = 128;

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_E, speed);

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_D, speed);

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_C, speed);

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_AS, speed);

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_B, speed);
	playNote(octave, NOTE_C, speed);

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_E, speed);

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_D, speed);

	noteDoomBase(octave, speed);
	playNote(octave, NOTE_C, speed);

	noteDoomBase(octave, speed);
	playNote(octave - 1, NOTE_AS, speed * 2);
	
	delay(4000); // Wait 4 seconds before repeating
}