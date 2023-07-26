#include "renderer.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <cmath>

int main()
{
	Renderer renderer{};

	renderer.init();

	renderer.loadModel("assets/player.glb");
	renderer.loadModel("assets/cubeW.glb");
	renderer.finalizeModels();

	renderer.meshInstances.push_back({
			.mesh{ 0 },
			.transform{ glm::mat4{ 1.0f } },
		});

	renderer.meshInstances.push_back({
			.mesh{ 1 },
			.transform{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, -3.0f }) },
		});

	glm::mat4 proj{ glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.01f, 1000.0f) };

	glm::mat4 view{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, -2.0f }) };

	glm::vec3 playerPos{ 0.0f, 0.0f, 0.0f };

	float yaw{ -90.0f };
	float pitch{ 0.0f };

	float frame{};

	float deltaTime{ 0.0f };
	float lastTime{ 0.0f };

	while (!renderer.windowShouldClose())
	{
		float currentTime{ static_cast<float>(glfwGetTime()) };
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		float lookSensitivity{ 70.0f };
		if (glfwGetKey(renderer.window(), GLFW_KEY_UP)) pitch -= lookSensitivity * deltaTime;
		if (glfwGetKey(renderer.window(), GLFW_KEY_DOWN)) pitch += lookSensitivity * deltaTime;
		if (glfwGetKey(renderer.window(), GLFW_KEY_LEFT)) yaw += lookSensitivity * deltaTime;
		if (glfwGetKey(renderer.window(), GLFW_KEY_RIGHT)) yaw -= lookSensitivity * deltaTime;

		glm::vec3 camPos
		{
			playerPos.x + 10.0f * (std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch))),
			playerPos.y + 10.0f * std::sin(glm::radians(pitch)),
			playerPos.z - 10.0f * (std::sin(glm::radians(yaw))* std::cos(glm::radians(pitch)))
		};

		view = glm::lookAt(camPos, playerPos, { 0.0f, 1.0f, 0.0f });

		float moveSpeed{ 4.0f };
		if (glfwGetKey(renderer.window(), GLFW_KEY_W))
		{
			playerPos.x -= std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
			playerPos.z += std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
		}
		if (glfwGetKey(renderer.window(), GLFW_KEY_S))
		{
			playerPos.x += std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
			playerPos.z -= std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
		}
		if (glfwGetKey(renderer.window(), GLFW_KEY_A))
		{
			playerPos.z += std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
			playerPos.x += std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
		}
		if (glfwGetKey(renderer.window(), GLFW_KEY_D))
		{
			playerPos.z -= std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
			playerPos.x -= std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
		}

		renderer.meshInstances[0].transform = glm::translate(glm::mat4{ 1.0f }, playerPos);
		renderer.render(proj * view);

		++frame;
	}

	renderer.cleanup();

	return 0;
}