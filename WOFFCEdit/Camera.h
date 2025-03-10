#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SceneObject.h"
#include "DisplayObject.h"
#include "DisplayChunk.h"
#include "ChunkObject.h"
#include "InputCommands.h"
#include <vector>

class Camera
{
public:
	Camera();
	~Camera();

	void Input();
	void Update();
	void Rotate();


	
};

