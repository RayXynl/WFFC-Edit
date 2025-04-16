#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
#include "SceneObject.h"
#include "DisplayObject.h"
#include "InputCommands.h"
#include "Structures.h"
#include <stack>
#include <vector>

class ObjectManipulationDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ObjectManipulationDialog)
public:
	ObjectManipulationDialog(CWnd* pParent, std::vector<SceneObject>* SceneGraph);
	ObjectManipulationDialog(CWnd* pParent = NULL);
	virtual ~ObjectManipulationDialog();
	void SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<int>* selection, std::vector<DisplayObject>* displayList, InputCommands* inputCommands);	//passing in pointers to the data the class will operate on.
	void SetObjectData(std::vector<DisplayObject>* displaylist);	//passing in pointers to the data the class will operate on.
	void SetStacks(std::stack<DObjectState>* undoStack, std::stack<DObjectState>* redoStack);
	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ObjManip };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void End();		//kill the dialogue

	std::vector<SceneObject>*	m_sceneGraph;
	std::vector<int>*			m_currentSelection;
	std::vector<DisplayObject>*	m_displayList;
	InputCommands*				m_toolInputCommands;

	CEdit						m_editX,	m_editY,	m_editZ;
	CEdit						m_RotX,		m_RotY,		m_RotZ;
	CEdit						m_ScaleX,	m_ScaleY,	m_ScaleZ;

	std::stack<DObjectState>*	m_undoStack;
	std::stack<DObjectState>*	m_redoStack;

	bool						m_onNewSelection;

	DECLARE_MESSAGE_MAP()

private:
	void PushUndo(DisplayObject& object);
	void ApplyObjectChange(float value, TransformType transform);
public:
	// Control variable for more efficient access of the listbox
	virtual BOOL OnInitDialog() override;
	
	virtual void PostNcDestroy();
	afx_msg void OnBnClickedOk();
//	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnEnChangeEditX();
	afx_msg void OnEnChangeEditY();
	afx_msg void OnEnChangeEditZ();
	afx_msg void OnEnChangeRotX();
	afx_msg void OnEnChangeRotY();
	afx_msg void OnEnChangeRotZ();
	afx_msg void OnEnChangeScaleX();
	afx_msg void OnEnChangeScaleY();
	afx_msg void OnEnChangeScaleZ();
	afx_msg void OnBnClickedFreemoveloc();
	//afx_msg void OnBnClickedFreemoveloc2();
	afx_msg void OnBnClickedFreemoverot();
	afx_msg void OnBnClickedFreemovescale();
};

INT_PTR CALLBACK SelectProc(HWND   hwndDlg, UINT   uMsg, WPARAM wParam, LPARAM lParam);