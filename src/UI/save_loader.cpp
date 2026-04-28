#include <fstream>
#include "json.hpp"
using json = nlohmann::json;
#include "world.h"
#include "object.h"
#include "save_loader.h"

void save_settings(const settings set) {
	json j = {
		{"aspectRatio", set.aspectRatio},
		{"imgWidth", set.imgWidth},
		{"samplesPerPixel", set.samplesPerPixel},
		{"maxDepth", set.maxDepth},
		{"vfov", set.vfov},
		{"defocusAngle", set.defocusAngle},
		{"position", {set.position[0], set.position[1], set.position[2]}},
		{"lookat", {set.lookat[0], set.lookat[1], set.lookat[2]}},
		{"vup", {set.vup[0], set.vup[1], set.vup[2]}},
	};

	std::ofstream o("save.json");
	o << std::setw(4) << j << std::endl;
}

void load_settings(settings& set) {
	std::ifstream f("save.json");
	json data = json::parse(f);

	set.aspectRatio = data["aspectRatio"];
	set.imgWidth = data["imgWidth"];
	set.samplesPerPixel = data["samplesPerPixel"];
	set.maxDepth = data["maxDepth"];
	set.vfov = data["vfov"];
	set.defocusAngle = data["defocusAngle"];
	for (int i = 0; i < 3; i++) {
		set.position[i] = data["position"][i];
		set.lookat[i] = data["lookat"][i];
		set.vup[i] = data["vup"][i];
	}
}

void save_objects(vector<ui_object*> objects){
	string file;

	for (auto obj : objects)
	{
		if (obj->type != "NOT_OBJ") {
			file.append("o ");
			file.append(obj->name);
			file.append("\n");
			file.append(obj->type);
			file.append("\n");
			file.append(obj->save());
		}
	}

	std::ofstream o("save.obj");
	o << std::setw(4) << file << std::endl;
}

vector<shared_ptr<ui_object>> load_objects(vector<ui_object*>& objs) { //TODO clean
	vector<shared_ptr<ui_object>> objects;
	std::ifstream f("save.obj");
	std::string line;
	string name;
	int lines_to_check = 0;
	int object_type;
	ObjectInsertSet insert;
	bool toInsert = true;

	for (auto obj : objs) {
		objects.push_back(obj->clone());
	}

	if (f.is_open()) {
		while (std::getline(f, line)) {
			if (line.starts_with("o ")) {
				name = line.substr(2);
			}

			if (lines_to_check != 0) {
				lines_to_check--;
				std::stringstream stream(line);
				std::string number;
				std::vector<std::string> numbers;
				while (stream >> number) {
					numbers.push_back(number);
				}

				switch (lines_to_check)
				{
				case 3:
					for (int i = 0; i < 3; i++) {
						insert.position[i] = std::stof(numbers[i]);
					}
					break;
				case 2:
					for (int i = 0; i < 3; i++) {
						insert.scale[i] = std::stof(numbers[i]);
					}
					break;
				case 1:
					for (int i = 0; i < 3; i++) {
						insert.rotation[i] = std::stof(numbers[i]);
					}
					break;
				case 0:
					insert.color = { std::stof(numbers[0]), std::stof(numbers[0]), std::stof(numbers[0]) };

					for (auto obj : objs) {
						if (obj->name == name) {
							toInsert = false; //TODO Change to make it so it just goes over every previous thing 
						}
					}

					if(toInsert){
						switch (object_type)
						{
						case 0:
							objects.push_back(make_shared<ui_cube>(insert, name));
							break;
						case 1:
							objects.push_back(make_shared<ui_sphere>(insert, name));
							break;
						case 2:
							objects.push_back(make_shared<ui_plane>(insert, name));
							break;
						}
					}

					break;
				}
			}

			if (line.starts_with("i ")) {
				if (line.substr(2) == "cube") {
					object_type = 0;
					lines_to_check = 4;
				}
				if (line.substr(2) == "sphere") {
					object_type = 1;
					lines_to_check = 4;
				}
				if (line.substr(2) == "plane") {
					object_type = 2;
					lines_to_check = 4;
				}
				if (line.substr(2) == "obj") {
					//TODO
				}
			}
		}
		f.close();
	}
	else {
		std::cerr << "Error: Could not open the file." << std::endl;
	}
	return objects;
}