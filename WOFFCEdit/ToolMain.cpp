#include "ToolMain.h"
#include "resource.h"
#include <vector>
#include <sstream>
#include <iostream>

//
//ToolMain Class
ToolMain::ToolMain()
{

	m_currentChunk = 0;		//default value
	//m_selectedObject = 0;	//initial selection ID
	m_sceneGraph.clear();	//clear the vector for the scenegraph
	m_databaseConnection = NULL;
}


ToolMain::~ToolMain()
{
	sqlite3_close(m_databaseConnection);		//close the database connection
}


std::vector<int> ToolMain::getCurrentSelectionID()
{
	return m_selectedObject;
}

void ToolMain::onActionInitialise(HWND handle, int width, int height, ObjectManipulationDialog* objectDialogRef, ObjectionCreationDialog* objectCreateRef, InputCommands* toolInputCommands)
{
	//window size, handle etc for directX
	m_width		= width;
	m_height	= height;
	m_ToolObjectManipDialog = objectDialogRef;
	m_ToolObjectCreationDialog = objectCreateRef;
	m_toolInputCommands = toolInputCommands;
	m_d3dRenderer.Initialize(handle, m_width, m_height, &m_displayList, &m_redoStack, &m_undoStack);

	//database connection establish
	int rc;
	rc = sqlite3_open_v2("database/test.db",&m_databaseConnection, SQLITE_OPEN_READWRITE, NULL);

	if (rc) 
	{
		TRACE("Can't open database");
		//if the database cant open. Perhaps a more catastrophic error would be better here
	}
	else 
	{
		TRACE("Opened database successfully");
	}

	//zero input commands
	m_toolInputCommands->forward			= false;
	m_toolInputCommands->back				= false;
	m_toolInputCommands->left				= false;
	m_toolInputCommands->right				= false;
	m_toolInputCommands->mouse_Mid_Down		= false;
	m_toolInputCommands->mouse_LB_Down		= false;

	m_toolInputCommands->g_key_down			= false;

	m_toolInputCommands->undoDown			= false;
	m_toolInputCommands->undoDownPrevState	= false;
	m_toolInputCommands->redoDown			= false;
	m_toolInputCommands->redoDownPrevState	= false;

	m_toolInputCommands->deleteDown			= false;

	m_toolInputCommands->f_key_down			= false;
	m_toolInputCommands->focus_prev			= false;

	m_toolInputCommands->editMode			= CameraMove;

	m_toolInputCommands->mouse_X			= 0;
	m_toolInputCommands->mouse_Y			= 0;

	onActionLoad();
}

void ToolMain::onActionLoad()
{
	//load current chunk and objects into lists
	if (!m_sceneGraph.empty())		//is the vector empty
	{
		m_sceneGraph.clear();		//if not, empty it
	}

	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	sqlite3_stmt *pResultsChunk;

	//OBJECTS IN THE WORLD
	//prepare SQL Text
	sqlCommand = "SELECT * from Objects";				//sql command which will return all records from the objects table.
	//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0 );
	
	//loop for each row in results until there are no more rows.  ie for every row in the results. We create and object
	while (sqlite3_step(pResults) == SQLITE_ROW)
	{	
		SceneObject newSceneObject;
		newSceneObject.ID = sqlite3_column_int(pResults, 0);
		newSceneObject.chunk_ID = sqlite3_column_int(pResults, 1);
		newSceneObject.model_path		= reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		newSceneObject.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		newSceneObject.posX = sqlite3_column_double(pResults, 4);
		newSceneObject.posY = sqlite3_column_double(pResults, 5);
		newSceneObject.posZ = sqlite3_column_double(pResults, 6);
		newSceneObject.rotX = sqlite3_column_double(pResults, 7);
		newSceneObject.rotY = sqlite3_column_double(pResults, 8);
		newSceneObject.rotZ = sqlite3_column_double(pResults, 9);
		newSceneObject.scaX = sqlite3_column_double(pResults, 10);
		newSceneObject.scaY = sqlite3_column_double(pResults, 11);
		newSceneObject.scaZ = sqlite3_column_double(pResults, 12);
		newSceneObject.render = sqlite3_column_int(pResults, 13);
		newSceneObject.collision = sqlite3_column_int(pResults, 14);
		newSceneObject.collision_mesh = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 15));
		newSceneObject.collectable = sqlite3_column_int(pResults, 16);
		newSceneObject.destructable = sqlite3_column_int(pResults, 17);
		newSceneObject.health_amount = sqlite3_column_int(pResults, 18);
		newSceneObject.editor_render = sqlite3_column_int(pResults, 19);
		newSceneObject.editor_texture_vis = sqlite3_column_int(pResults, 20);
		newSceneObject.editor_normals_vis = sqlite3_column_int(pResults, 21);
		newSceneObject.editor_collision_vis = sqlite3_column_int(pResults, 22);
		newSceneObject.editor_pivot_vis = sqlite3_column_int(pResults, 23);
		newSceneObject.pivotX = sqlite3_column_double(pResults, 24);
		newSceneObject.pivotY = sqlite3_column_double(pResults, 25);
		newSceneObject.pivotZ = sqlite3_column_double(pResults, 26);
		newSceneObject.snapToGround = sqlite3_column_int(pResults, 27);
		newSceneObject.AINode = sqlite3_column_int(pResults, 28);
		newSceneObject.audio_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 29));
		newSceneObject.volume = sqlite3_column_double(pResults, 30);
		newSceneObject.pitch = sqlite3_column_double(pResults, 31);
		newSceneObject.pan = sqlite3_column_int(pResults, 32);
		newSceneObject.one_shot = sqlite3_column_int(pResults, 33);
		newSceneObject.play_on_init = sqlite3_column_int(pResults, 34);
		newSceneObject.play_in_editor = sqlite3_column_int(pResults, 35);
		newSceneObject.min_dist = sqlite3_column_double(pResults, 36);
		newSceneObject.max_dist = sqlite3_column_double(pResults, 37);
		newSceneObject.camera = sqlite3_column_int(pResults, 38);
		newSceneObject.path_node = sqlite3_column_int(pResults, 39);
		newSceneObject.path_node_start = sqlite3_column_int(pResults, 40);
		newSceneObject.path_node_end = sqlite3_column_int(pResults, 41);
		newSceneObject.parent_id = sqlite3_column_int(pResults, 42);
		newSceneObject.editor_wireframe = sqlite3_column_int(pResults, 43);
		newSceneObject.name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));

		newSceneObject.light_type = sqlite3_column_int(pResults, 45);
		newSceneObject.light_diffuse_r = sqlite3_column_double(pResults, 46);
		newSceneObject.light_diffuse_g = sqlite3_column_double(pResults, 47);
		newSceneObject.light_diffuse_b = sqlite3_column_double(pResults, 48);
		newSceneObject.light_specular_r = sqlite3_column_double(pResults, 49);
		newSceneObject.light_specular_g = sqlite3_column_double(pResults, 50);
		newSceneObject.light_specular_b = sqlite3_column_double(pResults, 51);
		newSceneObject.light_spot_cutoff = sqlite3_column_double(pResults, 52);
		newSceneObject.light_constant = sqlite3_column_double(pResults, 53);
		newSceneObject.light_linear = sqlite3_column_double(pResults, 54);
		newSceneObject.light_quadratic = sqlite3_column_double(pResults, 55);
	

		//send completed object to scenegraph
		m_sceneGraph.push_back(newSceneObject);
	}

	//THE WORLD CHUNK
	//prepare SQL Text
	sqlCommand = "SELECT * from Chunks";				//sql command which will return all records from  chunks table. There is only one tho.
														//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResultsChunk, 0);


	sqlite3_step(pResultsChunk);
	m_chunk.ID = sqlite3_column_int(pResultsChunk, 0);
	m_chunk.name = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 1));
	m_chunk.chunk_x_size_metres = sqlite3_column_int(pResultsChunk, 2);
	m_chunk.chunk_y_size_metres = sqlite3_column_int(pResultsChunk, 3);
	m_chunk.chunk_base_resolution = sqlite3_column_int(pResultsChunk, 4);
	m_chunk.heightmap_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 5));
	m_chunk.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 6));
	m_chunk.tex_splat_alpha_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 7));
	m_chunk.tex_splat_1_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 8));
	m_chunk.tex_splat_2_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 9));
	m_chunk.tex_splat_3_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 10));
	m_chunk.tex_splat_4_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 11));
	m_chunk.render_wireframe = sqlite3_column_int(pResultsChunk, 12);
	m_chunk.render_normals = sqlite3_column_int(pResultsChunk, 13);
	m_chunk.tex_diffuse_tiling = sqlite3_column_int(pResultsChunk, 14);
	m_chunk.tex_splat_1_tiling = sqlite3_column_int(pResultsChunk, 15);
	m_chunk.tex_splat_2_tiling = sqlite3_column_int(pResultsChunk, 16);
	m_chunk.tex_splat_3_tiling = sqlite3_column_int(pResultsChunk, 17);
	m_chunk.tex_splat_4_tiling = sqlite3_column_int(pResultsChunk, 18);


	//Process REsults into renderable
	m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	//build the renderable chunk 
	m_d3dRenderer.BuildDisplayChunk(&m_chunk);
}

void ToolMain::onActionSave()
{
	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	

	//OBJECTS IN THE WORLD Delete them all
	//prepare SQL Text
	sqlCommand = "DELETE FROM Objects";	 //will delete the whole object table.   Slightly risky but hey.
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);
	sqlite3_step(pResults);

	//Populate with our new objects
	std::wstring sqlCommand2;
	int numObjects = m_sceneGraph.size();	//Loop thru the scengraph.

	for (int i = 0; i < numObjects; i++)
	{
		std::stringstream command;
		command << "INSERT INTO Objects " 
			<<"VALUES(" << m_sceneGraph.at(i).ID << ","
			<< m_sceneGraph.at(i).chunk_ID  << ","
			<< "'" << m_sceneGraph.at(i).model_path <<"'" << ","
			<< "'" << m_sceneGraph.at(i).tex_diffuse_path << "'" << ","
			<< m_sceneGraph.at(i).posX << ","
			<< m_sceneGraph.at(i).posY << ","
			<< m_sceneGraph.at(i).posZ << ","
			<< m_sceneGraph.at(i).rotX << ","
			<< m_sceneGraph.at(i).rotY << ","
			<< m_sceneGraph.at(i).rotZ << ","
			<< m_sceneGraph.at(i).scaX << ","
			<< m_sceneGraph.at(i).scaY << ","
			<< m_sceneGraph.at(i).scaZ << ","
			<< m_sceneGraph.at(i).render << ","
			<< m_sceneGraph.at(i).collision << ","
			<< "'" << m_sceneGraph.at(i).collision_mesh << "'" << ","
			<< m_sceneGraph.at(i).collectable << ","
			<< m_sceneGraph.at(i).destructable << ","
			<< m_sceneGraph.at(i).health_amount << ","
			<< m_sceneGraph.at(i).editor_render << ","
			<< m_sceneGraph.at(i).editor_texture_vis << ","
			<< m_sceneGraph.at(i).editor_normals_vis << ","
			<< m_sceneGraph.at(i).editor_collision_vis << ","
			<< m_sceneGraph.at(i).editor_pivot_vis << ","
			<< m_sceneGraph.at(i).pivotX << ","
			<< m_sceneGraph.at(i).pivotY << ","
			<< m_sceneGraph.at(i).pivotZ << ","
			<< m_sceneGraph.at(i).snapToGround << ","
			<< m_sceneGraph.at(i).AINode << ","
			<< "'" << m_sceneGraph.at(i).audio_path << "'" << ","
			<< m_sceneGraph.at(i).volume << ","
			<< m_sceneGraph.at(i).pitch << ","
			<< m_sceneGraph.at(i).pan << ","
			<< m_sceneGraph.at(i).one_shot << ","
			<< m_sceneGraph.at(i).play_on_init << ","
			<< m_sceneGraph.at(i).play_in_editor << ","
			<< m_sceneGraph.at(i).min_dist << ","
			<< m_sceneGraph.at(i).max_dist << ","
			<< m_sceneGraph.at(i).camera << ","
			<< m_sceneGraph.at(i).path_node << ","
			<< m_sceneGraph.at(i).path_node_start << ","
			<< m_sceneGraph.at(i).path_node_end << ","
			<< m_sceneGraph.at(i).parent_id << ","
			<< m_sceneGraph.at(i).editor_wireframe << ","
			<< "'" << m_sceneGraph.at(i).name << "'" << ","

			<< m_sceneGraph.at(i).light_type << ","
			<< m_sceneGraph.at(i).light_diffuse_r << ","
			<< m_sceneGraph.at(i).light_diffuse_g << ","
			<< m_sceneGraph.at(i).light_diffuse_b << ","
			<< m_sceneGraph.at(i).light_specular_r << ","
			<< m_sceneGraph.at(i).light_specular_g << ","
			<< m_sceneGraph.at(i).light_specular_b << ","
			<< m_sceneGraph.at(i).light_spot_cutoff << ","
			<< m_sceneGraph.at(i).light_constant << ","
			<< m_sceneGraph.at(i).light_linear << ","
			<< m_sceneGraph.at(i).light_quadratic

			<< ")";
		std::string sqlCommand2 = command.str();
		rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand2.c_str(), -1, &pResults, 0);
		sqlite3_step(pResults);	
	}
	MessageBox(NULL, L"Objects Saved", L"Notification", MB_OK);
}

void ToolMain::onActionSaveTerrain()
{
	m_d3dRenderer.SaveDisplayChunk(&m_chunk);
}

void ToolMain::Tick(MSG *msg)
{
	if (CWnd::GetFocus())																		// Get currently focused window
	{
		CString focusedWindowName;																// Create string to hold name of current focused window
		CWnd::GetFocus()->GetWindowText(focusedWindowName);										// Get the name of the focused window

		if (focusedWindowName.CompareNoCase(_T("World of Flim-Flam Craft Editor")) == 0)		// Check if focused window is the main editor window
		{
			if (m_toolInputCommands->mouse_LB_Down)												// If left mouse button is down
			{
				m_selectedObject = m_d3dRenderer.MousePicking(m_toolInputCommands->ctrlDown);	// Get selected object
				m_toolInputCommands->mouse_LB_Down = false;										// Reset left mouse button state
				if (IsWindow(m_ToolObjectManipDialog->GetSafeHwnd()))							// Check if object manipulation dialog box has been opened
				{
					m_ToolObjectManipDialog->SetObjectData(&m_displayList);						// Send display list to dialog box
					m_ToolObjectManipDialog->SetStacks(&m_undoStack, &m_redoStack);				// Set undo and redo stacks
				}
			}
		}
	}

	if (m_selectedObject.size() > 0)													// If object(s) have been selected
	{
		if (m_toolInputCommands->left || m_toolInputCommands->right ||					// WASD & EQ input check
			m_toolInputCommands->forward || m_toolInputCommands->back || 
			m_toolInputCommands->up || m_toolInputCommands->down)
		{
			if (!m_objManipHeld)														// if movement is not ongoing 
			{
				m_d3dRenderer.GetSelectedObject(m_selectedObject);						// Assign selected objects accoring to IDs 

				for (int i = 0; i < m_d3dRenderer.GetSelectedObjects().size(); i++)		// Loop through selected objects
				{
					if (m_selectedObject[i] == -1)										// If no object is selected
						break;															// break

					DisplayObject& object = *m_d3dRenderer.GetSelectedObjects()[i];		// Get selected object
					DObjectState undoState(object, object.m_ID, false);					// Create undo state
					m_undoStack.push(undoState);										// Push undo state to undo stack
				}
				m_objManipHeld = true;													// movement is ongoing
			}

			if (m_toolInputCommands->editMode == ModelMove)								// If edit mode is ModelMove
				m_d3dRenderer.MoveObjects(m_selectedObject);							// WASD & EQ input moves selected objects
			if (m_toolInputCommands->editMode == ModelRotate)							// If edit mode is ModelRotate
				m_d3dRenderer.RotateObjects(m_selectedObject);							// WASD & EQ input rotates selected objects
			if (m_toolInputCommands->editMode == ModelScale)							// If edit mode is ModelScale				
				m_d3dRenderer.ScaleObjects(m_selectedObject);							// WASD & EQ input scales selected objects
		}
		else if (m_objManipHeld)														// If movement is ongoing
		{
			m_d3dRenderer.GetSelectedObject(m_selectedObject);							// Assign selected objects accoring to IDs 

			for (int i = 0; i < m_d3dRenderer.GetSelectedObjects().size(); i++)			// Loop through selected objects
			{
				if (m_selectedObject[i] == -1)											// If no object is selected
					break;																// break					

				DisplayObject& object = *m_d3dRenderer.GetSelectedObjects()[i];			// Get selected object
				DObjectState undoState(object, object.m_ID, false);						// Create undo state
				m_undoStack.push(undoState);											// Push undo state to undo stack
			}
			m_objManipHeld = false;														// movement is not ongoing
		}
	}
	
	// Undo and Redo function calls /////
	if (m_toolInputCommands->undoDown && !m_toolInputCommands->undoDownPrevState)		// if undo keys are pressed 
	{
		m_d3dRenderer.Undo(&m_undoStack, &m_redoStack, m_sceneGraph);					// Undo previous action
		m_ToolObjectManipDialog->SetStacks(&m_undoStack, &m_redoStack);					// Set undo and redo stacks
	}
	m_toolInputCommands->undoDownPrevState = m_toolInputCommands->undoDown;				// Set prev undo state

	if (m_toolInputCommands->redoDown && !m_toolInputCommands->redoDownPrevState)		// if redo keys are pressed
	{
		m_d3dRenderer.Redo(&m_redoStack, &m_undoStack, m_sceneGraph);					// Redo previous action
		m_ToolObjectManipDialog->SetStacks(&m_undoStack, &m_redoStack);					// Set undo and redo stacks
	}
	m_toolInputCommands->redoDownPrevState = m_toolInputCommands->redoDown;				// Set prev redo state


	///// Copy and paste function calls /////
	if (m_toolInputCommands->copyDown && !m_toolInputCommands->copyDownPrevState)		// if copy keys are pressed 
	{
		m_d3dRenderer.Copy(m_selectedObject, m_copyList, m_sceneGraph);					// Copy selected objects
	}
	m_toolInputCommands->copyDownPrevState = m_toolInputCommands->copyDown;				// Set prev copy state

	if (m_toolInputCommands->pasteDown && !m_toolInputCommands->pasteDownPrevState)		// if paste keys are pressed
	{
		m_d3dRenderer.Paste(m_copyList, m_sceneGraph);									// Paste copied objects		
	}
	m_toolInputCommands->pasteDownPrevState = m_toolInputCommands->pasteDown;			// set prev paste state

	// Focus on object function call /////
	if (m_toolInputCommands->f_key_down && !m_toolInputCommands->focus_prev)			// if focus key is pressed
	{
		m_d3dRenderer.FocusOnObject(m_selectedObject);									// Focus camera on selected object
	}
	m_toolInputCommands->focus_prev = m_toolInputCommands->f_key_down;					// set prev focus state

	///// Delete object function calls /////
	if (m_toolInputCommands->deleteDown)												// if delete key is pressed
	{
		m_d3dRenderer.Delete(m_selectedObject, m_sceneGraph);							// Delete selected objects			
		m_selectedObject.clear();														// Clear selected object IDs to avoid crashes
	}

	///// Create new object functionality /////
	if (m_ToolObjectCreationDialog->GetObjectCreationFlag())							// If new object has been created
	{
		m_d3dRenderer.BuildDisplayList(&m_sceneGraph);									// Build display list
		DisplayObject& object = m_displayList.back();									// get new object

		DObjectState undoState(object, m_sceneGraph.back(), m_sceneGraph.back().ID, true);	// Create undo state
		m_undoStack.push(undoState);														// Push undo states
		m_undoStack.push(undoState);
		m_ToolObjectCreationDialog->SetObjectCreationFlag(false);							// Reset object creation flag
	}

	
	if (IsWindow(m_ToolObjectManipDialog->GetSafeHwnd()))				// Check if object manipulation dialog box has been opened
	{
		if (m_toolInputCommands->editMode == ModelMove ||				// If any of the edit modes are active
			m_toolInputCommands->editMode == ModelRotate ||
			m_toolInputCommands->editMode == ModelScale)
		{
			m_ToolObjectManipDialog->SetObjectData(&m_displayList);		// Set object data
		}
	}


	//Renderer Update Call
	m_d3dRenderer.Tick(m_toolInputCommands);
}

void ToolMain::UpdateInput(MSG * msg)
{
	switch (msg->message)
	{
		//Global inputs,  mouse position and keys etc
	case WM_KEYDOWN:
		m_keyArray[msg->wParam] = true;
		break;

	case WM_KEYUP:
		m_keyArray[msg->wParam] = false;
		break;

	case WM_MOUSEMOVE:
		m_toolInputCommands->mouse_X = GET_X_LPARAM(msg->lParam);
		m_toolInputCommands->mouse_Y = GET_Y_LPARAM(msg->lParam);
		break;

	case WM_LBUTTONDOWN:	//mouse button down,  you will probably need to check when its up too
		m_toolInputCommands->mouse_LB_Down = true;
		break;
	case WM_MBUTTONDOWN:
		m_toolInputCommands->mouse_Mid_Down = true;
		break;
	case WM_MBUTTONUP:
		m_toolInputCommands->mouse_Mid_Down = false;
		break;
	
	}
	//here we update all the actual app functionality that we want.  This information will either be used int toolmain, or sent down to the renderer (Camera movement etc
	//WASD movement
	if (m_keyArray['W'])	m_toolInputCommands->forward = true;
	else					m_toolInputCommands->forward = false;
	
	if (m_keyArray['S'])	m_toolInputCommands->back = true;
	else					m_toolInputCommands->back = false;

	if (m_keyArray['A'])	m_toolInputCommands->left = true;
	else					m_toolInputCommands->left = false;

	if (m_keyArray['D'])	m_toolInputCommands->right = true;
	else					m_toolInputCommands->right = false;
	//rotation
	if (m_keyArray['E'])	m_toolInputCommands->up = true;
	else					m_toolInputCommands->up = false;

	if (m_keyArray['Q'])	m_toolInputCommands->down = true;
	else					m_toolInputCommands->down = false;

	if (m_keyArray['Z'])	m_toolInputCommands->z_key_down = true;
	else					m_toolInputCommands->z_key_down = false;

	if (m_keyArray['Y'])	m_toolInputCommands->y_key_down = true;
	else					m_toolInputCommands->y_key_down = false;


	if (m_keyArray['C'])	m_toolInputCommands->c_key_down = true;
	else					m_toolInputCommands->c_key_down = false;

	if (m_keyArray['V'])	m_toolInputCommands->v_key_down = true;
	else					m_toolInputCommands->v_key_down = false;

	if (m_keyArray['F'])	m_toolInputCommands->f_key_down = true;
	else					m_toolInputCommands->f_key_down = false;

	if (m_keyArray[VK_CONTROL]) m_toolInputCommands->ctrlDown = true;
	else						m_toolInputCommands->ctrlDown = false;

	if (m_keyArray[VK_TAB])		m_toolInputCommands->tabDown = true;
	else						m_toolInputCommands->tabDown = false;

	if (m_keyArray[VK_DELETE])	m_toolInputCommands->deleteDown = true;
	else						m_toolInputCommands->deleteDown = false;


	// Key combinations
	if (m_toolInputCommands->z_key_down && m_toolInputCommands->ctrlDown) 	m_toolInputCommands->undoDown = true;
	else                                                                    m_toolInputCommands->undoDown = false;

	if (m_toolInputCommands->y_key_down && m_toolInputCommands->ctrlDown) 	m_toolInputCommands->redoDown = true;
	else                                                                    m_toolInputCommands->redoDown = false;
		

	if (m_toolInputCommands->c_key_down && m_toolInputCommands->ctrlDown) 	m_toolInputCommands->copyDown = true;
	else                                                                    m_toolInputCommands->copyDown = false;
		
	if (m_toolInputCommands->v_key_down && m_toolInputCommands->ctrlDown) 	m_toolInputCommands->pasteDown = true;
	else                                                                    m_toolInputCommands->pasteDown = false;

}
