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

void Renderer::Render()
{
	float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f;
			coord.x *= aspectRatio;
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;
	
	// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0
	// where a is ray origin, b is ray direction, r is sphere radius and t is hit distance;

	// float a = (bx^2 + by^2)
	// float b = (2(axbx + ayby))
	// float c = (ax^2 + ay^2 - r^2)
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// Quadratic Formula
	//(-b +- sqrt(discriminant)) / 2 * a
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant >= 0.0f)
	{
		float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
		float t2 = (-b + sqrt(discriminant)) / (2.0f * a);

		{
			glm::vec3 hitPoint1 = rayOrigin + rayDirection * t1;
		}
		{
			glm::vec3 hitPoint2 = rayOrigin + rayDirection * t2;
		}

		return 0xFF0000FF;
	}

	return 0xFF000000;

}
