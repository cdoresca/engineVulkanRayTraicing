#pragma once

void save_settings(const settings set);
void load_settings(settings& set);
void save_objects(vector<ui_object*> objects);
vector<shared_ptr<ui_object>> load_objects(vector<ui_object*>& objs);
