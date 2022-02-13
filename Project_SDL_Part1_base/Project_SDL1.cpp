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

  // Initialize SDL TTF
  if (TTF_Init() < 0)
    throw std::runtime_error("init():" + std::string(TTF_GetError()));

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
    TTF_Quit();
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

bool animal::isOnCouple(const std::shared_ptr<animal>& secondeAni) {
  float distance =
      sqrt(pow(secondeAni->position_.x - this->position_.x, 2) +
           pow(secondeAni->position_.y - this->position_.y, 2) * 1.0);
  return distance <= (secondeAni->position_.w / 2);
}

void animal::setSpeed(int newSpeed) { this->speed = newSpeed; }

void animal::runAway(const std::shared_ptr<animal>& seconde) {
  auto degree = SDL_atan2(position_.y - seconde->position_.y,
                          position_.x - seconde->position_.x);
  targetX = abs(position_.x + 100 * cos(-degree * 180 / PI));
  targetY = abs(position_.y + 100 * sin(-degree * 180 / PI));
  if (targetX < 0) {
    targetX = frame_boundary;
  }

  if (targetX > frame_width - position_.w) {
    targetX = frame_width - frame_boundary;
  }

  if (targetY < 0) {
    targetY = frame_boundary;
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

sheep::sheep(SDL_Surface* window_surface_ptr, int x, int y)
    : sheep(window_surface_ptr) {
  position_.x = x;
  position_.y = y;
  isChild();
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
  this->lastEatMs = SDL_GetTicks();
}

void wolf::setTarget(std::shared_ptr<animal>& target) {
  if (speed == 1) {
    targetX = target->position_.x;
    targetY = target->position_.y;
  }
}

bool wolf::hasEat() {
  return SDL_GetTicks() - lastEatMs > seconde_to_wolf_die * 1000;
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

float shepherd_dog::getRandomAngle() {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_real_distribution<> distr(0.0004, 0.0009);
  return distr(generator);
}

shepherd_dog::shepherd_dog(std::shared_ptr<shepherd>& master,
                           SDL_Surface* window_surface_ptr, SDL_Event& event)
    : animal("./media/shepherd_dog.png", window_surface_ptr) {
  window_surface_ptr_ = window_surface_ptr;
  position_.x = 131;
  position_.y = 131;
  position_.w = image_ptr_->w;
  position_.h = image_ptr_->h;
  shepherd_master = master;
  degree = 0.0;
  selected = false;
  inOrder = false;
  mouse_event = &event;
}

void shepherd_dog::move() {

  if (mouse_event->type == SDL_MOUSEBUTTONDOWN) {
    if (selected) {
      targetX = mouse_event->button.x;
      targetY = mouse_event->button.y;
      inOrder = true;
    } else {
      if (mouse_event->button.x >= position_.x &&
          mouse_event->button.x <= position_.x + position_.w &&
          mouse_event->button.y >= position_.y &&
          mouse_event->button.y <= position_.y + position_.h) {
        selected = true;
      }
    }
  }

  if (inOrder) {
    if (isOnTarget()) {
      targetX = shepherd_master->position_.x;
      targetY = shepherd_master->position_.y;
    }
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
    if (position_.x == shepherd_master->position_.x &&
        position_.y == shepherd_master->position_.y) {
      inOrder = false;
      selected = false;
    }
  } else {

    degree += degreeIncrement;
    if (degree >= 360.0) {
      degree = 0.0;
    }

    position_.x =
        abs(shepherd_master->position_.x +
            (shepherd_master->position_.w / 2) * cos(degree * 360 / PI));
    position_.y =
        abs(shepherd_master->position_.y +
            (shepherd_master->position_.h / 2) * sin(degree * 360 / PI));

    if (position_.x < 0)
      position_.x = 0;
    else if (position_.x > frame_width - position_.w)
      position_.x = frame_width - position_.w;
    if (position_.y < 0)
      position_.y = 0;
    else if (position_.y > frame_height - position_.h)
      position_.y = frame_height - position_.h;
  }
};

// ---------------- ground class impl ----------------

ground::ground(SDL_Surface* window_surface_ptr) {
  window_surface_ptr_ = window_surface_ptr;
  animals_ = std::vector<std::shared_ptr<animal>>();
}

ground::~ground() { animals_.clear(); };

unsigned ground::countSheep(unsigned nbSheep) {
  for (auto aniIT = animals_.begin(); aniIT != animals_.end(); ++aniIT) {
    animal& ani = *aniIT.base()->get();
    if (typeid(ani) == typeid(sheep)) {
      nbSheep++;
    }
  }
  return nbSheep;
}

void ground::add_animal(std::shared_ptr<animal> newAnimal) {
  animals_.push_back(std::move(newAnimal));
}

void ground::appendOffspring(const std::shared_ptr<sheep>& first,
                             const std::shared_ptr<sheep>& second) {
  if ((first->isFemal && !second->isFemal) ||
      (!first->isFemal && second->isFemal)) {
    this->add_animal(std::make_shared<sheep>(
        this->window_surface_ptr_, first->position_.x, first->position_.y));
    first->growingPerPourcent++;
    second->growingPerPourcent++;
  }
}

void ground::update() {
  // we compare the current animal to the next one and see if an event occur
  // this mean EACH animal is compared to ALL the others ONCE
  unsigned cmp = 0;
  for (auto aniIT = animals_.begin() + cmp; aniIT != animals_.end(); ++aniIT) {
    std::shared_ptr<animal> ani = *aniIT;
    ani->update();
    ani->draw();

    for (auto secondeIT = aniIT + 1; secondeIT < animals_.end(); ++secondeIT) {
      std::shared_ptr<animal> secondeAni = *secondeIT;

      if (typeid(*ani) == typeid(wolf)) {
        if (((wolf&)*ani).hasEat()) {
          aniIT = this->animals_.erase(aniIT);
        }
        if (typeid(*secondeAni) == typeid(sheep)) {
          if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                   pow(secondeAni->position_.y - ani->position_.y, 2) * 1.0) <=
              300) {
            ((wolf&)*ani).setTarget(secondeAni);
          }
          if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                   pow(secondeAni->position_.y - ani->position_.y, 2) * 1.0) <=
              200) {
            secondeAni->setSpeed(2);
            secondeAni->runAway(*aniIT);
            if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                     pow(secondeAni->position_.y - ani->position_.y, 2) *
                         1.0) <= secondeAni->position_.w / 2) {
              ((wolf&)*ani).lastEatMs = SDL_GetTicks();
              secondeIT = this->animals_.erase(secondeIT);
              break;
            }
          } else {
            secondeAni->setSpeed(1);
          }
        } else if (typeid(*secondeAni) == typeid(shepherd_dog)) {
          if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                   pow(secondeAni->position_.y - ani->position_.y, 2) * 1.0) <=
              200) {
            ani->setSpeed(2);
            ani->runAway(*secondeIT);
          } else {
            ani->setSpeed(1);
          }
        }
      } else if (typeid(*ani) == typeid(sheep)) {
        if (typeid(secondeAni) == typeid(sheep)) {
          if ((ani->growingPerPourcent == max_growing &&
               secondeAni->growingPerPourcent == max_growing) &&
              ani->isOnCouple(*secondeIT)) {
            appendOffspring(std::static_pointer_cast<sheep>(*aniIT),
                            std::static_pointer_cast<sheep>(*secondeIT));
          }
        } else if (typeid(*secondeAni) == typeid(wolf)) {
          if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                   pow(secondeAni->position_.y - ani->position_.y, 2) * 1.0) <=
              300) {
            ((wolf&)*secondeAni).setTarget(ani);
          }
          if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                   pow(secondeAni->position_.y - ani->position_.y, 2) * 1.0) <=
              200) {
            ani->setSpeed(2);
            ani->runAway(*secondeIT);
            if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                     pow(secondeAni->position_.y - ani->position_.y, 2) *
                         1.0) <= secondeAni->position_.w / 2) {
              ((wolf&)*secondeAni).lastEatMs = SDL_GetTicks();
              aniIT = this->animals_.erase(aniIT);
              break;
            }
          } else {
            ani->setSpeed(1);
          }
        }
      } else if (typeid(*ani) == typeid(shepherd_dog)) {
        if (typeid(*secondeAni) == typeid(wolf)) {
          if (sqrt(pow(secondeAni->position_.x - ani->position_.x, 2) +
                   pow(secondeAni->position_.y - ani->position_.y, 2) * 1.0) <=
              200) {
            secondeAni->setSpeed(2);
            secondeAni->runAway(*aniIT);
          } else {
            secondeAni->setSpeed(1);
          }
        }
      }
    }
    cmp++;
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
  font = TTF_OpenFont("media/NotoMono-Regular.ttf", 25);
  if (font == nullptr) {
    throw std::runtime_error("could not load the font :" +
                             std::string(TTF_GetError()));
  }

  ground_ = std::make_unique<ground>(window_surface_ptr_);

  for (size_t i = 0; i < n_sheep; i++) {
    ground_->add_animal(std::make_shared<sheep>(window_surface_ptr_));
  }

  for (size_t i = 0; i < n_wolf; i++)
    ground_->add_animal(std::make_shared<wolf>(window_surface_ptr_));
}

application::~application() {
  // Close and destroy the window
  SDL_FreeSurface(score_surface_ptr_);
  SDL_DestroyWindow(window_ptr_);
}

int application::loop(unsigned period) {
  SDL_Rect windowsRect = SDL_Rect{0, 0, frame_width, frame_height};
  std::shared_ptr<shepherd> player =
      std::make_shared<shepherd>(window_surface_ptr_);
  for (int i = 0; i < number_of_dogs; i++) {
    ground_->add_animal(std::make_shared<shepherd_dog>(
        player, window_surface_ptr_, window_event_));
  }

  bool keys[322] = {false};
  SDL_UpdateWindowSurface(window_ptr_);
  unsigned nbSheep = 0;
  unsigned secondsPassed = 1;
  float score = 0;
  SDL_Color color = {0, 0, 0};

  while (period * 1000 >= SDL_GetTicks()) {
    score = nbSheep / secondsPassed;
    score_surface_ptr_ = TTF_RenderText_Solid(
        font, std::to_string((float)(nbSheep / secondsPassed)).c_str(), color);
    score_position_ =
        SDL_Rect{5, 5, score_surface_ptr_->w, score_surface_ptr_->h};

    SDL_FillRect(window_surface_ptr_, &windowsRect,
                 SDL_MapRGB(window_surface_ptr_->format, 0, 255, 0));
    SDL_PollEvent(&window_event_);
    if (window_event_.type == SDL_QUIT ||
        window_event_.type == SDL_WINDOWEVENT &&
            window_event_.window.event == SDL_WINDOWEVENT_CLOSE)
      break;
    player->move(window_event_, keys);
    ground_->update();
    SDL_BlitScaled(score_surface_ptr_, NULL, window_surface_ptr_,
                   &score_position_);
    SDL_UpdateWindowSurface(window_ptr_);
    SDL_Delay(frame_time * 1000); // Pause execution for framerate milliseconds
    // We use SDL_GetTicks() to know if one seconde has passed
    if (SDL_GetTicks() / 1000 == secondsPassed + 1) {
      secondsPassed++;
      nbSheep = ground_->countSheep(nbSheep);
    }
  }
  TTF_CloseFont(font);
  std::cout << "Your score is : " << std::to_string(score) << std::endl;
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
