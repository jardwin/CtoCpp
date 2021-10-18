// SDL_Test.h: Includedatei f�r Include-Standardsystemdateien
// oder projektspezifische Includedateien.

#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <random>

// Defintions
constexpr double frame_rate = 60.0; // refresh rate
constexpr double frame_time = 1. / frame_rate;
constexpr unsigned frame_width = 1400; // Width of window in pixel
constexpr unsigned frame_height = 900; // Height of window in pixel
// Minimal distance of animals to the border
// of the screen
constexpr unsigned frame_boundary = 100;

// Helper function to initialize SDL
void init();

enum DIRECTION
{
	HORIZONTAL,
	VERTICAL
};

class animal {
private:
	SDL_Surface* window_surface_ptr_; // ptr to the surface on which we want the
									  // animal to be drawn, also non-owning
	SDL_Surface* image_ptr_; // The texture of the sheep (the loaded image), use
							 // load_surface_for
protected:
	SDL_Rect position_;
	int targetX, targetY;
	int getRandomSpawn(DIRECTION dir);
	int getRandomTarget(int bounding, DIRECTION dir);
public:
	animal(const std::string& file_path, SDL_Surface* window_surface_ptr);
	~animal();

	void draw();

	virtual void move() =0;
	// todo: Animals move around, but in a different
							   // fashion depending on which type of animal
};

// Insert here:
// class sheep, derived from animal
class sheep : public animal {
public:
	sheep(SDL_Surface* window_surface_ptr);
	~sheep() {}
	void move();
	// implement functions that are purely virtual in base class
};

// Insert here:
// class wolf, derived from animal
class wolf : public animal {
public:
	wolf(SDL_Surface* window_surface_ptr);
	~wolf() {}
	// implement functions that are purely virtual in base class
	void move();

};
// Use only sheep at first. Once the application works
// for sheep you can add the wolves

// The "ground" on which all the animals live (like the std::vector
// in the zoo example).
class ground {
private:
	// Attention, NON-OWNING ptr, again to the screen
	SDL_Surface* window_surface_ptr_;

	std::vector<animal*>* animals_;

public:
	ground(SDL_Surface* window_surface_ptr);
	~ground();

	// Add an animal
	void add_animal(animal* newAnimal);

	// "refresh the screen": Move animals and draw them
	void update();
	// todo: "refresh the screen": Move animals and draw them
	// Possibly other methods, depends on your implementation
};

// The application class, which is in charge of generating the window
class application {
private:
	// The following are OWNING ptrs
	SDL_Window* window_ptr_;
	SDL_Surface* window_surface_ptr_;
	SDL_Event window_event_;

	ground* ground_;
public:
	application(unsigned n_sheep, unsigned n_wolf);
	~application();

	int loop(unsigned period);
	// main loop of the application.
							   // this ensures that the screen is updated
							   // at the correct rate.
							   // See SDL_GetTicks() and SDL_Delay() to enforce a
							   // duration the application should terminate after
							   // 'period' seconds
};

