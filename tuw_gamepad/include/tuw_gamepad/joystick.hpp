#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>          // For open()
#include <unistd.h>         // For read(), close()
#include <linux/joystick.h> // For js_event struct and constants
#include <sys/ioctl.h>      // For ioctl()
#include <vector>
#include <mutex>
#include <thread>
#include <memory>

// Define the joystick device path
#define JOYSTICK_DEVICE "/dev/input/js0"

namespace tuw
{
class Joystick
{
public:
  static const int SUCCESS = 0;
  static const int NA = -1;
  static const int OPEN_FAILED = 1;
  static const int READING_EVENT_FAILED = 2;
        /**
         * Constructor
         * @param device device to use as joystick
         */
  Joystick(const std::string & device = "/dev/input/js0");
        /**
         * Attempt to open the device
         * @param msg information on the open attempt
         * @return zero on success
         */
  int init(std::string & msg);

        /**
         * blocking function reading device
         * @param msg information
         * @return zero on success
         */
  int wait_of_event(std::string & msg);

        /**
         * non blocking function reading device
         * results can be found in the using
         * @see state
         * @see get_buttons_values
         * @see get_axes_values
         * @return zero on success
         */
  std::shared_ptr<std::thread> start();

        /**
         * stops the non blocking function
         */
  void stop();

        /**
         * returns readable string to the current state
         * @return zero on success
         */
  std::string state();

        /**
         * returns the current button values
         * @param des destination vector
         * @return a ref to the destination vector
         */
  std::vector<bool> & get_buttons_values(std::vector<bool> & des);

        /**
         * returns the current button values
         * @return button values
         */
  std::vector<bool> get_buttons_values();

        /**
         * returns the current axes values
         * @param des destination vector
         * @return a ref to the destination vector
         */
  std::vector<double> & get_axes_values(std::vector<double> & des);

        /**
         * returns the current axes values
         * @return axes values
         */
  std::vector<double> get_axes_values();

private:
  std::mutex mutex_;
  std::string device_;
  int joystick_fd;
  struct js_event js_e;
  std::vector<bool> buttons_;
  std::vector<double> axes_;
  bool running_;

  std::string joystick_name_;
  std::string get_joystick_name();
  int get_num_buttons();
  int get_num_axes();
  std::shared_ptr<std::thread> thread_;
};
}
