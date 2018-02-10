#include "platform.h"

float get_timestamp(){
	struct timeval tv;

	gettimeofday(&tv, NULL);

	int ms = (tv.tv_usec) / 1000;
	
	return float(ms) / 1000.0;
}

string get_folder_from_filename(string filename){
	// Too lazy to use stl
	int size = filename.size();
	int end = 0;
	for(int i = size - 1; i > 1 && end == 0; i--){
		if(filename[i + 1] == '/'){
			end = i + 1;
		}
	}
	return filename.substr(0,end);
}
