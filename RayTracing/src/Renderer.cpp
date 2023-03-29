#include "Renderer.h"
#include "Walnut/Random.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
		{
			return;
		}
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			RayGen(x, y);

			glm::vec4 color = RayGen(x, y);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::RayGen(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirection()[x + y * m_FinalImage->GetWidth()];
	
	glm::vec3 color(0.0f);
	float multiplier = 1.0f;
	int bounces = 2;

	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitRecord record = TraceRay(ray);
		if (record.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.0f, 0.0f, 0.0f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDirection = glm::normalize(glm::vec3(-1, -1, -1));
		float light = glm::max(glm::dot(record.WNormal, -lightDirection), 0.0f);

		const Sphere& sphere = m_ActiveScene->Spheres[record.MaterialIndex];

		glm::vec3 sphereColor = sphere.Color;
		sphereColor *= light;
		color += sphereColor * multiplier;

		multiplier *= 0.7f;

		ray.Origin = record.WPosition + record.WNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, record.WNormal);
	}

	return glm::vec4(color, 1.0f);
}

Renderer::HitRecord Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;


		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f)
			continue;

		float t1 = (-b + sqrt(discriminant)) / (2.0f * a);
		float t2 = (-b - sqrt(discriminant)) / (2.0f * a);
		if (t2 > 0.0f && t2 < hitDistance)
		{
			hitDistance = t2;
			closestSphere = (int)i;
		}

	}
	if (closestSphere < 0)
		return RayMiss(ray);

	return RayClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitRecord Renderer::RayClosestHit(const Ray& ray, float hitDistance, int materialIndex)
{
	Renderer::HitRecord record;
	record.HitDistance = hitDistance;
	record.MaterialIndex = materialIndex;
	
	const Sphere& closestSphere = m_ActiveScene->Spheres[materialIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	record.WPosition = origin + ray.Direction * hitDistance;
	record.WNormal = glm::normalize(record.WPosition);

	record.WPosition += closestSphere.Position;

	return record;
}

Renderer::HitRecord Renderer::RayMiss(const Ray& ray)
{
	Renderer::HitRecord record;
	record.HitDistance = -1.0f;
	return record;
}
