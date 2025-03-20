#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

int main() {
  // Create a basic file logger
  auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/run.log");
  spdlog::set_default_logger(file_logger);

  // Log messages with different severity levels
  spdlog::info("Starting application...");
  spdlog::warn("This is a warning message");
  spdlog::error("An error occurred");

  return 0;
}
