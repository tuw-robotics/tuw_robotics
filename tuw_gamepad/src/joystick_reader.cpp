#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>      // For open()
#include <unistd.h>     // For read(), close()
#include <linux/joystick.h> // For js_event struct and constants
#include <sys/ioctl.h>  // For ioctl()

// Define the joystick device path
#define JOYSTICK_DEVICE "/dev/input/js0"

// Function to get the number of axes
int get_num_axes(int fd)
{
  char axes;
  if (ioctl(fd, JSIOCGAXES, &axes) == -1) {
    perror("ioctl (JSIOCGAXES)");
    return 0;
  }
  return axes;
}

// Function to get the number of buttons
int get_num_buttons(int fd)
{
  char buttons;
  if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1) {
    perror("ioctl (JSIOCGBUTTONS)");
    return 0;
  }
  return buttons;
}

// Function to get the joystick name
std::string get_joystick_name(int fd)
{
  char name[128];   // Max name length is 128 bytes
  if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) == -1) {
    perror("ioctl (JSIOCGNAME)");
    return "Unknown Joystick";
  }
  return std::string(name);
}

int main()
{
  int joystick_fd;
  struct js_event js_e;

    // Open the joystick device
  joystick_fd = open(JOYSTICK_DEVICE, O_RDONLY);
  if (joystick_fd == -1) {
    perror("Error opening joystick device");
    std::cerr << "Make sure the joystick is connected and you have permissions to read " <<
      JOYSTICK_DEVICE << std::endl;
    std::cerr << "You might need to run: sudo usermod -a -G input $USER && newgrp input" <<
      std::endl;
    return 1;
  }

    // Optionally set non-blocking mode (commented out for this blocking example)
    // fcntl(joystick_fd, F_SETFL, O_NONBLOCK);

  std::cout << "Opened joystick: " << get_joystick_name(joystick_fd) << std::endl;
  std::cout << "Number of axes: " << get_num_axes(joystick_fd) << std::endl;
  std::cout << "Number of buttons: " << get_num_buttons(joystick_fd) << std::endl;
  std::cout << "Listening for joystick events... Press Ctrl+C to exit." << std::endl;

  while (true) {
        // Read a joystick event
    ssize_t bytes_read = read(joystick_fd, &js_e, sizeof(js_event));

    if (bytes_read == -1) {
      perror("Error reading joystick event");
      break;
    }

    if (bytes_read == sizeof(js_event)) {
            // Process the event
      switch (js_e.type & ~JS_EVENT_INIT) {       // Mask out JS_EVENT_INIT to handle initial states
        case JS_EVENT_BUTTON:
          std::cout           << "Button " << (int)js_e.number << " "
                              << (js_e.value ? "pressed" : "released") << std::endl;
          break;
        case JS_EVENT_AXIS:
          std::cout << "Axis " << (int)js_e.number << " value: " << js_e.value << std::endl;
                    // Axis values typically range from -32767 to 32767
          break;
      }
    }
  }

    // Close the joystick device
  close(joystick_fd);
  std::cout << "Joystick device closed." << std::endl;

  return 0;
}
