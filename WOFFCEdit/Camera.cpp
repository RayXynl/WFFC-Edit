#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera() :
	m_orbitMode(false)
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
	if (m_InputCommands.mouse_Mid_Down)											// if middle mouse button is pressed 
	{
		GetCursorPos(&m_currMousePos);											// get current mouse position

		float moveX = (float)(m_currMousePos.x - m_prevMousePos.x);				// calculate mouse movement x 
		float moveY = (float)(m_currMousePos.y - m_prevMousePos.y);				// calculate mouse movement y

		m_camOrientation.y += moveX * m_camRotRate * m_movespeed;				// rotate camera around y axis
		m_camOrientation.x -= moveY * m_camRotRate * m_movespeed;				// rotate camera around x axis

		if (m_camOrientation.x > 89.f)			m_camOrientation.x = 89.f;		// limit camera rotation
		else if (m_camOrientation.x < -89.f)	m_camOrientation.x = -89.f;		

		m_prevMousePos = m_currMousePos;										// update previous mouse position
	}
	else if (!m_InputCommands.mouse_Mid_Down)									// if middle mouse button is not pressed
	{
		GetCursorPos(&m_prevMousePos);											// get current mouse position
	}

	if (m_orbitMode)															// If orbit mode has been enabled
	{
		if (m_InputCommands.editMode == CameraMove)								// if edit mode is cameraMove
		{
			if (m_InputCommands.left)		m_camOrientation.y	-= m_camRotRate * m_movespeed;	// rotate camera around y axis
			if (m_InputCommands.right)		m_camOrientation.y	+= m_camRotRate * m_movespeed;	// rotate camera around y axis
			if (m_InputCommands.up)			m_camOrientation.x	+= m_camRotRate * m_movespeed;	// rotate camera around x axis
			if (m_InputCommands.down)		m_camOrientation.x	-= m_camRotRate * m_movespeed;	// rotate camera around x axis
			if (m_InputCommands.forward)	m_orbitDistance		-= m_movespeed;					// move camera closer to focal point
			if (m_InputCommands.back)		m_orbitDistance		+= m_movespeed;					// move camera away from focal point
		}


		//create look direction from Euler angles in m_camOrientation
		m_camPosition.x = m_orbitFocal.x + m_orbitDistance * cosf(m_camOrientation.x * 3.14159265f / 180.0f) * cosf(m_camOrientation.y * 3.14159265f / 180.0f);
		m_camPosition.y = m_orbitFocal.y + m_orbitDistance * sinf(m_camOrientation.x * 3.14159265f / 180.0f);
		m_camPosition.z = m_orbitFocal.z + m_orbitDistance * cosf(m_camOrientation.x * 3.14159265f / 180.0f) * sinf(m_camOrientation.y * 3.14159265f / 180.0f);

		m_camLookAt = m_orbitFocal;							// set lookat point to focal point
		m_camLookDirection = m_camLookAt - m_camPosition;	// calculate look direction
		m_camLookDirection.Normalize();						// normalise look direction
	}
	else
	{
		//create look direction from Euler angles in m_camOrientation
		m_camLookDirection.x = 0.5 * cos(m_camOrientation.y * 3.14159265f / 180.0f) * cos(m_camOrientation.x * 3.14159265f / 180.0f);
		m_camLookDirection.y = 0.5 * sin(m_camOrientation.x * 3.14159265f / 180.0f);
		m_camLookDirection.z = 0.5 * sin(m_camOrientation.y * 3.14159265f / 180.0f) * cos(m_camOrientation.x * 3.14159265f / 180.0f);
		m_camLookDirection.Normalize();

		//create right vector from look Direction
		m_camLookDirection.Cross(Vector3::UnitY, m_camRight);
		if (!m_doOnce)
		{
			m_camLookDirection.Cross(Vector3::UnitZ, m_camUp);	// create up vector from look direction
			m_doOnce = true;									// set doOnce to true
		}

		//process input and update stuff
		if (m_InputCommands.editMode == CameraMove)
		{
			if (m_InputCommands.forward)	m_camPosition += m_camLookDirection * m_movespeed;	// move camera forward
			if (m_InputCommands.back)		m_camPosition -= m_camLookDirection * m_movespeed;	// move camera backward
			if (m_InputCommands.right)		m_camPosition += m_camRight * m_movespeed;			// move camera right
			if (m_InputCommands.left)		m_camPosition -= m_camRight * m_movespeed;			// move camera left
			if (m_InputCommands.up)			m_camPosition -= m_camUp * m_movespeed;				// move camera up
			if (m_InputCommands.down)		m_camPosition += m_camUp * m_movespeed;				// move camera down
		}
		//update lookat point
		m_camLookAt = m_camPosition + m_camLookDirection;										// set lookat point to camera position + look direction
	}
	//apply camera vectors
	m_view = Matrix::CreateLookAt(m_camPosition, m_camLookAt, Vector3::UnitY);					// create view matrix from camera position and lookat point
}
 
void Camera::Rotate()
{
}

void Camera::FocusOnObject(const DirectX::SimpleMath::Vector3 objectPos, float distance)
{
	if (!m_orbitMode)								// if orbit mode is not enabled
	{
		m_orbitFocal = objectPos;					// set focal point to object position
		m_orbitDistance = distance;					// set orbit distance to distance
		m_orbitMode = true;							// enable orbit mode
	}
	else
	{
		m_orbitMode = false;						// disable orbit mode
		return;										// return
	}

	Vector3 direction = objectPos - m_camPosition;	// calculate direction from camera position to object position
	direction.Normalize();							// normalise direction

	m_camPosition = (direction * distance) - objectPos; // set camera position to direction * distance - object position
	m_camLookAt = objectPos;							// set lookat point to object position

	Vector3 orientation = objectPos - m_camPosition;	// calculate orientation from object position to camera position
	orientation.Normalize();							// normalise orientation

	
	m_camOrientation.x = asin(orientation.y) * 180.0f / 3.14159265f;					// calculate x rotation from orientation
	m_camOrientation.y = atan2(orientation.z, orientation.x) * 180.0f / 3.14159265f;	// calculate y rotation from orientation

	m_camLookDirection = orientation;													// set look direction to orientation				
	m_view = Matrix::CreateLookAt(m_camPosition, m_camLookAt, Vector3::UnitY);			// create view matrix from camera position and lookat point
}
