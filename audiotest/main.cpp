#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>

bool playing = true;
bool day = true;

void background();
void ambient();

int main(int argc, char **argv)
{
	std::thread t1(background);
	std::thread t2(ambient);

	int a;
	std::cout <<"Input a character then press enter to end program\n";
	std::cin >> a;
	playing = false;
	
	system("killall play");
	t1.join();
	t2.join();
	
    return 0;
}

void background()
{
	FILE* file;
	int val;
	while (playing)
	{
		file = popen("pgrep -c play", "r");
		val = getc(file);
		if (val < '2')
		{
			system("(play -q water.wav fade h 1 15 1)");
		}
	}	
}

void ambient()
{
	FILE* file;
	int val;
	while (playing)
	{
		file = popen("pgrep -c play", "r");
		val = getc(file);
		if (val < '2')
		{
			if (day)
            {
                system("(play -q seagull.wav fade h 3 15 3)");
                day = false;
            } else
            {
                system("(play -q crickets.wav fade h 3 15 3)");
                day = true;
            }
		}
	}	
}