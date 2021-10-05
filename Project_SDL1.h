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

class animal {
private:
	SDL_Surface* window_surface_ptr_; // ptr to the surface on which we want the
									  // animal to be drawn, also non-owning
	SDL_Surface* image_ptr_; // The texture of the sheep (the loaded image), use
							 // load_surface_for
protected:
	SDL_Rect position_;
	int targetX, targetY;
	int getRandom(int max) {
		std::random_device                  rand_dev;
		std::mt19937                        generator(rand_dev());
		std::uniform_int_distribution<int>  distr(frame_boundary, max);
		return distr(generator);
	}
public:
	animal(const std::string& file_path, SDL_Surface* window_surface_ptr) {
		image_ptr_ = IMG_Load(file_path.c_str());
		window_surface_ptr_ = window_surface_ptr;
		position_.x = 0;
		position_.y = 0;
		position_.w = image_ptr_->w;
		position_.h = image_ptr_->h;

		targetX, targetY = 0;
	};
	~animal() {
		SDL_FreeSurface(image_ptr_);
	};

	void draw() {
		SDL_BlitScaled(image_ptr_, NULL, window_surface_ptr_, &position_);
	};

	virtual void move() {}; // todo: Animals move around, but in a different
							   // fashion depending on which type of animal
};

// Insert here:
// class sheep, derived from animal
class sheep : public animal {
public:
	sheep(SDL_Surface* window_surface_ptr) : animal("./media/sheep.png", window_surface_ptr) {
		this->position_.x = getRandom(frame_width - frame_boundary);
		this->position_.y = getRandom(frame_height - frame_boundary);
		this->targetX = getRandom(frame_width - frame_boundary);
		this->targetY = getRandom(frame_height - frame_boundary);
	}
	~sheep() {}
	void move() {
		if (position_.x > targetX) {
			position_.x--;
		}
		else if (position_.x < targetX) {
			position_.x++;
		}
		if (position_.y > targetY) {
			position_.y--;
		}
		else if (position_.y < targetY) {
			position_.y++;
		}
		if (targetX == position_.x && targetY == position_.y)
		{
			this->targetX = getRandom(frame_width - frame_boundary);
			this->targetY = getRandom(frame_height - frame_boundary);
		}
	}
	// implement functions that are purely virtual in base class
};

// Insert here:
// class wolf, derived from animal
class wolf : public animal {
public:
	wolf(SDL_Surface* window_surface_ptr) : animal("./media/wolf.png", window_surface_ptr) {
		this->position_.x = getRandom(frame_width - frame_boundary);
		this->position_.y = getRandom(frame_height - frame_boundary);

		this->targetX = getRandom(frame_width - frame_boundary);
		this->targetY = getRandom(frame_height - frame_boundary);
	}
	~wolf() {}
	// implement functions that are purely virtual in base class
	void move() {
		if (position_.x > targetX) {
			position_.x--;
		}
		else if (position_.x < targetX) {
			position_.x++;
		}
		if (position_.y > targetY) {
			position_.y--;
		}
		else if (position_.y < targetY) {
			position_.y++;
		}
		if (targetX == position_.x && targetY == position_.y)
		{
			this->targetX = getRandom(frame_width - frame_boundary);
			this->targetY = getRandom(frame_height - frame_boundary);
		}
	}
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
	ground(SDL_Surface* window_surface_ptr) {
		window_surface_ptr_ = window_surface_ptr;
		animals_ = new std::vector<animal*>();
	}// todo: Ctor
	~ground() {
		delete animals_;
	}; // todo: Dtor, again for clean up (if necessary)
	void add_animal(animal* newAnimal) {
		animals_->push_back(newAnimal);
	} // todo: Add an animal
	void update() {
		for (int i = 0; i < animals_->size(); i++)
		{
			animals_->at(i)->move();
			animals_->at(i)->draw();
		}
	} // todo: "refresh the screen": Move animals and draw them
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
	application(unsigned n_sheep, unsigned n_wolf) {
		// Create an application window with the following settings:
		window_ptr_ = SDL_CreateWindow(
			"An SDL2 window",                  // window title
			SDL_WINDOWPOS_UNDEFINED,           // initial x position
			SDL_WINDOWPOS_UNDEFINED,           // initial y position
			frame_width,                               // width, in pixels
			frame_height,                               // height, in pixels
			SDL_WINDOW_SHOWN // flags - see below
		);

		window_surface_ptr_ = SDL_GetWindowSurface(window_ptr_);

		ground_ = new ground(window_surface_ptr_);

		for (size_t i = 0; i < n_sheep; i++)
			ground_->add_animal(new sheep(window_surface_ptr_));

		for (size_t i = 0; i < n_wolf; i++)
			ground_->add_animal(new wolf(window_surface_ptr_));
	}
	~application() {
		// Close and destroy the window
		SDL_DestroyWindow(window_ptr_);
	}

	int loop(unsigned period) {
		SDL_Rect windowsRect = SDL_Rect{ 0,0,frame_width, frame_height };
		while (period * 1000 >= SDL_GetTicks()) {
			SDL_FillRect(window_surface_ptr_, &windowsRect, SDL_MapRGB(window_surface_ptr_->format, 0, 255, 0));
			SDL_PollEvent(&window_event_);
			ground_->update();
			SDL_UpdateWindowSurface(window_ptr_);
			SDL_Delay(frame_time * 1000);  // Pause execution for framerate milliseconds
		}
		return 1;
	}// main loop of the application.
							   // this ensures that the screen is updated
							   // at the correct rate.
							   // See SDL_GetTicks() and SDL_Delay() to enforce a
							   // duration the application should terminate after
							   // 'period' seconds
};
