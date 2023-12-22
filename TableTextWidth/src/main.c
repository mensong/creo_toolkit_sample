#include "./includes/main.h"

int CurrentMdlType()
{
    ProMdl mdl;
    ProMdlType mdltype;
    ProError status;
    status = ProMdlCurrentGet(&mdl);
    if (status != PRO_TK_NO_ERROR)
        return -1;
    status = ProMdlTypeGet(mdl, &mdltype);
    if (status != PRO_TK_NO_ERROR)
        return -1;
    else
        return mdltype;
}

static uiCmdAccessState AccessDRW(uiCmdAccessMode access_mode)
{
    if (CurrentMdlType() == PRO_DRAWING)
        return ACCESS_AVAILABLE;
    else
        return ACCESS_INVISIBLE;
}

void WrapText()
{
    ProError status;
    ProSelection *tableSels = NULL;
    int size;
    int table_segment, row, column;
    ProDwgtable table;

    status = ProMessageDisplay(MSGFILE, "IMI_PrompSelectTableCell");
    status = ProSelect((char *)"table_cell", 1, NULL, NULL, NULL, NULL, &tableSels, &size);
    if (status != PRO_TK_NO_ERROR || size < 1)
    {
        return;
    }

    status = ProArraySizeGet(tableSels, &size);
    if (status != PRO_TK_NO_ERROR || size <= 0)
    {
        return;
    }
    status = ProSelectionDwgtblcellGet(tableSels[0], &table_segment, &row, &column);
    if (status != PRO_TK_NO_ERROR)
    {
        return;
    }
    status = ProSelectionDwgtableGet(tableSels[0], &table);
    if (status != PRO_TK_NO_ERROR)
    {
        return;
    }
    status = ProDwgtableCelltextWrap(&table, row + 1, column + 1);
    status = ProMacroLoad(L"~ Command `ProCmdDwgRegenModel` ;~ Command `ProCmdWinActivate`;");
}

void SetAutoTextWidth()
{
    ProError status;
    ProSelection *tableSels = NULL;
    int size;
    int table_segment, row, column;
    double width;
    ProDwgtable table;
    ProDtlnote note;
    ProTextStyle textStyle;
    double textwidth = 0.85;
    ProVector envel[4];
    double cell_size;
    status = ProMessageDisplay(MSGFILE, "IMI_PrompSelectTableCell");
    status = ProSelect((char *)"table_cell", 1, NULL, NULL, NULL, NULL, &tableSels, &size);
    if (status != PRO_TK_NO_ERROR || size < 1)
    {
        return;
    }

    status = ProArraySizeGet(tableSels, &size);
    if (status != PRO_TK_NO_ERROR || size <= 0)
    {
        return;
    }
    status = ProSelectionDwgtblcellGet(tableSels[0], &table_segment, &row, &column);
    if (status != PRO_TK_NO_ERROR)
    {
        return;
    }
    status = ProSelectionDwgtableGet(tableSels[0], &table);
    if (status != PRO_TK_NO_ERROR)
    {
        return;
    }

    status = ProDwgtableCellNoteGet(&table, column + 1, row + 1, &note);
    status = ProNoteTextStyleGet(&note, &textStyle);

    status = ProDwgtableColumnSizeGet(&table, table_segment, column, &cell_size);
    status = ProDtlnoteLineEnvelopeGet(&note, 0, envel);

    status = ProTextStyleWidthGet(textStyle, &width);
    status = ProTextStyleWidthSet(textStyle, width * cell_size / (envel[1][0] - envel[0][0]) * 0.92);//给个系数0.92再缩短点

    status = ProNoteTextStyleSet(&note, textStyle);
    status = ProTextStyleFree(&textStyle);
    status = ProMacroLoad(L"~ Command `ProCmdDwgRegenModel` ;~ Command `ProCmdWinActivate`;");
}

int user_initialize()
{
    ProError status;
    uiCmdCmdId IMI_TextWrapmenuID, IMI_TextAutoWidthmenuID;

    status = ProMenubarMenuAdd("IMI_TableTextWidthmenu", "IMI_TableTextWidthmenu", "About", PRO_B_TRUE, MSGFILE);

    status = ProCmdActionAdd("IMI_TextWrap_Act", (uiCmdCmdActFn)WrapText, uiProeImmediate, AccessDRW, PRO_B_TRUE, PRO_B_TRUE, &IMI_TextWrapmenuID);
    status = ProMenubarmenuPushbuttonAdd("IMI_TableTextWidthmenu", "IMI_TextWrapmenu", "IMI_TextWrapmenu", "IMI_TextWrapmenutips", NULL, PRO_B_TRUE, IMI_TextWrapmenuID, MSGFILE);

    status = ProCmdActionAdd("IMI_TextAutoWidth_Act", (uiCmdCmdActFn)SetAutoTextWidth, uiProeImmediate, AccessDRW, PRO_B_TRUE, PRO_B_TRUE, &IMI_TextAutoWidthmenuID);
    status = ProMenubarmenuPushbuttonAdd("IMI_TableTextWidthmenu", "IMI_TextAutoWidthmenu", "IMI_TextAutoWidthmenu", "IMI_TextAutoWidthmenutips", NULL, PRO_B_TRUE, IMI_TextAutoWidthmenuID, MSGFILE);
    return PRO_TK_NO_ERROR;
}

void user_terminate()
{
}
