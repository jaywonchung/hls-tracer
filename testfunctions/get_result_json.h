#ifndef _GET_RESULT_JSON_H_
#define _GET_RESULT_JSON_H_

#include "json.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
using json = nlohmann::json;

json getResultInJson(const int *array, const int size, std::string filename) {
  json result;
  int current_index = array[size-2];
  bool wrapped = array[size-1] ? true : false;

  std::cout << "Recorded trace #: " << (wrapped ? (size - 2) / 2 : (current_index + 1) / 2) << std::endl;

  if (wrapped) {
    for (int i = current_index; i < size-2; i+=2) {
      result.push_back({{"line", array[i]}, {"column", array[i+1]}});
    }
  }

  for (int i = 0; i < current_index; i+=2) {
    result.push_back({{"line", array[i]}, {"column", array[i+1]}});
  }

  // Write json result to file
  char tmp[256];
  getcwd(tmp, 256);
  std::cout << "Saving trace as json to " << tmp << "/" << filename << std::endl;

  std::ofstream o(filename);
  o << std::setw(4) << result << std::endl;

  return result;
}

#endif