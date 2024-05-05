#include <stdexcept>
#include <string>

class HardwareInitializationException : public std::runtime_error {
public:
  HardwareInitializationException(const std::string &message)
      : std::runtime_error(message) {}
};
