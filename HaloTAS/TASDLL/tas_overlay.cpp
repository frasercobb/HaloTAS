#include "tas_overlay.h"
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <chrono>
#include <algorithm>

using namespace halo::addr;

void tas_overlay::glfwMouseButtonFunc(GLFWwindow* /*w*/, int button, int /*action*/, int /*mods*/) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		focused = true;
	}
}

tas_overlay::tas_overlay()
	: window(nullptr)
{
	
}

tas_overlay::~tas_overlay()
{
	glfwMakeContextCurrent(window);
	glfwDestroyWindow(window);
}

void tas_overlay::init_window() {
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

	auto& gEngine = halo_engine::get();

	window = glfwCreateWindow(1280, 720, "TAS Overlay", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	gl3wInit();

	if (!textRenderer) {
		textRenderer = std::make_unique<render_text>();
	}
	if (!cubeRenderer) {
		cubeRenderer = std::make_unique<render_cube>();;
	}

	glfwSetWindowUserPointer(window, this);
	auto func = [](GLFWwindow* w, int b, int a, int m)
	{
		static_cast<tas_overlay*>(glfwGetWindowUserPointer(w))->glfwMouseButtonFunc(w, b, a, m);
	};

	glfwSetMouseButtonCallback(window, func);
}

void tas_overlay::update_position()
{
	glfwMakeContextCurrent(window);

	auto& gEngine = halo_engine::get();
	RECT haloClientRect = gEngine.window_client_rect();
	RECT haloWindowRect = gEngine.window_window_rect();

	if (!IsRectEmpty(&haloWindowRect) && !IsRectEmpty(&haloClientRect)) {

		int width, height, x, y;
		glfwGetWindowSize(window, &width, &height);
		glfwGetWindowPos(window, &x, &y);

		int targetWidth, targetHeight, targetX, targetY;

		targetX = haloWindowRect.left + 8;
		targetY = haloWindowRect.top + 30;

		targetWidth = haloClientRect.right - haloClientRect.left;
		targetHeight = haloClientRect.bottom - haloClientRect.top;

		if (targetX != x || targetY != y) {
			glfwSetWindowPos(window, targetX, targetY);
		}
		if (targetWidth != width || targetHeight != height) {
			glfwSetWindowSize(window, targetWidth, targetHeight);
		}

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
	}
}



void tas_overlay::render(const tas_overlay_render_options& options)
{
	if (!options.enabled) {
		if (window != nullptr) {
			textRenderer.reset();
			cubeRenderer.reset();
			glfwDestroyWindow(window);
			window = nullptr;
		}
		return;
	}

	if (window == nullptr) {
		init_window();
	}

	glfwMakeContextCurrent(window);
	glfwPollEvents();
	update_position();

	auto& engine = halo_engine::get();
	if (focused) {
		engine.focus();
		focused = false;
	}

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);

	engine_snapshot snapshot = {};
	engine.get_snapshot(snapshot);

	std::unordered_set<uint32_t> objectCategories;
	for (auto& v : snapshot.gameObjects) {
		if (objectCategories.find(v->tag_id) == objectCategories.end()) {
			objectCategories.insert(v->tag_id);
		}
	}

	float horizontalFovRadians = **PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS;
	float verticalFov = horizontalFovRadians * (float)display_h / (float)display_w;
	verticalFov = std::clamp(verticalFov - .03f, 0.1f, glm::pi<float>()); // Have to offset by this to get correct ratio for 16:9, need to look into this further

	if (display_w < 400 || display_h < 400) {
		// Not within bounds
		return;
	}

	// Rendering
	glViewport(0, 0, display_w, display_h);
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 Projection = glm::perspectiveFov(verticalFov, (float)display_w, (float)display_h, 0.5f, options.cullDistance);

	glm::vec3 playerPos = *CAMERA_POSITION;
	glm::vec3 dir(CAMERA_LOOK_VECTOR[0], CAMERA_LOOK_VECTOR[1], CAMERA_LOOK_VECTOR[2]);
	glm::vec3 lookAt = playerPos + dir;

	// Camera matrix
	glm::mat4 View = glm::lookAt(playerPos, lookAt, glm::vec3(0, 0, 1));

	if (options.showPrimitives) {
		for (auto& v : snapshot.gameObjects) {

			glm::vec3 color = glm::vec3(1.0f);
			std::string name = "UNKNOWN";
			glm::vec3 modelPos = glm::vec3(v->unit_x, v->unit_y, v->unit_z);

			if (glm::distance(modelPos, playerPos) > options.cullDistance)
				continue;

			if (halo::KNOWN_TAGS.count(v->tag_id) > 0) {
				color = halo::KNOWN_TAGS.at(v->tag_id).displayColor;
				name = halo::KNOWN_TAGS.at(v->tag_id).displayName;
			}

			cubeRenderer->draw_cube(Projection, View, modelPos, .01f, color);

			std::stringstream ss;
			ss << name << " [" << static_cast<void*>(v) << "]";
			glm::vec3 textPos = glm::vec3(v->unit_x, v->unit_y - .05f, v->unit_z + .05f);
			textRenderer->draw_text(ss.str(), options.uiScale, color, Projection, View, textPos, playerPos);
		}
	}

	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);


}
