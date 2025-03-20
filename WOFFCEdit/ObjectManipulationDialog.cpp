#include "stdafx.h"
#include "ObjectManipulationDialog.h"

IMPLEMENT_DYNAMIC(ObjectManipulationDialog, CDialogEx)

BEGIN_MESSAGE_MAP(ObjectManipulationDialog, CDialogEx)
	ON_COMMAND(IDOK, &ObjectManipulationDialog::End)					//ok button
	ON_BN_CLICKED(IDOK, &ObjectManipulationDialog::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_X, &ObjectManipulationDialog::OnEnChangeEditX)
	ON_EN_CHANGE(IDC_EDIT_Y, &ObjectManipulationDialog::OnEnChangeEditY)
	ON_EN_CHANGE(IDC_EDIT_Z, &ObjectManipulationDialog::OnEnChangeEditZ)
	ON_EN_CHANGE(IDC_ROT_X, &ObjectManipulationDialog::OnEnChangeRotX)
	ON_EN_CHANGE(IDC_ROT_Y, &ObjectManipulationDialog::OnEnChangeRotY)
	ON_EN_CHANGE(IDC_ROT_Z, &ObjectManipulationDialog::OnEnChangeRotZ)
	ON_EN_CHANGE(IDC_SCALE_X, &ObjectManipulationDialog::OnEnChangeScaleX)
	ON_EN_CHANGE(IDC_SCALE_Y, &ObjectManipulationDialog::OnEnChangeScaleY)
	ON_EN_CHANGE(IDC_SCALE_Z, &ObjectManipulationDialog::OnEnChangeScaleZ)
END_MESSAGE_MAP()


ObjectManipulationDialog::ObjectManipulationDialog(CWnd* pParent, std::vector<SceneObject>* SceneGraph)		//constructor used in modal
	: CDialogEx(IDD_ObjManip, pParent)
{
	m_sceneGraph = SceneGraph;
}

ObjectManipulationDialog::ObjectManipulationDialog(CWnd* pParent)			//constructor used in modeless
	: CDialogEx(IDD_ObjManip, pParent)
{
}

ObjectManipulationDialog::~ObjectManipulationDialog()
{
}

void ObjectManipulationDialog::SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<int>* selection, std::vector<DisplayObject>*	displayList)
{
	m_sceneGraph = SceneGraph;
	m_currentSelection = selection;
	m_displayList = displayList;
}

void ObjectManipulationDialog::SetObjectData(std::vector<DisplayObject>* displaylist)
{
	m_onNewSelection = true;

	m_displayList = displaylist;
	m_editX.SetWindowText(NULL);
	m_editY.SetWindowText(NULL);
	m_editZ.SetWindowText(NULL);

	CString m_Xpos, m_Ypos, m_Zpos;
	CString m_Xrot, m_Yrot, m_Zrot;
	CString m_Xscale, m_Yscale, m_Zscale;
	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
	
		m_Xpos.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_position.x);
		m_Ypos.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_position.y);
		m_Zpos.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_position.z);

		m_Xrot.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_orientation.x);
		m_Yrot.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_orientation.y);
		m_Zrot.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_orientation.z);

		m_Xscale.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_scale.x);
		m_Yscale.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_scale.y);
		m_Zscale.Format(_T("%f"), m_displayList->at(m_currentSelection->front()).m_scale.z);


		m_editX.SetWindowText(m_Xpos);
		m_editY.SetWindowText(m_Ypos);
		m_editZ.SetWindowText(m_Zpos);

		m_RotX.SetWindowText(m_Xpos);
		m_RotY.SetWindowText(m_Ypos);
		m_RotZ.SetWindowText(m_Zpos);

		m_ScaleX.SetWindowText(m_Xscale);
		m_ScaleY.SetWindowText(m_Yscale);
		m_ScaleZ.SetWindowText(m_Zscale);
	}
	else
	{
		m_Xpos.Format(_T("Multiple"));
		m_Ypos.Format(_T("Multiple"));
		m_Zpos.Format(_T("Multiple"));

		m_Xrot.Format(_T("Multiple"));
		m_Yrot.Format(_T("Multiple"));
		m_Zrot.Format(_T("Multiple"));

		m_Xscale.Format(_T("Multiple"));
		m_Yscale.Format(_T("Multiple"));
		m_Zscale.Format(_T("Multiple"));

		m_editX.SetWindowText(m_Xpos);
		m_editY.SetWindowText(m_Ypos);
		m_editZ.SetWindowText(m_Zpos);

		m_RotX.SetWindowText(m_Xrot);
		m_RotY.SetWindowText(m_Yrot);
		m_RotZ.SetWindowText(m_Zrot);

		m_ScaleX.SetWindowText(m_Xscale);
		m_ScaleY.SetWindowText(m_Yscale);
		m_ScaleZ.SetWindowText(m_Zscale);
	}

	m_onNewSelection = false;
}

void ObjectManipulationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_X, m_editX);
	DDX_Control(pDX, IDC_EDIT_Y, m_editY);
	DDX_Control(pDX, IDC_EDIT_Z, m_editZ);

	DDX_Control(pDX, IDC_ROT_X, m_RotX);
	DDX_Control(pDX, IDC_ROT_Y, m_RotY);
	DDX_Control(pDX, IDC_ROT_Z, m_RotZ);

	DDX_Control(pDX, IDC_SCALE_X, m_ScaleX);
	DDX_Control(pDX, IDC_SCALE_Y, m_ScaleY);
	DDX_Control(pDX, IDC_SCALE_Z, m_ScaleZ);
}

void ObjectManipulationDialog::End()
{
	DestroyWindow();
}

BOOL ObjectManipulationDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}


void ObjectManipulationDialog::PostNcDestroy()
{
}

void ObjectManipulationDialog::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


void ObjectManipulationDialog::OnEnChangeEditX()
{
	if (m_onNewSelection)
		return;

	CString strX;
	m_editX.GetWindowText(strX);
	float newX = _ttof(strX); 

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_position.x = newX;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_position.x = newX;
		}
	}
}


void ObjectManipulationDialog::OnEnChangeEditY()
{
	if (m_onNewSelection)
		return;

	CString strY;
	m_editX.GetWindowText(strY);
	float newY = _ttof(strY); 

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_position.y += newY;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_position.y = newY;
		}
	}
}


void ObjectManipulationDialog::OnEnChangeEditZ()
{
	if (m_onNewSelection)
		return;

	CString strZ;
	m_editX.GetWindowText(strZ);
	float newZ = _ttof(strZ);

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_position.z = newZ;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_position.z = newZ;
		}
	}
}


void ObjectManipulationDialog::OnEnChangeRotX()
{
	if (m_onNewSelection)
		return;

	CString zRotate;
	m_RotX.GetWindowText(zRotate);
	float newRot = _ttof(zRotate);

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_orientation.x = newRot;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_orientation.x = newRot;
		}
	}
}


void ObjectManipulationDialog::OnEnChangeRotY()
{
	if (m_onNewSelection)
		return;

	CString yRotate;
	m_RotY.GetWindowText(yRotate);
	float newRot = _ttof(yRotate);

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_orientation.y = newRot;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_orientation.y = newRot;
		}
	}
}



void ObjectManipulationDialog::OnEnChangeRotZ()
{
	if (m_onNewSelection)
		return;

	CString zRotate;
	m_RotZ.GetWindowText(zRotate);
	float newRot = _ttof(zRotate);

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_orientation.z = newRot;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_orientation.z = newRot;
		}
	}
}


void ObjectManipulationDialog::OnEnChangeScaleX()
{
	if (m_onNewSelection)
		return;

	CString xScale;
	m_ScaleX.GetWindowText(xScale);
	float newScale = _ttof(xScale);

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_scale.x = newScale;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_scale.x = newScale;
		}
	}
}


void ObjectManipulationDialog::OnEnChangeScaleY()
{
	if (m_onNewSelection)
		return;

	CString yScale;
	m_ScaleY.GetWindowText(yScale);
	float newScale = _ttof(yScale);

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_scale.y = newScale;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_scale.y = newScale;
		}
	}
}


void ObjectManipulationDialog::OnEnChangeScaleZ()
{
	if (m_onNewSelection)
		return;

	CString zScale;
	m_ScaleZ.GetWindowText(zScale);
	float newScale = _ttof(zScale);

	if (m_currentSelection->size() == 1 && m_currentSelection->front() != -1)
	{
		m_displayList->at(m_currentSelection->front()).m_scale.z = newScale;

	}
	else if (m_currentSelection->size() > 1)
	{
		for (int i = 0; i < m_currentSelection->size(); i++)
		{
			m_displayList->at(m_currentSelection->at(i)).m_scale.z = newScale;
		}
	}
}
