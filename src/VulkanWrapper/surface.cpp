#include "surface.h"

surface::surface(instance* obj, GLFWwindow* w):instance_ptr(obj),window(w) {
	createSurface();
}

surface::~surface() {
	cleanup();
}

void surface::createSurface() {

	if (glfwCreateWindowSurface(instance_ptr->get(), window, nullptr, &m_surface) != VK_SUCCESS)
		throw runtime_error("failed to create window surface");
}

void surface::cleanup() {
	vkDestroySurfaceKHR(instance_ptr->get(), m_surface, nullptr);
	window = nullptr;
	instance_ptr = nullptr;
}

surface::surface(surface&& other) noexcept {
	moveFrom(other);
}
surface& surface::operator=(surface&& other) noexcept {
	if (this != &other) {
		cleanup();
		moveFrom(other);
	}
	return *this;
}


void surface::moveFrom(surface& other) {
	window = other.window;
	m_surface = other.m_surface;
	instance_ptr = other.instance_ptr;
	other.m_surface = VK_NULL_HANDLE;
	other.cleanup();
}

const VkSurfaceKHR surface::get() const { return m_surface; }