#pragma once

#include <vector>

class png_saver {
public:
	static bool save_image(int width, int height, std::vector<std::vector<color>> buffers, char* name);
};