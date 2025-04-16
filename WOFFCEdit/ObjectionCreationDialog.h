#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
#include "SceneObject.h"
#include <vector>
#include "DisplayObject.h"
#include "InputCommands.h"
#include "Structures.h"
#include <stack>

class ObjectionCreationDialog :public CDialogEx
{
	DECLARE_DYNAMIC(ObjectionCreationDialog)

public:
	ObjectionCreationDialog(CWnd* pParent, std::vector<SceneObject>* SceneGraph);   // modal // takes in out scenegraph in the constructor
	ObjectionCreationDialog(CWnd* pParent = NULL);
	virtual ~ObjectionCreationDialog();
	void SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<int>* selection, std::vector<DisplayObject>* displayList, InputCommands* inputCommands);	//passing in pointers to the data the class will operate on.
	void SetStacks(std::stack<DObjectState>* undoStack, std::stack<DObjectState>* redoStack);

	void CreateObject(std::string modelPath, std::string texPath, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale);

	void SetObjectCreationFlag(bool flag)	{ m_objectCreationFlag = flag; }
	bool GetObjectCreationFlag()			{ return m_objectCreationFlag; }
	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ObjCreate };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void End();		//kill the dialogue


	CEdit							m_mesh, m_texture;
	CEdit							m_editX, m_editY, m_editZ;
	CEdit							m_RotX, m_RotY, m_RotZ;
	CEdit							m_ScaleX, m_ScaleY, m_ScaleZ;

	std::vector<SceneObject>*		m_sceneGraph;
	std::vector<DisplayObject>*		m_displayList;
	std::vector<int>*				m_currentSelection;
	InputCommands*					m_toolInputCommands;

	std::stack<DObjectState>*		m_undoStack;
	std::stack<DObjectState>*		m_redoStack;

	std::string						m_path;

	bool							m_objectCreationFlag;

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog() override;
	virtual void PostNcDestroy();
	afx_msg void OnBnClickedOk();

	afx_msg void OnEnChangeMeshedit();
	afx_msg void OnBnClickedCreateobjbut();

private:
	int FindMaxCurrentID();
};

