#include "ObjectionCreationDialog.h"
#include "stdafx.h"
#include <fstream>

IMPLEMENT_DYNAMIC(ObjectionCreationDialog, CDialogEx)

BEGIN_MESSAGE_MAP(ObjectionCreationDialog, CDialogEx)
	ON_COMMAND(IDOK, &ObjectionCreationDialog::End)					//ok button
	ON_BN_CLICKED(IDOK, &ObjectionCreationDialog::OnBnClickedOk)
	ON_EN_CHANGE(IDC_MeshEdit, &ObjectionCreationDialog::OnEnChangeMeshedit)
	ON_BN_CLICKED(IDC_CreateObjBut, &ObjectionCreationDialog::OnBnClickedCreateobjbut)
END_MESSAGE_MAP()

ObjectionCreationDialog::ObjectionCreationDialog(CWnd* pParent, std::vector<SceneObject>* SceneGraph)
	: CDialogEx(IDD_ObjCreate, pParent)
{
	m_sceneGraph = SceneGraph;


}


ObjectionCreationDialog::ObjectionCreationDialog(CWnd* pParent) 
	: CDialogEx(IDD_ObjCreate, pParent)
{

}

ObjectionCreationDialog::~ObjectionCreationDialog()
{
}

void ObjectionCreationDialog::SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<int>* selection, std::vector<DisplayObject>* displayList, InputCommands* inputCommands)
{
	m_sceneGraph = SceneGraph;
	m_currentSelection = selection;
	m_displayList = displayList;
	m_toolInputCommands = inputCommands;

	m_path = "database/data/";
}

void ObjectionCreationDialog::SetStacks(std::stack<DObjectState>* undoStack, std::stack<DObjectState>* redoStack)
{
	m_undoStack = undoStack;
	m_redoStack = redoStack;
}

void ObjectionCreationDialog::CreateObject(std::string modelPath, std::string texPath, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	// Create a new SceneObject
	SceneObject newSceneObject;

	newSceneObject.ID = FindMaxCurrentID();
	newSceneObject.chunk_ID = 0;
	newSceneObject.model_path = m_path + modelPath;
	newSceneObject.tex_diffuse_path = m_path + texPath;

	// Default transform
	newSceneObject.posX = position.x;
	newSceneObject.posY = position.y;
	newSceneObject.posZ = position.z;
	newSceneObject.rotX = rotation.x;
	newSceneObject.rotY = rotation.y;
	newSceneObject.rotZ = rotation.z;
	newSceneObject.scaX = scale.x;
	newSceneObject.scaY = scale.x;
	newSceneObject.scaZ = scale.x;

	// Default settings
	newSceneObject.render = 1;
	newSceneObject.collision = 0;
	newSceneObject.collision_mesh = "";
	newSceneObject.collectable = 0;
	newSceneObject.destructable = 0;
	newSceneObject.health_amount = 0;

	// Editor flags
	newSceneObject.editor_render = 1;
	newSceneObject.editor_texture_vis = 1;
	newSceneObject.editor_normals_vis = 0;
	newSceneObject.editor_collision_vis = 0;
	newSceneObject.editor_pivot_vis = 0;

	// Pivot
	newSceneObject.pivotX = 0.0f;
	newSceneObject.pivotY = 0.0f;
	newSceneObject.pivotZ = 0.0f;

	// Audio
	newSceneObject.audio_path = "";
	newSceneObject.volume = 1.0f;
	newSceneObject.pitch = 1.0f;
	newSceneObject.pan = 0;
	newSceneObject.one_shot = 0;
	newSceneObject.play_on_init = 0;
	newSceneObject.play_in_editor = 0;
	newSceneObject.min_dist = 1.0f;
	newSceneObject.max_dist = 15.0f;

	// AI & Camera
	newSceneObject.AINode = 0;
	newSceneObject.camera = 0;
	newSceneObject.path_node = 0;
	newSceneObject.path_node_start = 0;
	newSceneObject.path_node_end = 0;
	newSceneObject.parent_id = -1;

	// Name & Visuals
	newSceneObject.name = "new";
	newSceneObject.editor_wireframe = 0;

	// Light settings
	newSceneObject.light_type = 0;
	newSceneObject.light_diffuse_r = 1.0f;
	newSceneObject.light_diffuse_g = 1.0f;
	newSceneObject.light_diffuse_b = 1.0f;
	newSceneObject.light_specular_r = 1.0f;
	newSceneObject.light_specular_g = 1.0f;
	newSceneObject.light_specular_b = 1.0f;
	newSceneObject.light_spot_cutoff = 45.0f;
	newSceneObject.light_constant = 1.0f;
	newSceneObject.light_linear = 0.09f;
	newSceneObject.light_quadratic = 0.032f;

	m_objectCreationFlag = true;

	// Add the new object to the scene graph
	m_sceneGraph->push_back(newSceneObject);
}

void ObjectionCreationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MeshEdit, m_mesh);
	DDX_Control(pDX, IDC_TextureEdit, m_texture);
	DDX_Control(pDX, IDC_POSX, m_editX);
	DDX_Control(pDX, IDC_POSY, m_editY);
	DDX_Control(pDX, IDC_POSZ, m_editZ);
	DDX_Control(pDX, IDC_ROTX, m_RotX);
	DDX_Control(pDX, IDC_ROTY, m_RotY);
	DDX_Control(pDX, IDC_ROTZ, m_RotZ);
	DDX_Control(pDX, IDC_SCALEX, m_ScaleX);
	DDX_Control(pDX, IDC_SCALEY, m_ScaleY);
	DDX_Control(pDX, IDC_SCALEZ, m_ScaleZ);
}

void ObjectionCreationDialog::End()
{
	DestroyWindow();
}

BOOL ObjectionCreationDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}

void ObjectionCreationDialog::PostNcDestroy()
{
}

void ObjectionCreationDialog::OnBnClickedOk()
{
	CDialogEx::OnOK();
}



void ObjectionCreationDialog::OnEnChangeMeshedit()
{

}


void ObjectionCreationDialog::OnBnClickedCreateobjbut()
{
	CString meshPath;
	m_mesh.GetWindowText(meshPath);
	std::string mesh = CT2A(meshPath.GetString());
	
	std::ifstream meshFile(m_path + mesh);
	if (!meshFile.good())
	{
		MessageBox(_T("Mesh file not found!"), _T("Error"), MB_OK | MB_ICONERROR);
		return;
	}

	CString texPath;
	m_texture.GetWindowText(texPath);
	std::string tex = CT2A(texPath.GetString());

	std::ifstream texFile(m_path + tex);
	if (!texFile.good())
	{
		MessageBox(_T("Mesh file not found!"), _T("Error"), MB_OK | MB_ICONERROR);
		return;
	}

	XMFLOAT3 position = XMFLOAT3(0, 0, 0);

	CString posX;
	m_editX.GetWindowText(posX);
	position.x = _ttof(posX);

	CString posY;
	m_editY.GetWindowText(posY);
	position.y = _ttof(posY);

	CString posZ;
	m_editZ.GetWindowText(posZ);
	position.z = _ttof(posZ);

	XMFLOAT3 rotation = XMFLOAT3(0, 0, 0);

	CString rotX;
	m_RotX.GetWindowText(rotX);
	rotation.x = _ttof(rotX);

	CString rotY;
	m_RotY.GetWindowText(rotY);
	rotation.y = _ttof(rotY);

	CString rotZ;
	m_RotZ.GetWindowText(posZ);
	rotation.z = _ttof(posZ);

	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);

	CString scaleX;
	m_ScaleX.GetWindowText(scaleX);
	scale.x = _ttof(scaleX);

	CString scaleY;
	m_ScaleY.GetWindowText(scaleY);
	scale .y = _ttof(scaleY);

	CString scaleZ;
	m_ScaleZ.GetWindowText(scaleZ);
	scale.z = _ttof(scaleZ);

	CreateObject(mesh, tex, position, rotation, scale);
}

int ObjectionCreationDialog::FindMaxCurrentID()
{
	int maxID = 0;

	for (int i = 0; i < m_sceneGraph->size(); i++)
	{
		if (m_sceneGraph->at(i).ID > maxID)
		{
			maxID = m_sceneGraph->at(i).ID;
		}
	}

	return maxID + 1;
}
