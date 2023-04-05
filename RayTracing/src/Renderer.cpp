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

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageX.resize(width);
	m_ImageY.resize(height);
	for(uint32_t i = 0; i < width; i++)
		m_ImageX[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageY[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

	//std::thread::hardware_concurrency()

#define MT 1
#if MT
	std::for_each(std::execution::par ,m_ImageY.begin(), m_ImageY.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageX.begin(), m_ImageX.end(),
			[this, y](uint32_t x)
			{
				glm::vec4 color = RayGen(x, y);
				m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

				glm::vec4 AccumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;

				AccumulatedColor = glm::clamp(AccumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(AccumulatedColor);
			});
	});

#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = RayGen(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 AccumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;

			AccumulatedColor = glm::clamp(AccumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(AccumulatedColor);
		}
	}
#endif
	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else{
		m_FrameIndex = 1;
	}
}

//Per pixel function
glm::vec4 Renderer::RayGen(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirection()[x + y * m_FinalImage->GetWidth()];
	
	glm::vec3 color(0.0f);
	float multiplier = 1.0f;
	int bounces = 5;

	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitRecord record = TraceRay(ray);
		if (record.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDirection = glm::normalize(glm::vec3(-1, -1, -1));
		float light = glm::max(glm::dot(record.WNormal, -lightDirection), 0.0f);

		const Sphere& sphere = m_ActiveScene->Spheres[record.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		glm::vec3 sphereColor = material.Albedo * light;
		color += sphereColor * multiplier;

		multiplier *= 0.5f;

		ray.Origin = record.WPosition + record.WNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, record.WNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
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

Renderer::HitRecord Renderer::RayClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitRecord record;
	record.HitDistance = hitDistance;
	record.ObjectIndex = objectIndex;
	
	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

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
