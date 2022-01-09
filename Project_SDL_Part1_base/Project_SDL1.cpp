// SDL_Test.cpp: Definiert den Einstiegspunkt f�r die Anwendung.
//

#include "Project_SDL1.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>

void init() {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0)
    throw std::runtime_error("init():" + std::string(SDL_GetError()));

  // Initialize PNG loading
  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags))
    throw std::runtime_error("init(): SDL_image could not initialize! "
                             "SDL_image Error: " +
                             std::string(IMG_GetError()));

  // todo Repetition des touches?
}

SDL_Surface *load_image(const std::string& file_path){
  SDL_Surface *image_ptr_ = IMG_Load(file_path.c_str());
  if (!image_ptr_) {
    std::cout << "OOPS! The image " << file_path
              << " could not have been loaded" << std::endl;
    std::cout << "Stopping application" << std::endl;
    SDL_Quit();
    std::exit(EXIT_FAILURE);
  }
  return image_ptr_;
}

// ---------------- animal class impl ----------------
int animal::getRandomSex() {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_int_distribution<int> distr(0, 1);
  return distr(generator);
}

int animal::getRandomSpawn(DIRECTION dir) {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  if (dir == DIRECTION::HORIZONTAL) {
    std::uniform_int_distribution<int> distr(frame_boundary,
                                             frame_width - frame_boundary);
    return distr(generator);
  } else {
    std::uniform_int_distribution<int> distr(frame_boundary,
                                             frame_height - frame_boundary);
    return distr(generator);
  }
}

int animal::getRandomTarget(int bounding, DIRECTION dir) {
  int min, max;
  if (dir == DIRECTION::HORIZONTAL) {
    if (position_.x - bounding <= frame_boundary) {
      min = frame_boundary;
    } else {
      min = position_.x - bounding;
    }

    if (position_.x + bounding >= frame_width - frame_boundary) {
      max = frame_width - frame_boundary;
    } else {
      max = position_.x + bounding;
    }
  } else {
    if (position_.y - bounding <= frame_boundary) {
      min = frame_boundary;
    } else {
      min = position_.y - bounding;
    }

    if (position_.y + bounding >= frame_height - frame_boundary) {
      max = frame_height - frame_boundary;
    } else {
      max = position_.y + bounding;
    }
  }
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_int_distribution<int> distr(min, max);
  return distr(generator);
}

bool animal::isOnTarget() {
  return (targetX - speed <= position_.x && position_.x <= targetX + speed) &&
         (targetY - speed <= position_.y && position_.y <= targetY + speed);
}

bool animal::isOnCouple(const animal& secondeAni) {
  float distance =
      sqrt(pow(secondeAni.position_.x - this->position_.x, 2) +
           pow(secondeAni.position_.y - this->position_.y, 2) * 1.0);
  return distance <= (secondeAni.position_.w / 2);
}

void animal::setSpeed(int newSpeed) { this->speed = newSpeed; }

animal::animal(const std::string& file_path, SDL_Surface* window_surface_ptr) {
  image_ptr_ = load_image(file_path.c_str());
  window_surface_ptr_ = window_surface_ptr;
  position_.x = 0;
  position_.y = 0;
  position_.w = image_ptr_->w;
  position_.h = image_ptr_->h;
  speed = 1;

  targetX = 0, targetY = 0;
};

animal::~animal() { SDL_FreeSurface(image_ptr_); };

void animal::draw() {
  SDL_BlitScaled(image_ptr_, NULL, window_surface_ptr_, &position_);
};

void animal::update() { this->move(); }

// ---------------- sheep class impl ----------------
sheep::sheep(SDL_Surface* window_surface_ptr)
    : animal("./media/sheep.png", window_surface_ptr) {
  this->position_.x = getRandomSpawn(DIRECTION::HORIZONTAL);
  this->position_.y = getRandomSpawn(DIRECTION::VERTICAL);
  this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
  this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);

  this->growingPerPourcent = 100;

  this->isFemal = getRandomSex();
}
void sheep::growingUp() {
  unsigned interval = SDL_GetTicks() - this->birthday;
  if (interval % 4 == 0) {
    this->growingPerPourcent++;
  }
}

void sheep::move() {
  if (position_.x > targetX) {
    position_.x -= speed;
  } else if (position_.x < targetX) {
    position_.x += speed;
  }
  if (position_.y > targetY) {
    position_.y -= speed;
  } else if (position_.y < targetY) {
    position_.y += speed;
  }
  if (isOnTarget()) {
    this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
    this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);
  }
}

void sheep::update() {
  if (this->growingPerPourcent <= 25) {
    this->image_ptr_ = load_image("./media/sheep25.png");
    position_.w = image_ptr_->w;
    position_.h = image_ptr_->h;
    this->growingUp();
  } else if (this->growingPerPourcent <= 50 && this->growingPerPourcent >= 25) {
    this->image_ptr_ = load_image("./media/sheep50.png");
    position_.w = image_ptr_->w;
    position_.h = image_ptr_->h;
    this->growingUp();
  } else if (this->growingPerPourcent <= 75 && this->growingPerPourcent >= 50) {
    this->image_ptr_ = load_image("./media/sheep75.png");
    position_.w = image_ptr_->w;
    position_.h = image_ptr_->h;
    this->growingUp();
  } else {
    this->image_ptr_ = load_image("./media/sheep.png");
    position_.w = image_ptr_->w;
    position_.h = image_ptr_->h;
    if (this->growingPerPourcent < 100) {
      this->growingUp();
    }
  }

  this->move();
}

void sheep::isChild() {
  this->growingPerPourcent = 1;
  this->birthday = SDL_GetTicks();
  this->image_ptr_ = load_image("./media/sheep25.png");
  position_.w = image_ptr_->w;
  position_.h = image_ptr_->h;
}

// ---------------- wolf class impl ----------------

wolf::wolf(SDL_Surface* window_surface_ptr)
    : animal("./media/wolf.png", window_surface_ptr) {
  this->position_.x = getRandomSpawn(DIRECTION::HORIZONTAL);
  this->position_.y = getRandomSpawn(DIRECTION::VERTICAL);

  this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
  this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);

  this->growingPerPourcent = 100;
}

void wolf::move() {
  if (position_.x > targetX) {
    position_.x -= speed;
  } else if (position_.x < targetX) {
    position_.x += speed;
  }
  if (position_.y > targetY) {
    position_.y -= speed;
  } else if (position_.y < targetY) {
    position_.y += speed;
  }
  if ((targetX - speed <= position_.x && position_.x <= targetX + speed) &&
      (targetY - speed <= position_.y && position_.y <= targetY + speed)) {
    this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
    this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);
  }
}

// ---------------- ground class impl ----------------

ground::ground(SDL_Surface* window_surface_ptr) {
  window_surface_ptr_ = window_surface_ptr;
  animals_ = std::vector<std::shared_ptr<animal>>();
}

ground::~ground(){};

void ground::add_animal(std::shared_ptr<animal> newAnimal) {
  animals_.push_back(newAnimal);
}

void ground::appendOffspring(sheep& first, sheep& second) {
  if ((first.isFemal && !second.isFemal) ||
      (!first.isFemal && second.isFemal)) {
    auto child = std::make_shared<sheep>(this->window_surface_ptr_);
    child->isChild();
    child->position_.x = first.position_.x;
    child->position_.y = first.position_.y;
    this->add_animal(child);
    first.growingPerPourcent++;
    second.growingPerPourcent++;
  }
}

void ground::update() {
  unsigned initSize = animals_.size();
  for (int i = 0; i < initSize; i++) {
    animal& ani = *animals_[i];
    ani.update();
    ani.draw();

    for (int y = i + 1; y < initSize; y++) {
      animal& secondeAni = *animals_[y];

      // collision between two sheep (hitbox start on top left and stop on the
      // half of the image, to simulate one is inside other)
      if ((ani.growingPerPourcent == max_growing &&
           secondeAni.growingPerPourcent == max_growing) &&
          (typeid(ani) == typeid(sheep) &&
           typeid(secondeAni) == typeid(sheep)) &&
          ani.isOnCouple(secondeAni)) {
        appendOffspring((sheep&)ani, (sheep&)secondeAni);
      }

      if ((typeid(ani) != typeid(secondeAni)) &&
          (typeid(ani) == typeid(wolf) || typeid(ani) == typeid(sheep)) &&
          (typeid(secondeAni) == typeid(wolf) ||
           typeid(secondeAni) == typeid(sheep))) {
        if (sqrt(pow(secondeAni.position_.x - ani.position_.x, 2) +
                 pow(secondeAni.position_.y - ani.position_.y, 2) * 1.0) <=
            100) {
          if (typeid(ani) == typeid(wolf)) {
            secondeAni.setSpeed(2);
          } else {
            ani.setSpeed(2);
          }
        } else {
          if (typeid(ani) == typeid(wolf)) {
            secondeAni.setSpeed(1);
          } else {
            ani.setSpeed(1);
          }
        }
      }
    }
  }
}

// ---------------- application class impl ----------------

application::application(unsigned n_sheep, unsigned n_wolf) {
  // Create an application window with the following settings:
  window_ptr_ = SDL_CreateWindow("An SDL2 window",        // window title
                                 SDL_WINDOWPOS_UNDEFINED, // initial x position
                                 SDL_WINDOWPOS_UNDEFINED, // initial y position
                                 frame_width,             // width, in pixels
                                 frame_height,            // height, in pixels
                                 SDL_WINDOW_SHOWN         // flags - see below
  );

  window_surface_ptr_ = SDL_GetWindowSurface(window_ptr_);

  ground_ = std::make_unique<ground>(window_surface_ptr_);

  for (size_t i = 0; i < n_sheep; i++) {
    ground_->add_animal(std::make_shared<sheep>(window_surface_ptr_));
  }

  for (size_t i = 0; i < n_wolf; i++)
    ground_->add_animal(std::make_shared<wolf>(window_surface_ptr_));
}

application::~application() {
  // Close and destroy the window
  SDL_DestroyWindow(window_ptr_);
}

int application::loop(unsigned period) {
  SDL_Rect windowsRect = SDL_Rect{0, 0, frame_width, frame_height};

  // Ajout du berger
  SDL_Surface* image_ptr_ = IMG_Load("./media/shepherd.png");

  SDL_Rect position_ = SDL_Rect{100, 100, image_ptr_->w, image_ptr_->h};

  SDL_UpdateWindowSurface(window_ptr_);
  while (period * 1000 >= SDL_GetTicks()) {
    SDL_FillRect(window_surface_ptr_, &windowsRect,
                 SDL_MapRGB(window_surface_ptr_->format, 0, 255, 0));
    SDL_PollEvent(&window_event_);
    SDL_BlitScaled(image_ptr_, NULL, window_surface_ptr_, &position_);
    if (window_event_.type == SDL_QUIT ||
        window_event_.type == SDL_WINDOWEVENT &&
            window_event_.window.event == SDL_WINDOWEVENT_CLOSE)
      break;

    ground_->update();
    // std::cout << std::to_string(position_.x) << std::endl;
    SDL_UpdateWindowSurface(window_ptr_);
    SDL_Delay(frame_time * 1000); // Pause execution for framerate milliseconds
  }
  return 1;
}
namespace {
// Defining a namespace without a name -> Anonymous workspace
// Its purpose is to indicate to the compiler that everything
// inside of it is UNIQUELY used within this source file.

SDL_Surface* load_surface_for(const std::string& path,
                              SDL_Surface* window_surface_ptr) {

  // Helper function to load a png for a specific surface
  // See SDL_ConvertSurface
  return NULL;
}
} // namespace
