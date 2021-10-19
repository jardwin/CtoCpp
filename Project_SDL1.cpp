// SDL_Test.cpp: Definiert den Einstiegspunkt für die Anwendung.
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
}
// ---------------- animal class impl ----------------

int animal::getRandomSpawn(DIRECTION dir) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    if (dir == DIRECTION::HORIZONTAL)
    {
        std::uniform_int_distribution<int>  distr(frame_boundary, frame_width - frame_boundary);
        return distr(generator);
    }
    else
    {
        std::uniform_int_distribution<int>  distr(frame_boundary, frame_height - frame_boundary);
        return distr(generator);
    }
}

int animal::getRandomTarget(int bounding, DIRECTION dir) {
    int min, max;
    if (dir == DIRECTION::HORIZONTAL)
    {
        if (position_.x - bounding <= frame_boundary)
        {
            min = frame_boundary;
        }
        else
        {
            min = position_.x - bounding;
        }

        if (position_.x + bounding >= frame_width - frame_boundary) {
            max = frame_width - frame_boundary;
        }
        else
        {
            max = position_.x + bounding;
        }
    }
    else {
        if (position_.y - bounding <= frame_boundary)
        {
            min = frame_boundary;
        }
        else
        {
            min = position_.y - bounding;
        }

        if (position_.y + bounding >= frame_height - frame_boundary) {
            max = frame_height - frame_boundary;
        }
        else
        {
            max = position_.y + bounding;
        }
    }
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distr(min, max);
    return distr(generator);
}

animal::animal(const std::string& file_path, SDL_Surface* window_surface_ptr) {
    image_ptr_ = IMG_Load(file_path.c_str());
    window_surface_ptr_ = window_surface_ptr;
    position_.x = 0;
    position_.y = 0;
    position_.w = image_ptr_->w;
    position_.h = image_ptr_->h;

    targetX = 0, targetY = 0;
};

animal::~animal() {
    SDL_FreeSurface(image_ptr_);
};

void animal::draw() {
    SDL_BlitScaled(image_ptr_, NULL, window_surface_ptr_, &position_);
};

// ---------------- sheep class impl ----------------
sheep::sheep(SDL_Surface* window_surface_ptr) : animal("./media/sheep.png", window_surface_ptr) {
    this->position_.x = getRandomSpawn(DIRECTION::HORIZONTAL);
    this->position_.y = getRandomSpawn(DIRECTION::VERTICAL);
    this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
    this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);
}

void sheep::move() {
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
        this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
        this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);
    }
}

// ---------------- wolf class impl ----------------

wolf::wolf(SDL_Surface* window_surface_ptr) : animal("./media/wolf.png", window_surface_ptr) {
    this->position_.x = getRandomSpawn(DIRECTION::HORIZONTAL);
    this->position_.y = getRandomSpawn(DIRECTION::VERTICAL);

    this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
    this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);
}

void wolf::move() {
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
        this->targetX = getRandomTarget(100, DIRECTION::HORIZONTAL);
        this->targetY = getRandomTarget(100, DIRECTION::VERTICAL);
    }
}

// ---------------- ground class impl ----------------

ground::ground(SDL_Surface* window_surface_ptr) {
    window_surface_ptr_ = window_surface_ptr;
    animals_ = new std::vector<animal*>();
}

ground::~ground() {
    delete animals_;
};

void ground::add_animal(animal* newAnimal) {
    animals_->push_back(newAnimal);
}

void ground::update() {
    for (animal* ani : *animals_)
    {
        ani->move();
        ani->draw();
    }
}

// ---------------- application class impl ----------------

application::application(unsigned n_sheep, unsigned n_wolf) {
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

application::~application() {
    // Close and destroy the window
    SDL_DestroyWindow(window_ptr_);
    delete ground_;
}

int application::loop(unsigned period) {
    SDL_Rect windowsRect = SDL_Rect{ 0,0,frame_width, frame_height };
    while (period * 1000 >= SDL_GetTicks()) {
        SDL_FillRect(window_surface_ptr_, &windowsRect, SDL_MapRGB(window_surface_ptr_->format, 0, 255, 0));
        SDL_PollEvent(&window_event_);
        if (window_event_.type == SDL_QUIT || window_event_.type == SDL_WINDOWEVENT &&
            window_event_.window.event == SDL_WINDOWEVENT_CLOSE)
            break;
        ground_->update();
        SDL_UpdateWindowSurface(window_ptr_);
        SDL_Delay(frame_time * 1000);  // Pause execution for framerate milliseconds
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
