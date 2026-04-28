#pragma once

#include "instance.h"

/**
 * @brief 
 * La class surface gère le VkSurface.
 * 
 */
class surface {

	GLFWwindow* window;
	VkSurfaceKHR m_surface;
	instance* instance_ptr;

	void createSurface();
	void cleanup();
	void moveFrom(surface&);
	

	public:
		surface(instance*, GLFWwindow*);
		~surface();

		surface(const surface&) = delete;
		surface& operator=(const surface&) = delete;

		surface(surface&&) noexcept;
		surface& operator=(surface&&) noexcept;

		const VkSurfaceKHR get() const;
};