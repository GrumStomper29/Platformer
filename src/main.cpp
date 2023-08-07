#define RENDERER_USE_IMGUI
#include "renderer.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>
#include <iostream>

#include "imgui.h"

constexpr float deltaTime{ 1.0f / 60.0f };

struct AABB
{
	glm::vec3 pos{};
	glm::vec3 scl{};
	int meshInstance{ 0 };
};

struct Enemy
{
	glm::vec3 pos{};
	glm::vec3 a{};
	glm::vec3 b{};
	int meshInstance{ -1 };
};

bool AABBvsAABB(const AABB& a, const AABB& b)
{
	return ((a.pos.x + a.scl.x >= b.pos.x - b.scl.x)
		&& (a.pos.x - a.scl.x <= b.pos.x + b.scl.x)
		&& (a.pos.y + a.scl.y >= b.pos.y - b.scl.y)
		&& (a.pos.y - a.scl.y <= b.pos.y + b.scl.y)
		&& (a.pos.z + a.scl.z >= b.pos.z - b.scl.z)
		&& (a.pos.z - a.scl.z <= b.pos.z + b.scl.z));
}

bool AABBvsAABBs(const AABB& aabb, const std::vector<AABB>& aabbs)
{
	for (const auto& b : aabbs)
	{
		if (AABBvsAABB(aabb, b))
		{
			return true;
		}
	}

	return false;
}

int AABBvsEnemies(const AABB& aabb, const std::vector<Enemy>& enemies)
{
	for (std::size_t i{ 0 }; i < enemies.size(); ++i)
	{
		if (AABBvsAABB(aabb, AABB{ enemies[i].pos, glm::vec3{ 1.0f }}))
		{
			return i;
		}
	}

	return -1;
}

void updateEnemies(Renderer& renderer, std::vector<Enemy>& enemies)
{
	float speed{ std::sin(static_cast<float>(glfwGetTime())) + 1.0f };

	for (auto& enemy : enemies)
	{
		enemy.pos = 
		{
			(((enemy.b.x - enemy.a.x) / 2.0f) * speed) + enemy.a.x,
			(((enemy.b.y - enemy.a.y) / 2.0f) * speed) + enemy.a.y,
			(((enemy.b.z - enemy.a.z) / 2.0f) * speed) + enemy.a.z,
		};

		renderer.meshInstances[enemy.meshInstance].transform =
			glm::translate(glm::mat4{ 1.0f }, enemy.pos);
	}
}

void update(Renderer& renderer, float& yaw, float& pitch, glm::vec3& playerPos, const std::vector<AABB>& aabbs, std::vector<Enemy>& enemies)
{
	updateEnemies(renderer, enemies);

	static glm::vec3 playerVel{ 0.0f };
	static float airTime{ 1.0f };

	airTime += deltaTime;

	playerVel.y -= 0.6f * deltaTime;

	constexpr float moveSpeed{ 1.5f };
	if (glfwGetKey(renderer.window(), GLFW_KEY_W))
	{
		playerVel.x -= std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
		playerVel.z += std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
	}
	if (glfwGetKey(renderer.window(), GLFW_KEY_S))
	{
		playerVel.x += std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
		playerVel.z -= std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
	}
	if (glfwGetKey(renderer.window(), GLFW_KEY_A))
	{
		playerVel.z += std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
		playerVel.x += std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
	}
	if (glfwGetKey(renderer.window(), GLFW_KEY_D))
	{
		playerVel.z -= std::cos(glm::radians(yaw)) * moveSpeed * deltaTime;
		playerVel.x -= std::sin(glm::radians(yaw)) * moveSpeed * deltaTime;
	}

	if (glfwGetKey(renderer.window(), GLFW_KEY_SPACE) && airTime < 0.1f)
	{
		playerVel.y = 0.20f;
	}

	constexpr float lookSensitivity{ 100.0f };
	if (glfwGetKey(renderer.window(), GLFW_KEY_UP)) pitch -= lookSensitivity * deltaTime;
	if (glfwGetKey(renderer.window(), GLFW_KEY_DOWN)) pitch += lookSensitivity * deltaTime;
	if (glfwGetKey(renderer.window(), GLFW_KEY_LEFT)) yaw += lookSensitivity * deltaTime;
	if (glfwGetKey(renderer.window(), GLFW_KEY_RIGHT)) yaw -= lookSensitivity * deltaTime;

	playerPos.y += playerVel.y;
	if (AABBvsAABBs({ playerPos, glm::vec3{ 1.0f } }, aabbs))
	{
		while (AABBvsAABBs({ playerPos, glm::vec3{ 1.0f } }, aabbs))
		{
			playerPos.y += 0.001f;
		}
		playerVel.y = 0.0f;
		airTime = 0.0f;
	}

	playerPos.x += playerVel.x;
	if (AABBvsAABBs({ playerPos, glm::vec3{ 1.0f } }, aabbs))
	{
		while (AABBvsAABBs({ playerPos, glm::vec3{ 1.0f } }, aabbs))
		{
			playerPos.x -= playerVel.x / 4.0f;
		}
		playerVel.x = 0.0f;
	}
	playerVel.x *= 0.7f;

	playerPos.z += playerVel.z;
	if (AABBvsAABBs({ playerPos, glm::vec3{ 1.0f } }, aabbs))
	{
		while (AABBvsAABBs({ playerPos, glm::vec3{ 1.0f } }, aabbs))
		{
			playerPos.z -= playerVel.z / 4.0f;
		}
		playerVel.z = 0.0f;
	}
	playerVel.z *= 0.7f;

	if (playerPos.y < -2.0f)
	{
		playerPos = { 0.0f, 6.0f, 2.3f };
		playerVel = glm::vec3{ 0.0f };
		airTime = 1.0f;
	}

	int enemyTouched{ AABBvsEnemies({ playerPos, glm::vec3{ 1.0f } }, enemies) };
	if (enemyTouched != -1)
	{
		if (playerVel.y < 0.0f)
		{
			renderer.meshInstances[enemies[enemyTouched].meshInstance].show = false;
			enemies.erase(enemies.begin() + enemyTouched);
		}
		else
		{
			playerPos = { 0.0f, 6.0f, 2.3f };
			playerVel = glm::vec3{ 0.0f };
			airTime = 1.0f;
		}
	}

	renderer.meshInstances[0].transform = glm::translate(glm::mat4{ 1.0f }, playerPos);
}

void drawGui(bool &showAabbs, int& aabbTarget, std::vector<AABB>& aabbs, Renderer& renderer, 
	const glm::vec3& playerPos, bool& drawShadows)
{
	ImGui::Begin("AABB");

	ImGui::Checkbox("Show", &showAabbs);

	ImGui::InputInt("Target", &aabbTarget);
	if (aabbTarget < 0) aabbTarget = 0;
	if (aabbTarget > aabbs.size() - 1) aabbTarget = aabbs.size() - 1;

	float pos[3]{ aabbs[aabbTarget].pos.x, aabbs[aabbTarget].pos.y, aabbs[aabbTarget].pos.z };
	ImGui::InputFloat3("Position", pos);
	aabbs[aabbTarget].pos = { pos[0], pos[1], pos[2] };

	float scl[3]{ aabbs[aabbTarget].scl.x, aabbs[aabbTarget].scl.y, aabbs[aabbTarget].scl.z };
	ImGui::InputFloat3("Scale", scl);
	aabbs[aabbTarget].scl = { scl[0], scl[1], scl[2] };

	{
		glm::mat4 transform{ glm::translate(glm::mat4{ 1.0f }, aabbs[aabbTarget].pos) };
		transform = glm::scale(transform, aabbs[aabbTarget].scl);
		renderer.meshInstances[aabbs[aabbTarget].meshInstance].transform = transform;
	}

	ImGui::End();

	ImGui::Begin("Global");
	ImGui::Text("Player position");
	ImGui::Text(std::string{ std::to_string(playerPos.x) + ' ' + std::to_string(playerPos.y) + ' ' + std::to_string(playerPos.z) }.c_str());
	ImGui::Checkbox("Draw shadows?", &drawShadows);
	ImGui::End();
}

int main()
{
	Renderer renderer{};

	renderer.init();

	renderer.loadModel("assets/player.glb");
	renderer.loadModel("assets/grass.glb");
	renderer.loadModel("assets/cube.glb");
	renderer.loadModel("assets/flag.glb");
	renderer.loadModel("assets/lvl1.glb");
	renderer.loadModel("assets/enemy.glb");
	renderer.loadModel("assets/sign.glb");
	renderer.finalizeModels();

	std::vector<AABB> aabbs{};
	aabbs.push_back({ { 0.0f, 1.7f, 0.0f }, { 2.0f, 2.0f, 4.2f } });
	aabbs.push_back({ { 0.0f, 1.7f, -12.0f }, { 2.0f, 2.0f, 4.2f } });
	aabbs.push_back({ { -6.3f, 2.3f, -14.5f }, { 4.2f, 3.0f, 1.7f } });
	aabbs.push_back({ { -15.7f, 3.5f, -17.9f }, { 2.0f, 3.5f, 5.0f } });
	aabbs.push_back({ { -8.4f, 3.9f, -23.4f }, { 2.0f, 4.4f, 2.0f } });
	
	std::vector<Enemy> enemies{};
	enemies.push_back({ { 0.0f, 0.0f, 0.0f }, { -15.7f, 8.0f, -15.0f }, { -15.7f, 8.0f, -21.0f } });

	renderer.meshInstances.push_back({
			.mesh{ 0 },
			.transform{ glm::mat4{ 1.0f } },
		});

	renderer.meshInstances.push_back({
			.mesh{ 4 },
			.transform{ glm::mat4{ glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 2.0f }) } },
		});

	renderer.meshInstances.push_back({
			.mesh{ 6 },
			.transform{ glm::mat4{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ -1.6f, 3.6f, 2.4f }) } },
		});

	for (auto& aabb : aabbs)
	{
		aabb.meshInstance = renderer.meshInstances.size();

		glm::mat4 aabbMat{ glm::translate(glm::mat4{ 1.0f }, aabb.pos) };
		aabbMat = glm::scale(aabbMat, aabb.scl);

		renderer.meshInstances.push_back({
			.mesh{ 2 },
			.transform{ aabbMat },
			.pass{ Renderer::AABB }
			});
	}

	for (auto& enemy : enemies)
	{
		enemy.meshInstance = renderer.meshInstances.size();

		renderer.meshInstances.push_back({
			.mesh{ 5 },
			.transform{ glm::mat4{ 1.0f } }
			});
	}

	glm::mat4 proj{ glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.01f, 1000.0f) };

	glm::mat4 view{ glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, -2.0f }) };

	glm::vec3 playerPos{ 0.0f, 6.0f, 2.3f };

	float yaw{ -90.0f };
	float pitch{ 0.0f };

	bool drawAabbs{ false };
	int aabbTarget{ 0 };

	bool drawShadows{ false };

	float accumulator{ 0.0f };
	float lastTime{ static_cast<float>(glfwGetTime()) };
	bool drawn{ false };

	while (!renderer.windowShouldClose())
	{
		float currentTime{ static_cast<float>(glfwGetTime()) };
		accumulator += currentTime - lastTime;
		lastTime = currentTime;

		while (accumulator > ::deltaTime)
		{
			update(renderer, yaw, pitch, playerPos, aabbs, enemies);

			accumulator -= deltaTime;
			drawn = false;
		}

		if (drawn)
		{
			
		}
		else
		{
			glm::vec3 camPos
			{
				playerPos.x + 10.0f * (std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch))),
					playerPos.y + 10.0f * std::sin(glm::radians(pitch)),
					playerPos.z - 10.0f * (std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)))
			};

			view = glm::lookAt(camPos, playerPos, { 0.0f, 1.0f, 0.0f });

			renderer.beginFrame();

			//drawGui(drawAabbs, aabbTarget, aabbs, renderer, playerPos, drawShadows);

			renderer.render(proj * view, drawShadows, drawAabbs);

			drawn = true;
		}
	}

	renderer.cleanup();

	return 0;
}