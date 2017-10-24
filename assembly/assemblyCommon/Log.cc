#include <Log.h>

#include <iostream>
#include <stdexcept>

void Log::KILL(const std::string& log_){

  throw std::runtime_error(log_);

  return;
}

void Log::WARNING(const std::string& log_){

  std::cerr << "@@@ WARNING -- "+log_+"\n";

  return;
}