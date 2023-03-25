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
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f;
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
	//rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where a is ray origin, b is ray direction, r is sphere radius and t is hit distance;

	// float a = (bx^2 + by^2)t^2
	// float b = (2(axbx + ayby))t
	// float c = (ax^2 + ay^2 - r^2)

	float a = glm::dot(rayDirection, rayDirection); // basically x^2 + y^2 + z^2
	float b = 2.0f * glm::dot(rayOrigin, rayDirection); // 2 * (axbx + ayby)
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius; // rayOrigin dot itself * - radius^2

	// Quadratic Formula
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant >= 0.0f)
		return 0xFF0000FF;

	return 0xFF000000;

}
