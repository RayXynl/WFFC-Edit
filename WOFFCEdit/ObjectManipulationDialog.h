#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
#include "SceneObject.h"
#include "DisplayObject.h"
#include <vector>

class ObjectManipulationDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ObjectManipulationDialog)
public:
	ObjectManipulationDialog(CWnd* pParent, std::vector<SceneObject>* SceneGraph);
	ObjectManipulationDialog(CWnd* pParent = NULL);
	virtual ~ObjectManipulationDialog();
	void SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<int>* Selection, std::vector<DisplayObject>* displaylist);	//passing in pointers to the data the class will operate on.
	void SetObjectData(std::vector<DisplayObject>* displaylist);	//passing in pointers to the data the class will operate on.

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ObjManip };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void End();		//kill the dialogue
	//afx_msg void Select();	//Item has been selected

	std::vector<SceneObject>* m_sceneGraph;
	//std::vector<DisplayObject>			m_displayList;
	std::vector<int>* m_currentSelection;
	std::vector<DisplayObject>*	m_displayList;
	CEdit m_editX, m_editY, m_editZ;
	CEdit m_RotX, m_RotY, m_RotZ;
	CEdit m_ScaleX, m_ScaleY, m_ScaleZ;

	bool m_onNewSelection;

	DECLARE_MESSAGE_MAP()
public:
	// Control variable for more efficient access of the listbox
	virtual BOOL OnInitDialog() override;
	
	virtual void PostNcDestroy();
	afx_msg void OnBnClickedOk();
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnEnChangeEditX();
	afx_msg void OnEnChangeEditY();
	afx_msg void OnEnChangeEditZ();
	afx_msg void OnEnChangeRotX();
	afx_msg void OnEnChangeRotY();
	afx_msg void OnEnChangeRotZ();
	afx_msg void OnEnChangeScaleX();
	afx_msg void OnEnChangeScaleY();
	afx_msg void OnEnChangeScaleZ();
};

INT_PTR CALLBACK SelectProc(HWND   hwndDlg, UINT   uMsg, WPARAM wParam, LPARAM lParam);