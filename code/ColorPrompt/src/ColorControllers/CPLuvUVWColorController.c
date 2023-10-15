
#include "CPColorController.h"

#include "../CPColorPromptApplication.h"
#include "../CPDesign.h"
#include "../CPPreferences.h"
#include "../CPTranslations.h"
#include "Displays/CPColorWell1D.h"
#include "Displays/CPColorWell2D.h"
#include "CPLuvUVWColorController.h"

#include "NAApp.h"

struct CPLuvUVWColorController{
  CPColorController baseController;
  
  CPColorWell2D* colorWell2D;

  NASpace* channelSpace;
  NARadio* radioLuv;
  NARadio* radioUVW;
  NALabel* label0;
  NALabel* label1;
  NALabel* label2;
  NATextField* textField0;
  NATextField* textField1;
  NATextField* textField2;
  CPColorWell1D* colorWell1D0;
  CPColorWell1D* colorWell1D1;
  CPColorWell1D* colorWell1D2;

  CMLVec3 color;
};



typedef enum {
  Luv,
  UVW
} LuvUVWSelect;



NABool cmLuvUVWSelectionChanged(NAReaction reaction){
  CPLuvUVWColorController* con = (CPLuvUVWColorController*)reaction.controller;
  
  LuvUVWSelect luvuvwSelect = (LuvUVWSelect)naGetPreferencesEnum(cpPrefs[CPLuvUVWSelect]);
  CMLColorType oldColorType = (luvuvwSelect == Luv) ? CML_COLOR_Luv : CML_COLOR_UVW;
  CMLColorType newColorType = oldColorType;

  if(reaction.uiElement == con->radioLuv){
    naSetPreferencesEnum(cpPrefs[CPLuvUVWSelect], Luv);
    newColorType = CML_COLOR_Luv;
  }else if(reaction.uiElement == con->radioUVW){
    naSetPreferencesEnum(cpPrefs[CPLuvUVWSelect], UVW);
    newColorType = CML_COLOR_UVW;
  }

  CMLColorMachine* cm = cpGetCurrentColorMachine();
  CMLColorConverter converter = cmlGetColorConverter(newColorType, oldColorType);
  converter(cm, con->color, con->color, 1);

  cmSetColorControllerColorType(&(con->baseController), newColorType);
  cpSetCurrentColorController(&(con->baseController));
  cpUpdateColor();
  
  return NA_TRUE;
}



NABool cmLuvValueEdited(NAReaction reaction){
  CPLuvUVWColorController* con = (CPLuvUVWColorController*)reaction.controller;
  
  if(reaction.uiElement == con->textField0){
    con->color[0] = (float)naGetTextFieldDouble(con->textField0);
  }else if(reaction.uiElement == con->textField1){
    con->color[1] = (float)naGetTextFieldDouble(con->textField1);
  }else if(reaction.uiElement == con->textField2){
    con->color[2] = (float)naGetTextFieldDouble(con->textField2);
  }
  
  cpSetCurrentColorController(&(con->baseController));
  cpUpdateColor();
  
  return NA_TRUE;
}



CPLuvUVWColorController* cmAllocLuvUVWColorController(void){
  LuvUVWSelect luvuvwSelect = (LuvUVWSelect)naInitPreferencesEnum(cpPrefs[CPLuvUVWSelect], Luv);
  CMLColorType colorType = (luvuvwSelect == Luv) ? CML_COLOR_Luv : CML_COLOR_UVW;

  CPLuvUVWColorController* con = naAlloc(CPLuvUVWColorController);

  cmInitColorController(&(con->baseController), colorType);
  
  con->colorWell2D = cmAllocColorWell2D(&(con->baseController), 0);

  con->channelSpace = naNewSpace(naMakeSize(1, 1));
  con->radioLuv = naNewRadio(cpTranslate(CPColorSpaceLuv), radioSelectWidth);
  con->radioUVW = naNewRadio(cpTranslate(CPColorSpaceUVW), radioSelectWidth);
  naAddUIReaction(con->radioLuv, NA_UI_COMMAND_PRESSED, cmLuvUVWSelectionChanged, con);
  naAddUIReaction(con->radioUVW, NA_UI_COMMAND_PRESSED, cmLuvUVWSelectionChanged, con);

  con->label0 = cpNewColorComponentLabel("");
  con->label1 = cpNewColorComponentLabel("");
  con->label2 = cpNewColorComponentLabel("");
  con->textField0 = cpNewValueTextField(cmLuvValueEdited, con);
  con->textField1 = cpNewValueTextField(cmLuvValueEdited, con);
  con->textField2 = cpNewValueTextField(cmLuvValueEdited, con);
  con->colorWell1D0 = cmAllocColorWell1D(&(con->baseController), con->color, 0);
  con->colorWell1D1 = cmAllocColorWell1D(&(con->baseController), con->color, 1);
  con->colorWell1D2 = cmAllocColorWell1D(&(con->baseController), con->color, 2);

  naSetUIElementNextTabElement(con->textField0, con->textField1);
  naSetUIElementNextTabElement(con->textField1, con->textField2);
  naSetUIElementNextTabElement(con->textField2, con->textField0);

  cpBeginUILayout(con->channelSpace, naMakeBezel4Zero());
  cpAddUIPos(0, (int)((colorWell2DSize - (4 * 25. + radioChannelCenteringOffset)) / 2.)); // center the channels
  cpAddUIRowH(con->radioLuv, colorValueCondensedRowHeight, colorComponentWidth + colorComponentMarginH);
  cpAddUICol(con->radioUVW, 10);
  cpAddUIPos(0, radioChannelOffset);

  cpAddUIRow(con->label0, colorValueCondensedRowHeight);
  cpAddUICol(con->textField0, colorComponentMarginH);
  cpAddUIColV(cmGetColorWell1DUIElement(con->colorWell1D0), 10, colorWell1DOffset);
  cpAddUIRow(con->label1, colorValueCondensedRowHeight);
  cpAddUICol(con->textField1, colorComponentMarginH);
  cpAddUIColV(cmGetColorWell1DUIElement(con->colorWell1D1), 10, colorWell1DOffset);
  cpAddUIRow(con->label2, colorValueCondensedRowHeight);
  cpAddUICol(con->textField2, colorComponentMarginH);
  cpAddUIColV(cmGetColorWell1DUIElement(con->colorWell1D2), 10, colorWell1DOffset);
  cpEndUILayout();
  
  cpBeginUILayout(con->baseController.space, colorWellBezel);
  cpAddUIRow(cmGetColorWell2DUIElement(con->colorWell2D), 0);
  cpAddUICol(con->channelSpace, colorWell2DRightMargin);
  cpEndUILayout();

  return con;
}



void cmDeallocLuvUVWColorController(CPLuvUVWColorController* con){
  cmDeallocColorWell2D(con->colorWell2D);
  cmDeallocColorWell1D(con->colorWell1D0);
  cmDeallocColorWell1D(con->colorWell1D1);
  cmDeallocColorWell1D(con->colorWell1D2);
  cmClearColorController(&(con->baseController));
  naFree(con);
}



const void* cmGetLuvUVWColorControllerColorData(const CPLuvUVWColorController* con){
  return &(con->color);
}



void cmSetLuvUVWColorControllerColorData(CPLuvUVWColorController* con, const void* data){
  cmlCpy3(con->color, data);
}



void cmUpdateLuvUVWColorController(CPLuvUVWColorController* con){
  cpUpdateColorController(&(con->baseController));

  LuvUVWSelect luvuvwSelect = (LuvUVWSelect)naGetPreferencesEnum(cpPrefs[CPLuvUVWSelect]);
  CMLColorType colorType = (luvuvwSelect == Luv) ? CML_COLOR_Luv : CML_COLOR_UVW;
  
  naSetRadioState(con->radioLuv, luvuvwSelect == Luv);
  naSetRadioState(con->radioUVW, luvuvwSelect == UVW);
  
  if(luvuvwSelect == Luv){
    naSetLabelText(con->label0, cpTranslate(CPLuvColorChannelL));
    naSetLabelText(con->label1, cpTranslate(CPLuvColorChannelu));
    naSetLabelText(con->label2, cpTranslate(CPLuvColorChannelv));
    cmSetColorWell2DFixedIndex(con->colorWell2D, 0);
  }else if(luvuvwSelect == UVW){
    naSetLabelText(con->label0, cpTranslate(CPUVWColorChannelU));
    naSetLabelText(con->label1, cpTranslate(CPUVWColorChannelV));
    naSetLabelText(con->label2, cpTranslate(CPUVWColorChannelW));
    cmSetColorWell2DFixedIndex(con->colorWell2D, 2);
  }

  CMLColorMachine* cm = cpGetCurrentColorMachine();
  CMLColorType currentColorType = cpGetCurrentColorType();
  const float* currentColorData = cpGetCurrentColorData();
  CMLColorConverter converter = cmlGetColorConverter(colorType, currentColorType);
  converter(cm, con->color, currentColorData, 1);
  
  cpUpdateColorWell2D(con->colorWell2D);

  naSetTextFieldText(
    con->textField0,
    naAllocSprintf(NA_TRUE, "%3.03f", con->color[0]));
  naSetTextFieldText(
    con->textField1,
    naAllocSprintf(NA_TRUE, "%3.02f", con->color[1]));
  naSetTextFieldText(
    con->textField2,
    naAllocSprintf(NA_TRUE, "%3.02f", con->color[2]));

  cpUpdateColorWell1D(con->colorWell1D0);
  cpUpdateColorWell1D(con->colorWell1D1);
  cpUpdateColorWell1D(con->colorWell1D2);
}
