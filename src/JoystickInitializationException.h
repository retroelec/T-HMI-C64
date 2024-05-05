#include <stdexcept>
#include <string>

class JoystickInitializationException : public std::runtime_error {
public:
  JoystickInitializationException(const std::string &message)
      : std::runtime_error(message) {}
};
