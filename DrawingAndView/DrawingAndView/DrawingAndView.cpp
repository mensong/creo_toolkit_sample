// DrawingAndView.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "DrawingAndView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CDrawingAndViewApp

BEGIN_MESSAGE_MAP(CDrawingAndViewApp, CWinApp)
END_MESSAGE_MAP()

// CDrawingAndViewApp 构造

CDrawingAndViewApp::CDrawingAndViewApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CDrawingAndViewApp 对象

CDrawingAndViewApp theApp;

// CDrawingAndViewApp 初始化

BOOL CDrawingAndViewApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

int CurrentMdlType()
{
	ProMdl mdl;
	ProMdlType mdlType;
	ProError status;
	status = ProMdlCurrentGet(&mdl);
	if (status != PRO_TK_NO_ERROR)
		return -1;
	status = ProMdlTypeGet(mdl, &mdlType);
	if (status != PRO_TK_NO_ERROR)
		return -1;
	else
		return mdlType;
}

static uiCmdAccessState AccessPRTorASM(uiCmdAccessMode access_mode)
{
	if (CurrentMdlType() == PRO_ASSEMBLY || CurrentMdlType() == PRO_PART)
		return ACCESS_AVAILABLE;
	else
		return ACCESS_INVISIBLE;
}

static uiCmdAccessState AccessDRW(uiCmdAccessMode access_mode)
{
	if (CurrentMdlType() == PRO_DRAWING)
		return ACCESS_AVAILABLE;
	else
		return ACCESS_INVISIBLE;
}

ProError _setDisplayStyle(ProDrawing drawing, ProView view, ProDisplayStyle style)
{
	ProError status;
	ProDrawingViewDisplay displayStatus;
	status = ProDrawingViewDisplayGet(drawing, view, &displayStatus);
	displayStatus.style = style;
	status = ProDrawingViewDisplaySet(drawing, view, &displayStatus);
	return status;
}

ProError _createDrawing(CString TemplateName)
{
	ProMdl solidmdl;
	ProPath currentpath;
	ProError status;
	ProMdlType mdlType;
	ProMdldata data;
	ProModel model;
	ProDrawing created_drawing = NULL;
	ProDwgcreateOptions options = PRODWGCREATE_DISPLAY_DRAWING;
	ProDwgcreateErrs errors;
	int new_win_id;

	status = ProMdlCurrentGet(&solidmdl);
	if (status != PRO_TK_NO_ERROR)
		return status;
	status = ProMdlTypeGet(solidmdl, &mdlType);
	if (status != PRO_TK_NO_ERROR)
		return status;

	if (mdlType == PRO_MDL_PART || mdlType == PRO_MDL_ASSEMBLY)
	{
		status = ProMdlDataGet(solidmdl, &data);
		status = ProDirectoryCurrentGet(currentpath);
		CString Path = CString(data.device) + _T(":") + CString(data.path);
		wchar_t *path = Path.AllocSysString();
		status = ProDirectoryChange(path);
		if (path != NULL)
			SysFreeString(path);

		for (int i = 0; i < 10; i++)
		{
			model.type[i] = data.type[i];
		}
		for (int i = 0; i < 80; i++)
		{
			model.name[i] = data.name[i];
		}

		wchar_t *wtemplatename = NULL;
		wtemplatename = TemplateName.AllocSysString();

		status = ProDrawingFromTmpltCreate(model.name, wtemplatename, &model, options, &created_drawing, &errors);

		if (wtemplatename != NULL)
			SysFreeString(wtemplatename);

		if (status == PRO_TK_NO_ERROR)
		{
			status = ProObjectwindowCreate(model.name, PRO_DRAWING, &new_win_id);
		}
		else if (status == PRO_TK_E_FOUND)
		{
			status = ProMdlLoad(model.name, PRO_MDL_DRAWING, PRO_B_FALSE, (ProMdl *)&created_drawing);
			status = ProObjectwindowCreate(model.name, PRO_DRAWING, &new_win_id);
		}
		else
		{
			status = ProDirectoryCurrentGet(currentpath);
			return PRO_TK_NO_ERROR;
		}
	}
	else
	{
		return PRO_TK_E_NOT_FOUND;
	}
	status = ProMdlDisplay(created_drawing);
	status = ProWindowActivate(new_win_id);
	status = ProDirectoryChange(currentpath);
	return PRO_TK_NO_ERROR;
}

void CreateDrw()
{
	if (_createDrawing(_T("D:\\mydoc\\creo_toolkit\\DrawingAndView\\drwtpl.drw")) == PRO_TK_NO_ERROR)
	{
		//do some thing
	}
}

ProError _createView()
{
	ProError status;
	ProMdl mdl;
	ProPoint3d position;
	ProMatrix matrix;
	ProView positive_view, top_view, left_view;
	ProDrawing drawing;
	ProSolid solid;
	ProMdlType mdlType;
	int sheet;

	status = ProMdlCurrentGet(&mdl);
	if (status != PRO_TK_NO_ERROR)
		return status;
	status = ProMdlTypeGet(mdl, &mdlType);
	if (status != PRO_TK_NO_ERROR)
		return status;

	if (mdlType == PRO_DRAWING)
	{
		drawing = (ProDrawing)mdl;
		status = ProDrawingCurrentsolidGet(drawing, &solid);
		status = ProDrawingCurrentSheetGet(drawing, &sheet);

		position[0] = 200;
		position[1] = 600;
		position[2] = 0;

		//////////////定义摆放方向,FRONT，设置比例0.015
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				matrix[i][j] = i == j ? 1 : 0;
			}
		}

		status = ProDrawingGeneralviewCreate(drawing, solid, sheet, PRO_B_FALSE, position, 1, matrix, &positive_view);
		status = _setDisplayStyle(drawing, positive_view, PRO_DISPSTYLE_HIDDEN_LINE);
		status = ProDrawingViewScaleSet(drawing, positive_view, 0.15);

		position[0] += 500;
		status = ProDrawingProjectedviewCreate(drawing, positive_view, PRO_B_FALSE, position, &top_view);
		status = _setDisplayStyle(drawing, top_view, PRO_DISPSTYLE_HIDDEN_LINE);

		position[0] -= 500;
		position[1] -= 400;
		status = ProDrawingProjectedviewCreate(drawing, positive_view, PRO_B_FALSE, position, &left_view);
		status = _setDisplayStyle(drawing, left_view, PRO_DISPSTYLE_HIDDEN_LINE);
		status = ProDwgSheetRegenerate(drawing, sheet);
		return PRO_TK_NO_ERROR;
	}
	else
		return PRO_TK_BAD_CONTEXT;
}

void CreateView()
{
	if (_createView() == PRO_TK_NO_ERROR)
	{
		//do some thing
	}
}

ProError _createAuxiliayView()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ProError status;
	ProMdl mdl;
	ProView auxiliaryView;
	ProDrawing drawing;
	ProMdlType mdlType;
	ProSelection *sel;
	int n_sel;
	int sheet;
	ProPoint3d point;
	//根据实际确定位置
	point[0] = 300;
	point[1] = 0;
	point[2] = 0;

	status = ProMdlCurrentGet(&mdl);
	if (status != PRO_TK_NO_ERROR)
		return status;
	status = ProMdlTypeGet(mdl, &mdlType);
	if (status != PRO_TK_NO_ERROR)
		return status;

	if (mdlType == PRO_DRAWING)
	{
		drawing = (ProDrawing)mdl;
		status = ProDrawingCurrentSheetGet(drawing, &sheet);

		drawing = (ProDrawing)mdl;
		AfxMessageBox(_T("请选择一条边以生成辅助视图。"));
		status = ProSelect((char *)"edge", 1, NULL, NULL, NULL, NULL, &sel, &n_sel);
		if (status == PRO_TK_NO_ERROR)
		{
			status = ProDrawingViewAuxiliaryCreate(drawing, *sel, point, &auxiliaryView);
			status = _setDisplayStyle(drawing, auxiliaryView, PRO_DISPSTYLE_HIDDEN_LINE);
			status = ProDwgSheetRegenerate(drawing, sheet);
			return PRO_TK_NO_ERROR;
		}
		else
			return status;
	}
	else
		return PRO_TK_BAD_CONTEXT;
}

void CreateAuxiliaryView()
{
	if (_createAuxiliayView() == PRO_TK_NO_ERROR)
	{
		//do some thing
	}
}

ProError _create2DSectionView()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ProError status;
	ProMdl mdl;
	ProView parentView, _2DSectionView;
	ProDrawing drawing;
	ProSolid solid;
	ProMdlType mdlType;
	ProSelection *sel;
	int n_sel;
	int sheet;
	ProPoint3d refPoint;

	status = ProMdlCurrentGet(&mdl);
	if (status != PRO_TK_NO_ERROR)
		return status;
	status = ProMdlTypeGet(mdl, &mdlType);
	if (status != PRO_TK_NO_ERROR)
		return status;

	if (mdlType == PRO_DRAWING)
	{
		drawing = (ProDrawing)mdl;
		status = ProDrawingCurrentSheetGet(drawing, &sheet);
		status = ProDrawingCurrentsolidGet(drawing, &solid);

		AfxMessageBox(_T("本例仅为示例，所以代码写死，默认创建的视图横截面名为\"TESTSEC\"，所以请确保当前模型已创建了一个名为\"TESTSEC\"的截面。\n接下来请选择一个视图以生成剖视图。"));
		status = ProSelect((char *)"dwg_view", 1, NULL, NULL, NULL, NULL, &sel, &n_sel);
		if (status == PRO_TK_NO_ERROR)
		{
			status = ProSelectionPoint3dGet(sel[0], refPoint);
			status = ProSelectionViewGet(sel[0], &parentView);

			//根据实际计算调整，这里做死了
			refPoint[1] -= 300;
			status = ProDrawingProjectedviewCreate(drawing, parentView, PRO_B_FALSE, refPoint, &_2DSectionView);
			status = ProDrawingView2DSectionSet(drawing, _2DSectionView, L"TESTSEC", PRO_VIEW_SECTION_AREA_FULL, NULL, NULL, parentView);
			status = _setDisplayStyle(drawing, _2DSectionView, PRO_DISPSTYLE_HIDDEN_LINE);
			status = ProDwgSheetRegenerate(drawing, sheet);
			return PRO_TK_NO_ERROR;
		}
		else
			return status;
	}
	else
		return PRO_TK_BAD_CONTEXT;
}

void Create2DSectionView()
{
	if (_create2DSectionView() == PRO_TK_NO_ERROR)
	{
		//do some thing
	}
}
//下面两个函数直接拷贝官方帮助文件
/*====================================================================*\
    FUNCTION :	ProUtilVectorDiff()
    PURPOSE  :	Difference of two vectors
\*====================================================================*/
double *ProUtilVectorDiff(double a[3], double b[3], double c[3])
{
	c[0] = a[0] - b[0];
	c[1] = a[1] - b[1];
	c[2] = a[2] - b[2];
	return (c);
}

/*====================================================================*\
    FUNCTION :	ProUtilVectorLength()
    PURPOSE  :	Length of a vector
\*====================================================================*/
double ProUtilVectorLength(double v[3])
{
	return (sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));
}

ProError _createDetailedView()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ProError status;
	ProMdl mdl;
	ProView parentView, detailedView;
	ProDrawing drawing;
	ProMdlType mdlType;
	ProSelection *sel;
	ProSolid solid;
	int n_sel;
	int sheet;
	ProPoint3d refPoint;

	int np, n;
	ProMouseButton btn;
	Pro3dPnt sel_pnt, *pnt_arr, chord, *p_tan, *tan_arr;
	ProCurvedata crv_data;

	double start_angle = 0.0, end_angle = 0.0, radius = 0.0;
	double angle = 0.0, maj_lenth = 0.0, min_lenth = 0.0, len = 0.0;
	double range[2] = {0.0, 90.0}, *par_arr;

	status = ProMdlCurrentGet(&mdl);
	if (status != PRO_TK_NO_ERROR)
		return status;
	status = ProMdlTypeGet(mdl, &mdlType);
	if (status != PRO_TK_NO_ERROR)
		return status;

	if (mdlType == PRO_DRAWING)
	{
		drawing = (ProDrawing)mdl;
		status = ProDrawingCurrentSheetGet(drawing, &sheet);
		status = ProDrawingCurrentsolidGet(drawing, &solid);

		AfxMessageBox(_T("请选择一个边以生成详细视图。"));
		status = ProSelect((char *)"edge", 1, NULL, NULL, NULL, NULL, &sel, &n_sel);
		if (status == PRO_TK_NO_ERROR)
		{
			status = ProSelectionPoint3dGet(sel[0], refPoint);
			status = ProSelectionViewGet(sel[0], &parentView);

			status = ProArrayAlloc(0, sizeof(ProPoint3d), 1, (ProArray *)&pnt_arr);
			AfxMessageBox(_T("请点击选择边周围的点以生成边界，按鼠标中键以结束。"));

			//做演示这里手动选择点，实际可以根据上面选择的点然后上下左右各偏移一段距离自动生成四个点作为圆的内接正方形即可
			while (TRUE)
			{
				status = ProMousePickGet(PRO_ANY_BUTTON, &btn, sel_pnt);
				if (btn != PRO_LEFT_BUTTON || status != PRO_TK_NO_ERROR)
					break;
				ProGraphicsCircleDraw(sel_pnt, 0.5);
				status = ProArrayObjectAdd((ProArray *)&pnt_arr, PRO_VALUE_UNUSED, 1, sel_pnt);
			}
			status = ProArraySizeGet((ProArray)pnt_arr, &np);
			if (status != PRO_TK_NO_ERROR || np == 0)
				return PRO_TK_BAD_CONTEXT;
			status = ProArrayAlloc(0, sizeof(ProPoint3d), 1, (ProArray *)&p_tan);
			status = ProArrayAlloc(0, sizeof(double), 1, (ProArray *)&par_arr);
			tan_arr = (ProPoint3d *)calloc(np, sizeof(ProPoint3d));
			tan_arr[0][0] = pnt_arr[1][0] - pnt_arr[0][0];
			tan_arr[0][1] = 2 * pnt_arr[1][1] - pnt_arr[2][1] - pnt_arr[0][1];
			tan_arr[np - 1][0] = -(pnt_arr[np - 2][0] - pnt_arr[np - 1][0]);
			tan_arr[np - 1][1] = -(2 * pnt_arr[np - 2][1] - pnt_arr[np - 3][1] - pnt_arr[np - 1][1]);

			for (n = 1; n < np - 1; n++)
			{
				tan_arr[n][0] = pnt_arr[n + 1][0] - pnt_arr[n - 1][0];
				tan_arr[n][1] = pnt_arr[n + 1][1] - pnt_arr[n - 1][1];
			}
			for (n = 0; n < np; n++)
			{
				len = (tan_arr[n][0] * tan_arr[n][0]) + (tan_arr[n][1] * tan_arr[n][1]);
				len = sqrt(len);
				tan_arr[n][0] /= len;
				tan_arr[n][1] /= len;
				status = ProArrayObjectAdd((ProArray *)&p_tan, PRO_VALUE_UNUSED, 1, tan_arr[n]);
			}
			angle = 0.0;
			status = ProArrayObjectAdd((ProArray *)&par_arr, PRO_VALUE_UNUSED, 1, &angle);
			for (n = 1; n < np; n++)
			{
				ProUtilVectorDiff(pnt_arr[n], pnt_arr[n - 1], chord);
				angle = ProUtilVectorLength(chord) + par_arr[n - 1];
				status = ProArrayObjectAdd((ProArray *)&par_arr, PRO_VALUE_UNUSED, 1, &angle);
			}
			status = ProSplinedataInit(par_arr, pnt_arr, p_tan, np, &crv_data);

			//根据实际计算调整，这里做死了
			refPoint[1] -= 100;
			status = ProDrawingViewDetailCreate(drawing, parentView, sel[0], &crv_data, refPoint, &detailedView);
			status = _setDisplayStyle(drawing, detailedView, PRO_DISPSTYLE_HIDDEN_LINE);
			status = ProDwgSheetRegenerate(drawing, sheet);

			status = ProArrayFree((ProArray *)&p_tan);
			status = ProArrayFree((ProArray *)&par_arr);
			status = ProArrayFree((ProArray *)&pnt_arr);
			return PRO_TK_NO_ERROR;
		}
		else
			return status;
	}
	else
		return PRO_TK_BAD_CONTEXT;
}

void CreateDetailedView()
{
	if (_createDetailedView() == PRO_TK_NO_ERROR)
	{
		//do some thing
	}
}

extern "C" int user_initialize()
{
	ProError status;
	uiCmdCmdId CreateDrwID, CreateViewID, CreateAuxiliaryViewID, CreateDetailedViewID, Create2DSectionViewID;

	status = ProMenubarMenuAdd("DrawingAndView", "DrawingAndView", "About", PRO_B_TRUE, MSGFILE);
	status = ProMenubarmenuMenuAdd("DrawingAndView", "DrawingAndView", "DrawingAndView", NULL, PRO_B_TRUE, MSGFILE);

	status = ProCmdActionAdd("CreateDrw_Act", (uiCmdCmdActFn)CreateDrw, uiProeImmediate, AccessPRTorASM, PRO_B_TRUE, PRO_B_TRUE, &CreateDrwID);
	status = ProMenubarmenuPushbuttonAdd("DrawingAndView", "CreateDrw", "CreateDrw", "CreateDrwtips", NULL, PRO_B_TRUE, CreateDrwID, MSGFILE);

	status = ProCmdActionAdd("CreateView_Act", (uiCmdCmdActFn)CreateView, uiProeImmediate, AccessDRW, PRO_B_TRUE, PRO_B_TRUE, &CreateViewID);
	status = ProMenubarmenuPushbuttonAdd("DrawingAndView", "CreateView", "CreateView", "CreateViewtips", NULL, PRO_B_TRUE, CreateViewID, MSGFILE);

	status = ProCmdActionAdd("CreateAuxiliaryView_Act", (uiCmdCmdActFn)CreateAuxiliaryView, uiProeImmediate, AccessDRW, PRO_B_TRUE, PRO_B_TRUE, &CreateAuxiliaryViewID);
	status = ProMenubarmenuPushbuttonAdd("DrawingAndView", "CreateAuxiliaryView", "CreateAuxiliaryView", "CreateAuxiliaryViewtips", NULL, PRO_B_TRUE, CreateAuxiliaryViewID, MSGFILE);

	status = ProCmdActionAdd("CreateDetailedView_Act", (uiCmdCmdActFn)CreateDetailedView, uiProeImmediate, AccessDRW, PRO_B_TRUE, PRO_B_TRUE, &CreateDetailedViewID);
	status = ProMenubarmenuPushbuttonAdd("DrawingAndView", "CreateDetailedView", "CreateDetailedView", "CreateDetailedViewtips", NULL, PRO_B_TRUE, CreateDetailedViewID, MSGFILE);

	status = ProCmdActionAdd("Create2DSectionView_Act", (uiCmdCmdActFn)Create2DSectionView, uiProeImmediate, AccessDRW, PRO_B_TRUE, PRO_B_TRUE, &Create2DSectionViewID);
	status = ProMenubarmenuPushbuttonAdd("DrawingAndView", "Create2DSectionView", "Create2DSectionView", "Create2DSectionViewtips", NULL, PRO_B_TRUE, Create2DSectionViewID, MSGFILE);

	return PRO_TK_NO_ERROR;
}

extern "C" void user_terminate()
{
}