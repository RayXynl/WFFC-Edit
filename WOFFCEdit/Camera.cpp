#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera()
{
	//functional
	m_movespeed = 0.15;
	m_camRotRate = 3.0;

	//camera
	m_camPosition.x = 0.0f;
	m_camPosition.y = 3.7f;
	m_camPosition.z = -3.5f;

	m_camOrientation.x = 0;
	m_camOrientation.y = 0;
	m_camOrientation.z = 0;

	m_camLookAt.x = 0.0f;
	m_camLookAt.y = 0.0f;
	m_camLookAt.z = 0.0f;

	m_camLookDirection.x = 0.0f;
	m_camLookDirection.y = 0.0f;
	m_camLookDirection.z = 0.0f;

	m_camRight.x = 0.0f;
	m_camRight.y = 0.0f;
	m_camRight.z = 0.0f;

	m_camUp.x = 0.0f;
	m_camUp.y = 0.0f;
	m_camUp.z = 0.0f;

	m_camOrientation.x = 0.0f;
	m_camOrientation.y = 0.0f;
	m_camOrientation.z = 0.0f;
}

Camera::~Camera()
{
}

void Camera::Tick(InputCommands* Input)
{
	m_InputCommands = *Input;
}

void Camera::Update(DX::StepTimer const& timer)
{
	if (m_InputCommands.mouse_Mid_Down)
	{
		GetCursorPos(&m_currMousePos);

		float moveX = (float)(m_currMousePos.x - m_prevMousePos.x);
		float moveY = (float)(m_currMousePos.y - m_prevMousePos.y);

		m_camOrientation.y += moveX * m_camRotRate * m_movespeed;
		m_camOrientation.x -= moveY * m_camRotRate * m_movespeed;


		if (m_camOrientation.x > 89.f)			m_camOrientation.x = 89.f;
		else if (m_camOrientation.x < -89.f)	m_camOrientation.x = -89.f;

		m_prevMousePos = m_currMousePos;
	}
	else if (!m_InputCommands.mouse_Mid_Down)
	{
		GetCursorPos(&m_prevMousePos);
	}

	//create look direction from Euler angles in m_camOrientation
	m_camLookDirection.x = 0.5 * cos(m_camOrientation.y * 3.14159265f / 180.0f) * cos(m_camOrientation.x * 3.14159265f / 180.0f);
	m_camLookDirection.y = 0.5 * sin(m_camOrientation.x * 3.14159265f / 180.0f);
	m_camLookDirection.z = 0.5 * sin(m_camOrientation.y * 3.14159265f / 180.0f) * cos(m_camOrientation.x * 3.14159265f / 180.0f);
	m_camLookDirection.Normalize();

	//create right vector from look Direction
	m_camLookDirection.Cross(Vector3::UnitY, m_camRight);
	if (!m_doOnce)
	{
		m_camLookDirection.Cross(Vector3::UnitZ, m_camUp);
		m_doOnce = true;
	}

	//process input and update stuff
	if (m_InputCommands.editMode == CameraMove)
	{
		if (m_InputCommands.forward)	m_camPosition += m_camLookDirection * m_movespeed;
		if (m_InputCommands.back)		m_camPosition -= m_camLookDirection * m_movespeed;
		if (m_InputCommands.right)		m_camPosition += m_camRight * m_movespeed;
		if (m_InputCommands.left)		m_camPosition -= m_camRight * m_movespeed;
		if (m_InputCommands.up)			m_camPosition -= m_camUp * m_movespeed;
		if (m_InputCommands.down)		m_camPosition += m_camUp * m_movespeed;
	}
	//update lookat point
	m_camLookAt = m_camPosition + m_camLookDirection;

	//apply camera vectors
	m_view = Matrix::CreateLookAt(m_camPosition, m_camLookAt, Vector3::UnitY);

}

void Camera::Rotate()
{
}
