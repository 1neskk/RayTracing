#pragma once

#include "Walnut/Image.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>
#include <imgui.h>

class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
	struct HitRecord
	{
		glm::vec3 WPosition;
		glm::vec3 WNormal;
		float HitDistance;
		int ObjectIndex;
	};

	HitRecord TraceRay(const Ray& ray);
	HitRecord RayClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitRecord RayMiss(const Ray& ray);
	glm::vec4 RayGen(uint32_t x, uint32_t y);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
};

namespace Utils {

	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

}
