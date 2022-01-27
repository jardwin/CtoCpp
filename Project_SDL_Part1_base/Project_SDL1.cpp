#include "Project_SDL1.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <math.h>
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
}

SDL_Surface* load_image(const std::string& file_path) {
  SDL_Surface* image_ptr_ = IMG_Load(file_path.c_str());
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

void animal::runAway(const animal& seconde) {
  auto degree = SDL_atan2(position_.y - seconde.position_.y,
                          position_.x - seconde.position_.x);
  targetX = abs(position_.x + 100 * cos(-degree * 180 / PI));
  targetY = abs(position_.y + 100 * sin(-degree * 180 / PI));
  if (targetX < 0) {
    targetX = 0;
  }

  if (targetX > frame_width - position_.w) {
    targetX = frame_width - frame_boundary;
  }

  if (targetY < 0) {
    targetY = 0;
  }

  if (targetY > frame_height - position_.h) {
    targetY = frame_height - frame_boundary;
  }
}

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

void animal::move() {
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

// ---------------- shepherd class impl ----------------

shepherd::shepherd(SDL_Surface* window_surface_ptr) { // Ajout du berger
  image_ptr_ = load_image("./media/shepherd.png");
  window_surface_ptr_ = window_surface_ptr;
  position_.x = 100;
  position_.y = 100;
  position_.w = image_ptr_->w;
  position_.h = image_ptr_->h;
}

shepherd::~shepherd() { SDL_FreeSurface(image_ptr_); }

void shepherd::move(const SDL_Event& event, bool keys[322]) {
  // Gestion des touches
  if (event.type == SDL_KEYDOWN) {
    keys[event.key.keysym.scancode] = true;
  } else if (event.type == SDL_KEYUP) {
    keys[event.key.keysym.scancode] = false;
  }

  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) // W -> Z on AZERTY
  {
    if (position_.y > 0)
      position_.y -= 3;
  }
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    if (position_.y < frame_height - position_.h)
      position_.y += 3;
  }
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) // A -> Q on AZERTY
  {
    if (position_.x > 0)
      position_.x -= 3;
  }
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    if (position_.x < frame_width - position_.w)
      position_.x += 3;
  }

  SDL_BlitScaled(image_ptr_, NULL, window_surface_ptr_, &position_);
}

// ---------------- shepherd_dog class impl ----------------

shepherd_dog::shepherd_dog(SDL_Surface* window_surface_ptr) : 
                           animal("./media/shepherd_dog.png", window_surface_ptr) {
  window_surface_ptr_ = window_surface_ptr;
  position_.x = 131;
  position_.y = 131;
  position_.w = image_ptr_->w;
  position_.h = image_ptr_->h;
}

void shepherd_dog::move(const shepherd& master, double degree) {
  position_.x = abs(master.position_.x +
                    (master.position_.w / 2) * cos(degree * 360 / PI));
  position_.y = abs(master.position_.y +
                    (master.position_.h / 2) * sin(degree * 360 / PI));

  if (position_.x < 0)
    position_.x = 0;
  else if (position_.x > frame_width - position_.w)
    position_.x = frame_width - position_.w;
  if (position_.y < 0)
    position_.y = 0;
  else if (position_.y > frame_height - position_.h)
    position_.y = frame_height - position_.h;
}

// ---------------- ground class impl ----------------

bool test = true;

ground::ground(SDL_Surface* window_surface_ptr) {
  window_surface_ptr_ = window_surface_ptr;
  animals_ = std::vector<std::shared_ptr<animal>>();
}

ground::~ground() { SDL_FreeSurface(window_surface_ptr_); };

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
  
  // we compare the current animal to the next one and see if an event occur
  // this mean EACH animal is compared to ALL the others ONCE
  for (auto aniIT = animals_.begin(); aniIT != animals_.end(); ++aniIT) {
    animal& ani = *aniIT.base()->get();
    ani.update();
    ani.draw();

    // TODO : faire le chien qui chasse les loups..........
    /*if(typeid(ani) == typeid(wolf)){
      std::cout << "wolf" << std::endl;
    }
    if(typeid(ani) == typeid(sheep)){
      std::cout << "sheep" << std::endl;
    }
    if(typeid(ani) == typeid(shepherd_dog)){
      std::cout << "Dog" << std::endl;
    }*/

    for (auto secondeIT = aniIT + 1; secondeIT < animals_.end(); ++secondeIT) {
      animal& secondeAni = *secondeIT.base()->get();

      // collision between two sheep (hitbox start on top left and stop on the
      // half of the image, to simulate one is inside other)
      if ((ani.growingPerPourcent == max_growing &&
           secondeAni.growingPerPourcent == max_growing) &&
          (typeid(ani) == typeid(sheep) &&
           typeid(secondeAni) == typeid(sheep)) &&
          ani.isOnCouple(secondeAni)) {
        appendOffspring((sheep&)ani, (sheep&)secondeAni);
      }

      // handle the wolf-sheep chase : - if a sheep is near to a wolf it run
      // - if the wolf is on a sheep he's eaten
      if ((typeid(ani) == typeid(wolf) && typeid(secondeAni) == typeid(sheep)) ||
          (typeid(ani) == typeid(sheep) || typeid(secondeAni) == typeid(wolf))) {
        if (sqrt(pow(secondeAni.position_.x - ani.position_.x, 2) +
                 pow(secondeAni.position_.y - ani.position_.y, 2) * 1.0) <=
            200) {
          if (typeid(ani) == typeid(wolf)) {
            secondeAni.setSpeed(2);
            secondeAni.runAway(ani);
            if (sqrt(pow(secondeAni.position_.x - ani.position_.x, 2) +
                     pow(secondeAni.position_.y - ani.position_.y, 2) * 1.0) <=
                secondeAni.position_.w / 2) {
              this->animals_.erase(secondeIT);
            }
          } else {
            ani.setSpeed(2);
            ani.runAway(secondeAni);
            if (sqrt(pow(secondeAni.position_.x - ani.position_.x, 2) +
                     pow(secondeAni.position_.y - ani.position_.y, 2) * 1.0) <=
                secondeAni.position_.w / 2) {
              this->animals_.erase(aniIT);
              break;
            }
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

  ground_->add_animal(std::make_shared<shepherd_dog>(window_surface_ptr_));

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
  shepherd player = shepherd(window_surface_ptr_);
  shepherd_dog doggo = shepherd_dog(window_surface_ptr_);

  bool keys[322] = {false};
  SDL_UpdateWindowSurface(window_ptr_);
  // dirty... this is just to have a very tiny number and not make the doggo
  // spin like mad
  double degree = 0.0;

  while (period * 1000 >= SDL_GetTicks()) {
    SDL_FillRect(window_surface_ptr_, &windowsRect,
                 SDL_MapRGB(window_surface_ptr_->format, 0, 255, 0));
    SDL_PollEvent(&window_event_);
    if (window_event_.type == SDL_QUIT ||
        window_event_.type == SDL_WINDOWEVENT &&
            window_event_.window.event == SDL_WINDOWEVENT_CLOSE)
      break;
    degree += 0.0006;
    if (degree >= 360.0) {
      degree = 0.0;
    }
    player.move(window_event_, keys);
    doggo.move(player, degree);
    ground_->update();
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
