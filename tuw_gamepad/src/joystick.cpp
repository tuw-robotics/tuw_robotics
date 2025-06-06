#include "tuw_gamepad/joystick.hpp"
#include <sstream>
#include <iomanip>

using namespace tuw;

Joystick::Joystick(const std::string & device)
: device_(device)
{
}

int Joystick::init(std::string & msg)
{
    // Open the joystick device
  std::stringstream ss;
  bool result = NA;
  joystick_fd = open(device_.c_str(), O_RDONLY);
  if (joystick_fd == -1) {
    result = OPEN_FAILED;
    ss << "Error opening joystick device" << std::endl;
    ss << "Make sure the joystick is connected and you have permissions to read " << device_ <<
      std::endl;
    ss << "You might need to run: sudo usermod -a -G input $USER && newgrp input" << std::endl;
  } else {
    ss << "Opened joystick: " << get_joystick_name() << std::endl;
    ss << "Number of axes: " << get_num_axes() << std::endl;
    ss << "Number of buttons: " << get_num_buttons() << std::endl;
    result = SUCCESS;
  }
  msg = ss.str();
  return result;
}

// Function to get the number of axes
int Joystick::get_num_axes()
{
  char axes;
  if (ioctl(joystick_fd, JSIOCGAXES, &axes) == -1) {
    perror("ioctl (JSIOCGAXES)");
    return 0;
  }
  axes_ = std::vector<double>(axes, 0.0);
  return axes;
}

// Function to get the number of buttons
int Joystick::get_num_buttons()
{
  char buttons;
  if (ioctl(joystick_fd, JSIOCGBUTTONS, &buttons) == -1) {
    perror("ioctl (JSIOCGBUTTONS)");
    return 0;
  }
  buttons_ = std::vector<bool>(buttons, false);
  return buttons;
}

// Function to get the joystick name
std::string Joystick::get_joystick_name()
{
  char name[128];   // Max name length is 128 bytes
  if (ioctl(joystick_fd, JSIOCGNAME(sizeof(name)), name) == -1) {
    perror("ioctl (JSIOCGNAME)");
    return "Unknown Joystick";
  }
  joystick_name_ = std::string(name);
  return joystick_name_;
}

int Joystick::wait_of_event(std::string & msg)
{
  std::stringstream ss;
  bool result = NA;
    // Read a joystick event
  ssize_t bytes_read = read(joystick_fd, &js_e, sizeof(js_event));

  if (bytes_read == -1) {
    ss << "Error reading joystick event " << std::endl;
    result = READING_EVENT_FAILED;
  } else {
    if (bytes_read == sizeof(js_event)) {
      std::lock_guard<std::mutex> lock(mutex_);
            // Process the event
      switch (js_e.type & ~JS_EVENT_INIT) { // Mask out JS_EVENT_INIT to handle initial states
        case JS_EVENT_BUTTON:
          buttons_[(int)js_e.number] = js_e.value;
                // std::cout << "Button " << (int)js_e.number << " "   << (js_e.value ? "pressed" : "released") << std::endl;
          break;
        case JS_EVENT_AXIS:
          axes_[(int)js_e.number] = static_cast<double>(js_e.value) / 32767.0;
                // std::cout << "Axis " << (int)js_e.number << " value: " << js_e.value << std::endl;
                // Axis values typically range from -32767 to 32767
          break;
      }
    }
    result = SUCCESS;
  }
  msg = ss.str();
  return result;
}

std::vector<bool> & Joystick::get_buttons_values(std::vector<bool> & des)
{
  std::lock_guard<std::mutex> lock(mutex_);
  des = buttons_;
  return des;
}
std::vector<bool> Joystick::get_buttons_values()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return buttons_;
}

std::vector<double> & Joystick::get_axes_values(std::vector<double> & des)
{
  std::lock_guard<std::mutex> lock(mutex_);
  des = axes_;
  return des;
}
std::vector<double> Joystick::get_axes_values()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return axes_;
}

std::string Joystick::state()
{
  std::lock_guard<std::mutex> lock(mutex_);
  std::stringstream ss;
  ss << "buttons ";
  for (size_t i = 0; i < buttons_.size(); i++) {
    ss << (i == 0 ? " " : ", ");
    ss << i << ":" << (buttons_[i] ? "\033[31mX" : "0") << "\033[0m";
  }
  ss << "; axes ";
  ss << std::fixed << std::setprecision(4) << std::showpos;
  for (size_t i = 0; i < axes_.size(); i++) {
    ss << (i == 0 ? " " : ", ") << i << ":";
    if(axes_[i] > 0.) {ss << "\033[32m";}
    if(axes_[i] < 0.) {ss << "\033[31m";}
    ss << (axes_[i]) << "\033[0m";
  }
  return ss.str();
}

std::shared_ptr<std::thread> Joystick::start()
{
  running_ = true;
  thread_ = std::make_shared<std::thread>([this]() {
        std::string info;
        while (running_) {
          if(this->wait_of_event(info) != tuw::Joystick::SUCCESS) {
            running_ = false;
          }
        }
    });
  return thread_;
}

void Joystick::stop()
{
  running_ = false;
}
