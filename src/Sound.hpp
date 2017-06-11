// Authorship: Jeremy Hughes (a1646624)
// Sound class
// A class to manage the ambient background sounds

#ifndef SOUND_HPP
#define SOUND_HPP

#include <stdio.h>
#include <vector>
#include <string>

class Sound
{
public:
	void initialize();
	void cleanup();

	void play(std::string filename);
	int playing_now();
	
	bool enabled;
};

#endif // SOUND_HPP
