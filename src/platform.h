#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>
#include <sys/time.h>

float get_timestamp();

using namespace std;

string get_folder_from_filename(string filename);

#endif
