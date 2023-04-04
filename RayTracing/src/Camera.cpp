#include "Camera.h"

#include <glm/gtc/matrix_transform.inl>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Walnut/Input/Input.h"

Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: m_VerticalFov(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip)
{
	m_Direction = glm::vec3(0, 0, -1);
	m_Position = glm::vec3(0, 0, 6);
}

bool Camera::OnUpdate(float ts)
{
	glm::vec2 mousePos = Walnut::Input::GetMousePosition();
	glm::vec2 mouseDelta = (mousePos - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePos;

	if (!Walnut::Input::IsMouseButtonDown(Walnut::MouseButton::Right))
	{
		Walnut::Input::SetCursorMode(Walnut::CursorMode::Normal);
		return false;
	}
	
	Walnut::Input::SetCursorMode(Walnut::CursorMode::Locked);

	bool moved = false;

	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(m_Direction, upDirection);

	float speed = 5.0f;

	if (Walnut::Input::IsKeyDown(Walnut::Key::W))
	{
		m_Position += m_Direction * speed * ts;
		moved = true;
	}
	else if (Walnut::Input::IsKeyDown(Walnut::Key::S))
	{
		m_Position -= m_Direction * speed * ts;
		moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::Key::A))
	{
		m_Position -= rightDirection * speed * ts;
		moved = true;
	}
	else if (Walnut::Input::IsKeyDown(Walnut::Key::D))
	{
		m_Position += rightDirection * speed * ts;
		moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::Key::Q))
	{
		m_Position -= upDirection * speed * ts;
		moved = true;
	}
	else if (Walnut::Input::IsKeyDown(Walnut::Key::E))
	{
		m_Position += upDirection * speed * ts;
		moved = true;
	}

	if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
	{
		float pitch = mouseDelta.y * GetRotationSpeed();
		float yaw = mouseDelta.x * GetRotationSpeed();

		glm::quat quaternion = glm::normalize(glm::cross(glm::angleAxis(-pitch, rightDirection), glm::angleAxis(-yaw, glm::vec3(0.0f, 1.0f, 0.0f))));
		m_Direction = glm::rotate(quaternion, m_Direction);

		moved = true;
	}
	if (moved)
	{
		RecalculateView();
		RecalculateRayDirection();
	}
	return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height)
{
	if(width == m_ViewportWidth && height == m_ViewportHeight)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	RecalculateProjection();
	RecalculateRayDirection();
}

float Camera::GetRotationSpeed()
{
	return 0.3f;
}

void Camera::RecalculateProjection()
{
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFov), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
	m_View = glm::lookAt(m_Position, m_Position + m_Direction, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}

void Camera::RecalculateRayDirection()
{
	m_RayDirection.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++)
	{
		for (uint32_t x = 0; x < m_ViewportWidth; x++)
		{
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight };
			coord = coord * 2.0f - 1.0f;

			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0.0f));
			m_RayDirection[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}
