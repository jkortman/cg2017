#include <fstream>
#include <iostream>
#include <string>
#include <thread>

bool playing = true;

void background();
void ambient();

int main(int argc, char **argv)
{
	std::fstream file;
	std::thread t1(background);
	std::thread t2(ambient);

	int a;
	std::cout <<"Press input something to join threads, then ctrl-c once or twice it end early \n";
	std::cin >> a;
	playing = false;
	t1.join();
	t2.join();
	std::cout << a << "\n";

	
    return 0;
}

void background()
{
	while (playing)
	{
		std::cout << playing <<"\n";
		system("play -q water.wav fade h 1 15 1");
	}	
}

void ambient()
{
	while (playing)
	{
		system("play -q seagull.wav fade h 3 15 3");
		system("play -q crickets.wav fade h 3 15 3");
	}
}