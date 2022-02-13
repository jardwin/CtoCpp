#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <vector>

#define PI 3.14159265

// Defintions
constexpr int number_of_dogs = 2;
constexpr double frame_rate = 60.0; // refresh rate
constexpr double frame_time = 1. / frame_rate;
constexpr unsigned frame_width = 1400 / 2; // Width of window in pixel
constexpr unsigned frame_height = 900 / 2; // Height of window in pixel
// Minimal distance of animals to the border
// of the screen
constexpr unsigned frame_boundary = 100;

constexpr unsigned max_growing = 100;

// Helper function to initialize SDL
void init();

enum DIRECTION { HORIZONTAL, VERTICAL };

// ------------------------- Animal class -------------------------
class animal {
private:
  SDL_Surface* window_surface_ptr_; // ptr to the surface on which we want the
                                    // animal to be drawn, also non-owning
protected:
  SDL_Surface* image_ptr_; // The texture of the sheep (the loaded image), use
                           // load_surface_for
  int speed;
  int targetX, targetY;
  int getRandomSpawn(DIRECTION dir);
  int getRandomTarget(int bounding, DIRECTION dir);
  int getRandomSex();
  bool isOnTarget();

public:
  int growingPerPourcent;
  SDL_Rect position_;
  animal(const std::string& file_path, SDL_Surface* window_surface_ptr);
  ~animal();

  bool isOnCouple(const std::shared_ptr<animal>&);
  void draw();
  void setSpeed(int newSpeed);
  void runAway(const std::shared_ptr<animal>&);

  virtual void move();
  virtual void update();
  // todo: Animals move around, but in a different
  // fashion depending on which type of animal
};

// ------------------------- Sheep class -------------------------
// class sheep, derived from animal
class sheep : public animal {
private:
  unsigned birthday;
  void growingUp();

public:
  bool isFemal;
  sheep(SDL_Surface* window_surface_ptr);
  sheep(SDL_Surface* window_surface_ptr, int x, int y);
  ~sheep() {}
  void isChild();
  void update();
  // implement functions that are purely virtual in base class
};

// ------------------------- Wolf class -------------------------
// class wolf, derived from animal
class wolf : public animal {
public:
  wolf(SDL_Surface* window_surface_ptr);
  ~wolf() {}
  void setTarget(std::shared_ptr<animal>& target);
  // implement functions that are purely virtual in base class
};

// ------------------------- Shepherd class -------------------------
class shepherd {
public:
  SDL_Surface* window_surface_ptr_;
  SDL_Rect position_;
  shepherd(SDL_Surface* window_surface_ptr);
  ~shepherd();
  void move(const SDL_Event& event, bool keys[322]);

private:
  SDL_Surface* image_ptr_;
};

// ------------------------- Shepherd dog class -------------------------
class shepherd_dog : public animal {
public:
  SDL_Surface* window_surface_ptr_;
  std::shared_ptr<shepherd> shepherd_master;
  float degree;
  // variable determinating the "speed" of the dogs so they are not glued
  // altogether
  float degreeIncrement = getRandomAngle();

  shepherd_dog(std::shared_ptr<shepherd>& master,
               SDL_Surface* window_surface_ptr);
  ~shepherd_dog() {}
  void move();

private:
  float getRandomAngle();
};

// ------------------------- Ground class -------------------------
// The "ground" on which all the animals live (like the std::vector
// in the zoo example).
class ground {
private:
  // Attention, NON-OWNING ptr, again to the screen
  SDL_Surface* window_surface_ptr_;

  std::vector<std::shared_ptr<animal>> animals_;

public:
  ground(SDL_Surface* window_surface_ptr);
  ~ground();

  // Add an animal
  void add_animal(std::shared_ptr<animal> newAnimal);

  void appendOffspring(const std::shared_ptr<sheep>& first,
                       const std::shared_ptr<sheep>& second);

  // "refresh the screen": Move animals and draw them
  void update();
  // todo: "refresh the screen": Move animals and draw them
  // Possibly other methods, depends on your implementation

  // count the number of sheeps in animals vector
  unsigned countSheep(unsigned nbSheep);
};

// ------------------------- Application class -------------------------
// The application class, which is in charge of generating the window
class application {
private:
  // The following are OWNING ptrs
  SDL_Window* window_ptr_;
  SDL_Surface* window_surface_ptr_;
  SDL_Event window_event_;
  SDL_Surface* score_surface_ptr_;
  SDL_Rect* score_position_;

  TTF_Font* font;

  std::unique_ptr<ground> ground_;

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
