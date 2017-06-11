/*
Authorship: Jeremy Hughes (a1646624)
*/
#include "Sound.hpp"

#include <cstdlib>

void Sound::initialize()
{
	// Need to verify SoX and pgrep are available on the system
	// pgrep & SoX should be available on most linux systems
	// pgrep should be available on OSX versions 10.8+
	// SoX is installable on OSX

	// The terminal command "which <something>" gives directory of <something> if installed, nothing if not.
	// popen gets the result of that command to check if a value was returned.
	// Sound is disable if either SoX or pgrep is unavailalbe.
	// As these are system() commands, their absense doesn't affect compilation

	this->enabled = true;

    FILE* file;
    int val;
    file = popen("which play", "r");
    val = getc(file);
    if (val == -1)
    {
        printf("SoX unavailable.\n");
        this->enabled = false;
    }
    file = popen("which pgrep", "r");
    val = getc(file);
    if (val == -1)
    {
        printf("pgrep unavailable.\n");
        this->enabled = false;
    }
    if (!(this->enabled))
    {
        printf("Background sounds disabled.\n");
    }
}

// Plays the requested sound in quiet mode with a fade in/out effect
void Sound::play(std::string filename)
{
	int fade;
	float time = 27;
	if (filename == "sea") fade = 1;
	else fade = 3;
	std::string command = "(play -q sounds/" + filename + ".wav fade h " + std::to_string(fade) + " " + std::to_string(time) + " " + std::to_string(fade) + ")&";
	system(command.c_str());
}

// Requests number of active 'play' processes.
int Sound::playing_now()
{
	FILE* file = popen("pgrep -c play", "r");
    int val = getc(file);

    if (val <= '0') return 0;
    if (val == '1') return 1;
    if (val == '2') return 2;
    return -1;
}

// Stops any active 'play' processes
void Sound::cleanup()
{
    system("killall play");
}