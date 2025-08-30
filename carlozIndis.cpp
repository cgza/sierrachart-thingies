// The top of every source code file must include this line
#include "sierrachart.h"

// For reference, refer to this page:
// https://www.sierrachart.com/index.php?page=doc/AdvancedCustomStudyInterfaceAndLanguage.php

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies.
SCDLLName("carlozIndis")

    SCSFExport scsf_HiVolCandles(SCStudyInterfaceRef sc) {
  SCSubgraphRef upTop = sc.Subgraph[0];
  SCSubgraphRef upBottom = sc.Subgraph[1];
  SCSubgraphRef downTop = sc.Subgraph[2];
  SCSubgraphRef downBottom = sc.Subgraph[3];

  SCInputRef per = sc.Input[0];
  SCInputRef multiplier = sc.Input[1];
  SCInputRef clearCurrentTrades = sc.Input[2];

  if (sc.SetDefaults) {
    sc.GraphName = "High Volume Candles";
    sc.StudyDescription = "Displays high volume candle as ranges";
    sc.AutoLoop = 1;
    sc.GraphRegion = 0;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    sc.ScaleRangeType = SCALE_SAMEASREGION;

    upTop.Name = "Hi Up-Volume Top";
    upTop.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
    upTop.PrimaryColor = RGB(0, 0, 255);
    upTop.DrawZeros = false;

    upBottom.Name = "Hi Up-Volume Bottom";
    upBottom.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
    upBottom.PrimaryColor = RGB(0, 0, 255);
    upBottom.DrawZeros = false;

    downTop.Name = "Hi Down-Volume Top";
    downTop.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
    downTop.PrimaryColor = RGB(255, 0, 0);
    downTop.DrawZeros = false;

    downBottom.Name = "Hi Down-Volume Bottom";
    downBottom.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
    downBottom.PrimaryColor = RGB(255, 0, 0);
    downBottom.DrawZeros = false;

    per.Name = "Volume SMA period";
    per.SetInt(10);

    multiplier.Name = "Multiplier of volume SMA to consider as high volume";
    multiplier.SetFloat(2);

    clearCurrentTrades.Name = "Clear Current Trades on new high Volume bar";
    clearCurrentTrades.SetYesNo(0);
  }

  // fill subgraphs on the first bar so that indicator can start
  if (sc.Index == 0) {
    upTop[0] = sc.High[0];
    downBottom[0] = sc.Low[0];
  }

  // make the last bar index persistent
  int &lastIndex = sc.GetPersistentInt(0);

  // if we have a new bar
  if (sc.Index != lastIndex) {
    // calculate average volume
    float avg = 0;
    for (int x = 2; x < 2 + per.GetInt(); x++) {
      avg += sc.Volume[sc.Index - x];
    }
    avg /= per.GetInt();

    // how does this bar's volume compare
    float ratio = sc.Volume[sc.Index - 1] / avg;
    if (ratio > multiplier.GetFloat()) {
      // if last bar was up and the previous bar didn't fire a new range
      // if (sc.Close[sc.Index - 1] > sc.Open[sc.Index - 1]) {
      // delta was negative (people absorbing at the bid)
      if (sc.BidVolume[sc.Index - 1] > sc.AskVolume[sc.Index - 1]) {
        if (upTop[sc.Index - 3] != 0) {
          // set prev subgraph value to zero
          upTop[sc.Index - 2] = 0;
          upBottom[sc.Index - 2] = 0;
          // create range
          upTop[sc.Index - 1] = sc.High[sc.Index - 1];   // sc.Close[sc.Index - 1];
          upBottom[sc.Index - 1] = sc.Low[sc.Index - 1]; // sc.Open[sc.Index - 1];
          // // forward project new range
          // for (int x = 0; x < 100; x++) {
          //   upTop[sc.Index + x] = upTop[sc.Index - 1];
          //   upBottom[sc.Index + x] = upBottom[sc.Index - 1];
          // }
          if (clearCurrentTrades.GetYesNo() == 1)
            sc.ClearCurrentTradedBidAskVolume();
        }
      }
      // last bar was down
      // else {
      // delta was positive (people absorbing at the ask)
      else if (sc.AskVolume[sc.Index - 1] > sc.BidVolume[sc.Index - 1]) {
        if (downBottom[sc.Index - 3] != 0) {
          // set prev subgraph value to zero
          downTop[sc.Index - 2] = 0;
          downBottom[sc.Index - 2] = 0;
          // create range
          downBottom[sc.Index - 1] = sc.Low[sc.Index - 1]; // sc.Close[sc.Index - 1];
          downTop[sc.Index - 1] = sc.High[sc.Index - 1];   // sc.Open[sc.Index - 1];
          // // forward project new range
          // for (int x = 0; x < 100; x++) {
          //   downTop[sc.Index + x] = downTop[sc.Index - 1];
          //   downBottom[sc.Index + x] = downBottom[sc.Index - 1];
          // }
          if (clearCurrentTrades.GetYesNo() == 1)
            sc.ClearCurrentTradedBidAskVolume();
        }
      }
    }
    // we have a new bar, so
    // copy previous bar's values into new bar
    upTop[sc.Index] = upTop[sc.Index - 1];
    upBottom[sc.Index] = upBottom[sc.Index - 1];
    downTop[sc.Index] = downTop[sc.Index - 1];
    downBottom[sc.Index] = downBottom[sc.Index - 1];

    // save current bar index
    lastIndex = sc.Index;
  }
}

SCSFExport scsf_DeltaVolumeBars(SCStudyInterfaceRef sc) {
  SCSubgraphRef volume = sc.Subgraph[0];
  SCSubgraphRef askDelta = sc.Subgraph[1];
  SCSubgraphRef bidDelta = sc.Subgraph[2];

  if (sc.SetDefaults) {
    sc.GraphName = "Volume & Delta Bars";
    sc.StudyDescription = "Displays volume bars and their delta overlayed";
    sc.AutoLoop = 1;
    sc.ValueFormat = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    // sc.DisplayStudyName = 0;
    // sc.DisplayStudyInputValues = 0;
    // sc.GlobalDisplayStudySubgraphsNameAndValue = 0;

    volume.Name = "Volume Bar";
    volume.DrawStyle = DRAWSTYLE_CANDLE_BODYCLOSE;
    volume.PrimaryColor = RGB(181, 208, 176);
    volume.SecondaryColor = RGB(221, 171, 164);
    volume.SecondaryColorUsed = 1;
    volume.DrawZeros = false;
    volume.AutoColoring = AUTOCOLOR_BASEGRAPH;

    askDelta.Name = "Ask Delta";
    askDelta.DrawStyle = DRAWSTYLE_CANDLE_BODYCLOSE;
    askDelta.PrimaryColor = RGB(255, 0, 0);
    askDelta.DrawZeros = false;

    bidDelta.Name = "Bid Delta";
    bidDelta.DrawStyle = DRAWSTYLE_CANDLE_BODYCLOSE;
    bidDelta.PrimaryColor = RGB(0, 128, 0);
    bidDelta.DrawZeros = false;
  }

  // auxiliary variables
  int i = sc.Index;
  int delta = sc.AskVolume[i] - sc.BidVolume[i];

  // copy volume to my subgraph and color it
  volume[i] = sc.Volume[i];
  volume.DataColor[i] = sc.Close[i] > sc.Open[i] ? volume.PrimaryColor : volume.SecondaryColor;

  // display the bar delta in the correspondig subgrapgh
  if (delta > 0) {
    askDelta[i] = delta;
    bidDelta[i] = 0;
  } else if (delta < 0) {
    bidDelta[i] = -delta;
    askDelta[i] = 0;
  } else {
    bidDelta[i] = 0;
    askDelta[i] = 0;
  }
}

SCSFExport scsf_SRlevels(SCStudyInterfaceRef sc) {
  SCSubgraphRef nUpT = sc.Subgraph[0];
  SCSubgraphRef nUpB = sc.Subgraph[1];
  SCSubgraphRef nDownT = sc.Subgraph[2];
  SCSubgraphRef nDownB = sc.Subgraph[3];

  SCSubgraphRef brkUpT = sc.Subgraph[4];
  SCSubgraphRef brkUpB = sc.Subgraph[5];
  SCSubgraphRef brkDownT = sc.Subgraph[6];
  SCSubgraphRef brkDownB = sc.Subgraph[7];

  SCSubgraphRef highsLows = sc.Subgraph[8];
  SCSubgraphRef retraceUp = sc.Subgraph[9];
  SCSubgraphRef retraceDown = sc.Subgraph[10];
  SCSubgraphRef nearestRetraceUp = sc.Subgraph[11];
  SCSubgraphRef nearestRetraceDown = sc.Subgraph[12];

  SCSubgraphRef uUpT = sc.Subgraph[13];
  SCSubgraphRef uUpB = sc.Subgraph[14];
  SCSubgraphRef uDownT = sc.Subgraph[15];
  SCSubgraphRef uDownB = sc.Subgraph[16];

  SCSubgraphRef upT = sc.Subgraph[17];
  SCSubgraphRef upB = sc.Subgraph[18];
  SCSubgraphRef downT = sc.Subgraph[19];
  SCSubgraphRef downB = sc.Subgraph[20];

  SCInputRef levelType = sc.Input[0];
  SCInputRef nBarsHL = sc.Input[1];
  SCInputRef retracePercent = sc.Input[2];

  if (sc.SetDefaults) {
    // SUBGRAPHS
    sc.GraphName = "Swings and S&R Zones";
    sc.StudyDescription = "Show S&R levels, Swing HiLos and Retracements";
    sc.AutoLoop = 1;
    sc.GraphRegion = 0;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    sc.ScaleRangeType = SCALE_SAMEASREGION;
    // set initial transparency
    sc.SetChartStudyTransparencyLevel(sc.ChartNumber, sc.StudyGraphInstanceID, 75);

    // inputs
    levelType.Name = "Create S&R Levels from";
    levelType.SetCustomInputStrings("OrderBlocks; Fair Value Gaps");
    levelType.SetCustomInputIndex(0);

    nBarsHL.Name = "Number of Bars to confirm new High || Low";
    nBarsHL.SetInt(12);

    retracePercent.Name = "Swing retracements % (0=disable)";
    retracePercent.SetFloat(61.8);

    // fills
    nDownT.Name = "Level-Nearest Resistance: top";
    nDownT.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
    nDownT.PrimaryColor = RGB(255, 0, 128);
    nDownT.DrawZeros = false;
    nDownB.Name = "Level-Nearest Resistance: bottom";
    nDownB.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
    nDownB.PrimaryColor = RGB(255, 0, 128);
    nDownB.DrawZeros = false;

    nUpT.Name = "Level-Nearest Support: top";
    nUpT.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
    nUpT.PrimaryColor = RGB(0, 0, 255);
    nUpT.DrawZeros = false;
    nUpB.Name = "Level-Nearest Support: bottom";
    nUpB.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
    nUpB.PrimaryColor = RGB(0, 0, 255);
    nUpT.DrawZeros = false;

    brkDownT.Name = "Level-Broken Resistance: top";
    brkDownT.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
    brkDownT.PrimaryColor = RGB(255, 160, 210);
    brkDownT.DrawZeros = false;
    brkDownB.Name = "Level-Broken Resistance: bottom";
    brkDownB.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
    brkDownB.PrimaryColor = RGB(255, 160, 210);
    brkDownB.DrawZeros = false;

    brkUpT.Name = "Level-Broken Support: top";
    brkUpT.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
    brkUpT.PrimaryColor = RGB(170, 170, 255);
    brkUpT.DrawZeros = false;
    brkUpB.Name = "Level-Broken Support: bottom";
    brkUpB.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
    brkUpB.PrimaryColor = RGB(170, 170, 255);
    brkUpT.DrawZeros = false;

    highsLows.Name = "Highs & Lows Zigzag";
    highsLows.DrawStyle = DRAWSTYLE_LINE;
    highsLows.PrimaryColor = RGB(0, 0, 255);
    highsLows.LineStyle = LINESTYLE_SOLID;
    highsLows.LineWidth = 1;
    highsLows.DrawZeros = false;

    retraceUp.Name = "UpSwing Retracement";
    retraceUp.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    retraceUp.PrimaryColor = RGB(0, 0, 255);
    retraceUp.LineStyle = LINESTYLE_SOLID;
    retraceUp.LineWidth = 1;
    retraceUp.DrawZeros = false;

    retraceDown.Name = "DownSwing Retracement";
    retraceDown.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    retraceDown.PrimaryColor = RGB(255, 0, 0);
    retraceDown.LineStyle = LINESTYLE_SOLID;
    retraceDown.LineWidth = 1;
    retraceDown.DrawZeros = false;

    nearestRetraceUp.Name = "Nearest UpSwing Retracement";
    nearestRetraceUp.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    nearestRetraceUp.PrimaryColor = RGB(0, 0, 255);
    nearestRetraceUp.LineStyle = LINESTYLE_SOLID;
    nearestRetraceUp.LineWidth = 3;
    nearestRetraceUp.DrawZeros = false;

    nearestRetraceDown.Name = "Nearest DownSwing Retracement";
    nearestRetraceDown.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    nearestRetraceDown.PrimaryColor = RGB(255, 0, 0);
    nearestRetraceDown.LineStyle = LINESTYLE_SOLID;
    nearestRetraceDown.LineWidth = 3;
    nearestRetraceDown.DrawZeros = false;

    uDownT.Name = "Bar-Untested Resistance: top";
    uDownT.DrawStyle = DRAWSTYLE_HIDDEN;
    uDownT.PrimaryColor = RGB(255, 0, 128);
    uDownT.DrawZeros = false;
    uDownB.Name = "Bar-Untested Resistance: bottom";
    uDownB.DrawStyle = DRAWSTYLE_HIDDEN;
    uDownB.PrimaryColor = RGB(255, 0, 128);
    uDownB.DrawZeros = false;

    uUpT.Name = "Bar-Untested Support: top";
    uUpT.DrawStyle = DRAWSTYLE_HIDDEN;
    uUpT.PrimaryColor = RGB(0, 0, 255);
    uUpT.DrawZeros = false;
    uUpB.Name = "Bar-Untested Support: bottom";
    uUpB.DrawStyle = DRAWSTYLE_HIDDEN;
    uUpB.PrimaryColor = RGB(0, 0, 255);
    uUpT.DrawZeros = false;

    downT.Name = "Bar-Filled Resistance: top";
    downT.DrawStyle = DRAWSTYLE_HIDDEN;
    downT.PrimaryColor = RGB(255, 160, 210);
    downT.DrawZeros = false;
    downB.Name = "Bar-Filled Resistance: bottom";
    downB.DrawStyle = DRAWSTYLE_HIDDEN;
    downB.PrimaryColor = RGB(255, 160, 210);
    downB.DrawZeros = false;

    upT.Name = "Bar-Filled Support: top";
    upT.DrawStyle = DRAWSTYLE_HIDDEN;
    upT.PrimaryColor = RGB(170, 170, 255);
    upT.DrawZeros = false;
    upB.Name = "Bar-Filled Support: bottom";
    upB.DrawStyle = DRAWSTYLE_HIDDEN;
    upB.PrimaryColor = RGB(170, 170, 255);
    upT.DrawZeros = false;

    return;
  }

  //**PERSISTENT
  int &lastIndex = sc.GetPersistentInt(0);
  int &lastHighIndex = sc.GetPersistentInt(1);
  int &lastLowIndex = sc.GetPersistentInt(2);
  int &lookFor = sc.GetPersistentInt(3);
  int &alreadyGapping = sc.GetPersistentInt(4);
  int &lastHiOB = sc.GetPersistentInt(5);
  int &lastLoOB = sc.GetPersistentInt(6);
  float &swingHigh = sc.GetPersistentFloat(0);
  float &swingLow = sc.GetPersistentFloat(1);
  // variables
  float lastHigh = sc.High[lastHighIndex], lastLow = sc.Low[lastLowIndex];

  //*** VALORES INICIALES DE BARRA NUEVA
  if (sc.Index != lastIndex) {
    uUpT[sc.Index] = 0;
    uUpB[sc.Index] = 0;
    uDownT[sc.Index] = 0;
    uDownB[sc.Index] = 0;
    nUpB[sc.Index] = nUpB[sc.Index - 1];
    nUpT[sc.Index] = nUpT[sc.Index - 1];
    nDownB[sc.Index] = nDownB[sc.Index - 1];
    nDownT[sc.Index] = nDownT[sc.Index - 1];
    brkUpB[sc.Index] = brkUpB[sc.Index - 1];
    brkUpT[sc.Index] = brkUpT[sc.Index - 1];
    brkDownB[sc.Index] = brkDownB[sc.Index - 1];
    brkDownT[sc.Index] = brkDownT[sc.Index - 1];
    highsLows[sc.Index] = 0;
    retraceUp[sc.Index] = retraceUp[sc.Index - 1];
    retraceDown[sc.Index] = retraceDown[sc.Index - 1];
    nearestRetraceUp[sc.Index] = nearestRetraceUp[sc.Index - 1];
    nearestRetraceDown[sc.Index] = nearestRetraceDown[sc.Index - 1];
  }

  // valores en las primeras barras del chart para calcular a partir de aquí
  if (sc.Index < 2 * nBarsHL.GetInt() + 2) {
    if (sc.High[sc.Index] > lastHigh) {
      lastHigh = sc.High[sc.Index];
      lastHighIndex = sc.Index;
      nDownT[sc.Index] = lastHigh;
    }
    if (sc.Low[sc.Index] < lastLow) {
      lastLow = sc.Low[sc.Index];
      lastLowIndex = sc.Index;
    }
    return;
  }

  //*** LEVEL RETESTS
  // si se rompe el nearest untested level, se transforma en el último broken
  // level, se busca si se ha roto algún otro nivel más, se borran de untested y
  // se busca un nuevo nearest level.
  if (sc.Low[sc.Index] < nUpB[sc.Index] && nUpB[sc.Index] != 0) {
    for (int i = sc.Index - 1; i >= 0; i--) {
      // si he llegado al final del loop, aseguro que nearest es cero
      if (i == 0) {
        nUpB[sc.Index] = 0;
        nUpT[sc.Index] = 0;
        break;
      }
      if (uUpB[i] == 0)
        continue;
      // si este untested lo he reventado, lo muevo a filled y lo elimino de
      // untested
      if (uUpB[i] > sc.Low[sc.Index]) {
        for (int x = i; x <= sc.Index; x++) {
          brkUpB[x] = uUpB[i];
          brkUpT[x] = uUpT[i];
        }
        upB[i] = uUpB[i];
        upT[i] = uUpT[i];
        uUpB[i] = 0;
        uUpT[i] = 0;
        continue;
      }
      // he encontrado el nearest más cercano
      if (uUpB[i] <= sc.Low[sc.Index]) {
        for (int x = i; x <= sc.Index; x++) {
          nUpB[x] = uUpB[i];
          nUpT[x] = uUpT[i];
        }
        break;
      }
    }
  }
  if (sc.High[sc.Index] > nDownT[sc.Index] && nDownT[sc.Index] != 0) {
    for (int i = sc.Index - 1; i >= 0; i--) {
      // si he llegado al final del loop, aseguro que nearest es cero
      if (i == 0) {
        nDownT[sc.Index] = 0;
        nDownB[sc.Index] = 0;
        break;
      }
      if (uDownT[i] == 0)
        continue;
      // si este untested lo he reventado, lo muevo a filled y lo elimino de
      // untested
      if (uDownT[i] < sc.High[sc.Index]) {
        for (int x = i; x <= sc.Index; x++) {
          brkDownT[x] = uDownT[i];
          brkDownB[x] = uDownB[i];
        }
        downT[i] = uDownT[i];
        downB[i] = uDownB[i];
        uDownT[i] = 0;
        uDownB[i] = 0;
        continue;
      }
      // he encontrado el nearest más cercano
      if (uDownT[i] >= sc.High[sc.Index]) {
        for (int x = i; x <= sc.Index; x++) {
          nDownT[x] = uDownT[i];
          nDownB[x] = uDownB[i];
        }
        break;
      }
    }
  }

  //*** SI LA BARRA HA CERRADO, BUSCO SI SE HAN CREADO NUEVOS NIVELES O NUEVO
  // HIGH || LOW
  if (sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED) {
    //**NUEVO HIGH || LOW
    int mmIndex = sc.Index - nBarsHL.GetInt();
    int sinceHigh =
        sc.NumberOfBarsSinceHighestValue(sc.BaseDataIn[SC_HIGH], 2 * nBarsHL.GetInt() + 1);
    int sinceLow = sc.NumberOfBarsSinceLowestValue(sc.BaseDataIn[SC_LOW], 2 * nBarsHL.GetInt() + 1);

    // if there is a new high || low nBarsHL ago
    if (sinceHigh == nBarsHL.GetInt() && (lookFor >= 0 || sc.High[mmIndex] > lastHigh)) {
      // si este high supera al anterior antes de haber
      // confirmado low, borro el anterior y lo sustituyo por éste
      if (lookFor <= 0 && sc.High[mmIndex] > lastHigh) {
        highsLows[lastHighIndex] = 0;

        // copiar hasta el nuevo high el down-retracement anterior al high que
        // ya no vale
        if (retracePercent.GetFloat() > 0) {
          for (int x = lastHighIndex - 1; x <= sc.Index - nBarsHL.GetInt(); x++) {
            retraceDown[x] = retraceDown[lastHighIndex - 2];
          }
          // corregir el up-retracement de este swing
          float val = sc.Low[lastLowIndex] + (sc.High[mmIndex] - sc.Low[lastLowIndex]) *
                                                 (100 - retracePercent.GetFloat()) / 100;
          for (int x = lastLowIndex; x <= sc.Index; x++) {
            retraceUp[x] = val;
          }
        }
      }
      // confirmo nuevo high
      highsLows[mmIndex] = sc.High[mmIndex];
      lastHigh = sc.High[mmIndex];
      lastHighIndex = mmIndex;

      // calcular nuevo retracement down
      if (retracePercent.GetFloat() > 0) {
        swingLow = sc.Low[mmIndex];
        retraceDown[mmIndex - 1] = 0;
        swingHigh = 0;
        for (int x = mmIndex + 1; x <= sc.Index; x++)
          swingLow = sc.Low[x] < swingLow ? sc.Low[x] : swingLow;
        for (int x = mmIndex; x <= sc.Index; x++)
          retraceDown[x] =
              lastHigh - (lastHigh - swingLow) * (100 - retracePercent.GetFloat()) / 100;
      }
      // busco siguiente Entry OnRetracement
      nearestRetraceDown[sc.Index] = 0;
      double aux = lastHigh;
      for (int x = lastHighIndex; x > 0; x--) {
        aux = sc.High[x] > aux ? sc.High[x] : aux;
        if (retraceDown[x] > aux && retraceDown[x + 1] == 0 && retraceDown[x] != 0) {
          nearestRetraceDown[sc.Index] = retraceDown[x];
          if (nearestRetraceDown[sc.Index] != nearestRetraceDown[sc.Index - 1])
            nearestRetraceDown[sc.Index - 1] = 0;
          break;
        }
      }
      // save flag to now look for new low
      lookFor = -1;
      // reset flag to reset current trades only if price has not ALREADY broken
      // the last low while confirming the new high
      // if(sc.GetLowest(sc.BaseDataIn[SC_LOW], nBarsHL.GetInt() ) >= lastLow)
      // lastHiLoBroken = 0;
    } else if (sinceLow == nBarsHL.GetInt() && (lookFor <= 0 || sc.Low[mmIndex] < lastLow)) //)
    {
      if (lookFor >= 0 && sc.Low[mmIndex] < lastLow) {
        highsLows[lastLowIndex] = 0;

        // copiar hasta el nuevo high el down-retracement anterior al high que
        // ya no vale
        if (retracePercent.GetFloat() > 0) {
          for (int x = lastLowIndex - 1; x <= sc.Index - nBarsHL.GetInt(); x++) {
            retraceUp[x] = retraceUp[lastLowIndex - 2];
          }
          // corregir el down-retracement de este swing
          float val = sc.High[lastHighIndex] - (sc.High[lastHighIndex] - sc.Low[mmIndex]) *
                                                   (100 - retracePercent.GetFloat()) / 100;
          for (int x = lastHighIndex; x <= sc.Index; x++) {
            retraceDown[x] = val;
          }
        }
      }
      // confirmo nuevo low
      highsLows[mmIndex] = sc.Low[mmIndex];
      lastLow = sc.Low[mmIndex];
      lastLowIndex = mmIndex;

      // calcular nuevo retracement up
      if (retracePercent.GetFloat() > 0) {
        swingHigh = sc.High[mmIndex];
        retraceUp[mmIndex - 1] = 0;
        swingLow = 0;
        for (int x = mmIndex + 1; x <= sc.Index; x++)
          swingHigh = sc.High[x] > swingHigh ? sc.High[x] : swingHigh;
        for (int x = mmIndex; x <= sc.Index; x++)
          retraceUp[x] = lastLow + (swingHigh - lastLow) * (100 - retracePercent.GetFloat()) / 100;
      }

      // busco siguiente Entry OnRetracement
      nearestRetraceUp[sc.Index] = 0;
      double aux = lastLow;
      for (int x = lastLowIndex; x > 0; x--) {
        aux = sc.Low[x] < aux ? sc.Low[x] : aux;
        if (retraceUp[x] < aux && retraceUp[x + 1] == 0 && retraceUp[x] != 0) {
          nearestRetraceUp[sc.Index] = retraceUp[x];
          if (nearestRetraceUp[sc.Index] != nearestRetraceUp[sc.Index - 1])
            nearestRetraceUp[sc.Index - 1] = 0;
          break;
        }
      }

      // save flag to now look for new high
      lookFor = 1;
      // reset flag to reset current trades only if price has not ALREADY broken
      // the last high while confirming the new low
      // if(sc.GetHighest(sc.BaseDataIn[SC_HIGH], nBarsHL.GetInt() ) <=
      // lastHigh) lastHiLoBroken = 0;
    }

    // compruebo que no se hayan rebasado los últimos highs/lows sin haber
    // confirmado primero el opuesto. Si se han rebasado, los anulo y corrijo
    // retracements
    if (sc.High[sc.Index] > lastHigh && lastHighIndex > lastLowIndex && lastLowIndex > 0 &&
        sc.NumberOfBarsSinceLowestValue(sc.BaseDataIn[SC_LOW], sc.Index - lastHighIndex) >
            nBarsHL.GetInt()) {
      // elimino el último high
      highsLows[lastHighIndex] = 0;
      // corrijo down-retracement prolongando el anterior al high que estamos
      // borrando
      for (int x = lastHighIndex - 1; x <= sc.Index; x++) {
        retraceDown[x] = retraceDown[lastHighIndex - 2];
      }
      // corrijo los swings
      swingLow = 0;
      swingHigh = sc.High[sc.Index - sc.NumberOfBarsSinceHighestValue(sc.BaseDataIn[SC_HIGH],
                                                                      sc.Index - lastHighIndex)];
      for (int x = lastLowIndex; x <= sc.Index; x++)
        retraceUp[x] = lastLow + (swingHigh - lastLow) * (100 - retracePercent.GetFloat()) / 100;
      // flag para seguir buscando un high
      lookFor = 1;
    }
    if (sc.Low[sc.Index] < lastLow && lastLowIndex > lastHighIndex && lastHighIndex > 0 &&
        sc.NumberOfBarsSinceHighestValue(sc.BaseDataIn[SC_HIGH], sc.Index - lastLowIndex) >
            nBarsHL.GetInt()) {
      // elimino el último low
      highsLows[lastLowIndex] = 0;
      // corrijo up-retracement prolongando el anterior al low que estamos
      // borrando
      for (int x = lastLowIndex - 1; x <= sc.Index; x++) {
        retraceUp[x] = retraceUp[lastLowIndex - 2];
      }
      // corrijo los swings
      swingHigh = 0;
      swingLow = sc.Low[sc.Index - sc.NumberOfBarsSinceLowestValue(sc.BaseDataIn[SC_LOW],
                                                                   sc.Index - lastLowIndex)];
      for (int x = lastHighIndex; x <= sc.Index; x++)
        retraceDown[x] = lastHigh - (lastHigh - swingLow) * (100 - retracePercent.GetFloat()) / 100;
      // flag para seguir buscando un low
      lookFor = -1;
    }

    //**LEVEL FROM FVGs
    if (levelType.GetIndex() == 1) {
      // static int alreadyGapping = 0;
      //  VEO SI HAY GAPUP
      if (sc.Low[sc.Index] > sc.High[sc.Index - 2]) {
        // coloreo la barra/candle
        if (alreadyGapping == 0) {
          uUpT[sc.Index - 1] = sc.Low[sc.Index];
          uUpB[sc.Index - 1] = sc.High[sc.Index - 2];
          nUpT[sc.Index] = uUpT[sc.Index - 1];
          nUpB[sc.Index] = uUpB[sc.Index - 1];
        }
        alreadyGapping = 1;
      } else
        // VEO SI HAY GAPDOWN
        if (sc.High[sc.Index] < sc.Low[sc.Index - 2]) {
          // coloreo la barra/candle
          if (alreadyGapping == 0) {
            uDownT[sc.Index - 1] = sc.Low[sc.Index - 2];
            uDownB[sc.Index - 1] = sc.High[sc.Index];
            nDownT[sc.Index] = uDownT[sc.Index - 1];
            nDownB[sc.Index] = uDownB[sc.Index - 1];
          }
          alreadyGapping = 1;
        } else {
          alreadyGapping = 0;
        }
    }
    //** LEVEL FROM ORDERBLOCKS
    else {
      // tengo un bearish OB si el precio ha roto el último low
      if (sc.Low[sc.Index] < lastLow && lastHighIndex != lastHiOB) {
        // si el último high se ha roto antes de romper este low, espero
        // a que se confirme el nuevo high para marcar el OB
        bool hiBroken = false;
        for (int x = lastHighIndex; x <= sc.Index; x++) {
          if (sc.High[x] > lastHigh) {
            hiBroken = true;
            break;
          }
        }
        if (!hiBroken) {
          for (int x = lastHighIndex; x > 0; x--) // antes desde lastHighIndex a secas
          {
            // el OB es la última bullish candle antes del high que nos ha
            // llevado a romper el low
            if (sc.Close[x] > sc.High[x - 1]) // sc.Open[x])
            {
              // fabrico el nuevo untested level en la barra del OB
              uDownT[x] = sc.High[x];
              uDownB[x] = sc.Open[x];
              // si el nuevo level está más cerca que el nearest, lo convierto
              // en el nearest
              if (uDownT[x] < nDownT[sc.Index] || nDownT[sc.Index] == 0) {
                for (int z = x; z <= sc.Index; z++) {
                  nDownT[z] = uDownT[x];
                  nDownB[z] = uDownB[x];
                }
              }
              break;
            }
          }
          // guardo el index de este high que ha provocado un OB
          lastHiOB = lastHighIndex;
        }
      }
      // tengo un bullish OB si el precio ha roto el último high
      if (sc.High[sc.Index] > lastHigh && lastLowIndex != lastLoOB) {
        // si el último low se ha roto antes de romper este high, espero
        // a que se confirme el nuevo low para marcar el OB
        bool loBroken = false;
        for (int x = lastLowIndex; x <= sc.Index; x++) {
          if (sc.Low[x] < lastLow) {
            loBroken = true;
            break;
          }
        }
        if (!loBroken) {
          for (int x = lastLowIndex; x > 0; x--) // antes desde lastLowIndex a secas
          {
            // el OB es la última bearish candle antes del low que nos ha
            // llevado a romper el high
            if (sc.Close[x] < sc.Low[x - 1]) // sc.Open[x]) //sc.Close[x-1])//
            {
              // fabrico el nuevo untested level en la barra del OB
              uUpT[x] = sc.Open[x];
              uUpB[x] = sc.Low[x];
              // si el nuevo level está más cerca que el nearest, lo convierto
              // en el nearest
              if (uUpB[x] > nUpB[sc.Index]) {
                for (int z = x; z <= sc.Index; z++) {
                  nUpT[z] = uUpT[x];
                  nUpB[z] = uUpB[x];
                }
              }
              break;
            }
          }
          // guardo el index de este high que ha provocado un OB
          lastLoOB = lastLowIndex;
        }
      }
    }
  }

  //*** ACTUALIZACIÓN DE RETRACEMENTS
  if (retracePercent.GetFloat() > 0) {
    if (swingHigh != 0 && sc.High[sc.Index] > swingHigh) {
      swingHigh = sc.High[sc.Index];
      for (int x = lastLowIndex; x <= sc.Index; x++)
        retraceUp[x] = lastLow + (swingHigh - lastLow) * (100 - retracePercent.GetFloat()) / 100;
    }
    if (swingLow != 0 && sc.Low[sc.Index] < swingLow) {
      swingLow = sc.Low[sc.Index];
      for (int x = lastHighIndex; x <= sc.Index; x++)
        retraceDown[x] = lastHigh - (lastHigh - swingLow) * (100 - retracePercent.GetFloat()) / 100;
    }
  }

  // flag última barra procesada
  lastIndex = sc.Index;
}

int SecondsPerDay(int useSecondTimes, int start1, int end1) {
  int secsPerDay = useSecondTimes == 1 ? 60 * 60 * 24 : end1 - start1 + 1;

  secsPerDay = secsPerDay > 1 ? secsPerDay : 60 * 60 * 24;
  return secsPerDay;
}

SCSFExport scsf_TimeframeLooper(SCStudyInterfaceRef sc) {
  SCInputRef tfDownButtonName = sc.Input[0];
  SCInputRef tfUpButtonName = sc.Input[1];
  const int TFstart = 10;
  const int TFamt = 10;
  // here timeframe inputs: see defaults
  SCInputRef setDaysToLoad = sc.Input[TFstart + TFamt];
  SCInputRef barsInChart = sc.Input[TFstart + TFamt + 1];
  const int toLoadStart = TFstart + TFamt + 2;
  const int toLoadAmt = 6;
  // here days to load inputs: see defaults
  const int otherParamsStart = toLoadStart + 10;
  const int otherParamsAmt = 5;
  SCInputRef gotoHistorical = sc.Input[otherParamsStart];
  SCInputRef useSpacing = sc.Input[otherParamsStart + 1];
  SCInputRef minThreshold = sc.Input[otherParamsStart + 2];
  SCInputRef maxThreshold = sc.Input[otherParamsStart + 3];
  SCInputRef vbpID = sc.Input[otherParamsStart + 4];
  const int hideStudiesStart = otherParamsStart + otherParamsAmt;
  const int hideStudiesAmt = 5;

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "Timeframe Looper";
    sc.StudyDescription = "Change chart timeframe automatically depending on "
                          "bars spacing || up/down arrow keys";
    sc.AutoLoop = 0;
    sc.GraphRegion = 0;
    sc.UpdateAlways = 1;
    sc.ReceiveKeyboardKeyEvents = 1;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;

    tfDownButtonName.Name = "TimeFrame DOWN Button Name";
    tfDownButtonName.SetString("<<TF");

    tfUpButtonName.Name = "TimeFrame UP Button Name";
    tfUpButtonName.SetString("TF>>");

    sc.Input[TFstart].Name = "Lowest Intraday Timeframe (sec)";
    sc.Input[TFstart].SetInt(1);

    sc.Input[TFstart + 1].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 1].SetInt(10);

    sc.Input[TFstart + 2].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 2].SetInt(30);

    sc.Input[TFstart + 3].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 3].SetInt(60);

    sc.Input[TFstart + 4].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 4].SetInt(300);

    sc.Input[TFstart + 5].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 5].SetInt(900);

    sc.Input[TFstart + 6].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 6].SetInt(1800);

    sc.Input[TFstart + 7].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 7].SetInt(3600);

    sc.Input[TFstart + 8].Name = "Next Timeframe (sec)";
    sc.Input[TFstart + 8].SetInt(0);

    sc.Input[TFstart + 9].Name = "Highest Intraday Timeframe (sec)";
    sc.Input[TFstart + 9].SetInt(86400);

    setDaysToLoad.Name = "Automatically set days to load";
    setDaysToLoad.SetCustomInputStrings(
        "Disabled;Based on max number of bars in chart;Customize depending on "
        "Timeframe");
    setDaysToLoad.SetCustomInputIndex(1);

    barsInChart.Name = "Max Number of bars in chart";
    barsInChart.SetInt(1000);

    sc.Input[toLoadStart].Name = "Days To Load-1min or below";
    sc.Input[toLoadStart].SetInt(1);
    sc.Input[toLoadStart].SetIntLimits(1, 10000);

    sc.Input[toLoadStart + 1].Name = "Days To Load-5min or below";
    sc.Input[toLoadStart + 1].SetInt(4);
    sc.Input[toLoadStart + 1].SetIntLimits(1, 10000);

    sc.Input[toLoadStart + 2].Name = "Days To Load-30min or below";
    sc.Input[toLoadStart + 2].SetInt(14);
    sc.Input[toLoadStart + 2].SetIntLimits(1, 10000);

    sc.Input[toLoadStart + 3].Name = "Days To Load-1H(65m) or below";
    sc.Input[toLoadStart + 3].SetInt(31);
    sc.Input[toLoadStart + 3].SetIntLimits(1, 10000);

    sc.Input[toLoadStart + 4].Name = "Days To Load-Above 1H(65m)";
    sc.Input[toLoadStart + 4].SetInt(92);
    sc.Input[toLoadStart + 4].SetIntLimits(1, 10000);

    sc.Input[toLoadStart + 5].Name = "Days To Load-Historical";
    sc.Input[toLoadStart + 5].SetInt(5000);
    sc.Input[toLoadStart + 5].SetIntLimits(1, 10000);

    gotoHistorical.Name = "Go-To Historical charts after intraday list";
    gotoHistorical.SetYesNo(0);

    useSpacing.Name = "Use chartbars spacing for timeframe looping";
    useSpacing.SetYesNo(0);

    minThreshold.Name = "Chartbars spacing to change timeframe upwards";
    minThreshold.SetInt(4);

    maxThreshold.Name = "Chartbars spacing to change timeframe downwards";
    maxThreshold.SetInt(20);

    vbpID.Name = "VbP ID to Set \"Separate Evening Sess\" param";
    vbpID.SetStudyID(0);

    for (int x = hideStudiesStart; x < hideStudiesStart + hideStudiesAmt * 2; x = x + 2) {
      sc.Input[x].Name.Format("Hide Study %i", (x - hideStudiesStart) / 2 + 1);
      sc.Input[x].SetStudyID(0);

      sc.Input[x + 1].Name = "From TF(secs) || above";
      sc.Input[x + 1].SetInt(3900);
    }

    return;
  }

  if (sc.IsFullRecalculation) {
    // set custom study buttons
    sc.ReceivePointerEvents = ACS_RECEIVE_POINTER_EVENTS_ALWAYS;
    sc.SetCustomStudyControlBarButtonText(1, tfDownButtonName.GetString());
    sc.SetCustomStudyControlBarButtonHoverText(1, "Decrease Chart Timeframe");
    sc.SetCustomStudyControlBarButtonText(2, tfUpButtonName.GetString());
    sc.SetCustomStudyControlBarButtonHoverText(2, "Increase Chart Timeframe");
  }

  int spacing = sc.ChartBarSpacing;
  int nextTF = 0;

  // INCREASE TIMEFRAME
  if ((useSpacing.GetYesNo() == 1 && spacing < minThreshold.GetInt()) ||
      sc.GetCustomStudyControlBarButtonEnableState(2) == 1) {
    // get period parameters
    n_ACSIL::s_BarPeriod BarPeriod;
    sc.GetBarPeriodParameters(BarPeriod);

    // if intraday, go to next higher timeframe
    if (BarPeriod.ChartDataType == INTRADAY_DATA) {
      int actualTF = BarPeriod.IntradayChartBarPeriodParameter1;

      // get seconds per day to check the day is divisible by the new timeframe
      int secsPerDay = SecondsPerDay(sc.UseSecondStartEndTimes, sc.StartTime1, sc.EndTime1);
      // loop through the timeframe list to find the next timeframe
      for (int i = TFstart; i < TFstart + TFamt; i++) {
        if (sc.Input[i].GetInt() <= actualTF)
          continue;
        // the day must be evenly divisible by the new timeframe
        if (secsPerDay % sc.Input[i].GetInt() != 0)
          continue;

        nextTF = sc.Input[i].GetInt();
        BarPeriod.IntradayChartBarPeriodParameter1 = nextTF;
        break;
      }

      // if we were already at the the end of the intraday timeframes, go to
      // daily
      if (nextTF == 0 && gotoHistorical.GetYesNo() == 1) {
        BarPeriod.ChartDataType = DAILY_DATA;
        BarPeriod.HistoricalChartBarPeriodType =
            actualTF == 86400 ? HISTORICAL_CHART_PERIOD_WEEKLY : HISTORICAL_CHART_PERIOD_DAYS;
        BarPeriod.HistoricalChartDaysPerBar = 1;

        nextTF = -1; // -1 means daily chart || higher

        // explicitly set days to load, so that current value will not be copied
        // (strangely)
        sc.DaysToLoadInChart = sc.Input[toLoadStart + toLoadAmt - 1].GetInt();
      }
    }
    // if chart is daily, then go to weekly, monthly....
    else {
      nextTF = -1; // -1 means daily chart || higher
      if (BarPeriod.HistoricalChartBarPeriodType < 5)
        BarPeriod.HistoricalChartBarPeriodType = static_cast<HistoricalChartBarPeriodEnum>(
            static_cast<int>(BarPeriod.HistoricalChartBarPeriodType) + 1);
      else
        nextTF = 0; // if already at highest TF possible do nothing
    }

    // set the new timeframe
    if (nextTF != 0) {
      sc.SetBarPeriodParameters(BarPeriod);
      if (useSpacing.GetYesNo() == 1)
        sc.ChartBarSpacing =
            minThreshold.GetInt() + (int)((maxThreshold.GetInt() - minThreshold.GetInt()) * 0.2);
    }

    // deactivate custom button
    sc.SetCustomStudyControlBarButtonEnable(2, 0);
  }

  // DECREASE TIMEFRAME
  if ((useSpacing.GetYesNo() == 1 && spacing > maxThreshold.GetInt()) ||
      sc.GetCustomStudyControlBarButtonEnableState(1) == 1) {
    n_ACSIL::s_BarPeriod BarPeriod;
    sc.GetBarPeriodParameters(BarPeriod);

    // if chart is above daily, then go to monthly, weekly, daily....or if daily
    // then change to the highest intraday
    if (BarPeriod.ChartDataType == DAILY_DATA) {
      nextTF = -1; // -1 means daily chart || higher
      HistoricalChartBarPeriodEnum thresholdToIntraday = static_cast<HistoricalChartBarPeriodEnum>(
          sc.Input[TFstart + TFamt - 1].GetInt() == 86400 && gotoHistorical.GetYesNo() == 1 ? 2
                                                                                            : 1);
      if (BarPeriod.HistoricalChartBarPeriodType > thresholdToIntraday)
        BarPeriod.HistoricalChartBarPeriodType = static_cast<HistoricalChartBarPeriodEnum>(
            static_cast<int>(BarPeriod.HistoricalChartBarPeriodType) - 1);
      else if (gotoHistorical.GetYesNo() == 1) {
        BarPeriod.ChartDataType = INTRADAY_DATA;
        // get seconds per day to check the day is divisible by the new
        // timeframe
        int secsPerDay = SecondsPerDay(sc.UseSecondStartEndTimes, sc.StartTime1, sc.EndTime1);
        for (int x = TFstart + TFamt - 1; x >= TFstart; x--) {
          if (sc.Input[x].GetInt() != 0 && secsPerDay % sc.Input[x].GetInt() == 0) {
            nextTF = sc.Input[x].GetInt();
            break;
          }
        }
        BarPeriod.IntradayChartBarPeriodParameter1 = nextTF;

        // explicitly set days to load, so that current value will not be copied
        // (strangely)
        sc.DaysToLoadInChart = sc.Input[toLoadStart + toLoadAmt - 2].GetInt();
      }
    }
    // if intraday, go to next lower timeframe
    else {
      int actualTF = BarPeriod.IntradayChartBarPeriodParameter1;
      // get seconds per day to check the day is divisible by the new
      // timeframe
      int secsPerDay = SecondsPerDay(sc.UseSecondStartEndTimes, sc.StartTime1, sc.EndTime1);

      // loop through the timeframe list to find the next lower timeframe
      for (int i = TFstart + TFamt - 1; i >= TFstart; i--) {
        if (sc.Input[i].GetInt() >= actualTF || sc.Input[i].GetInt() == 0)
          continue;
        // the day must be evenly divisible by the new timeframe
        if (secsPerDay % sc.Input[i].GetInt() != 0)
          continue;
        nextTF = sc.Input[i].GetInt();
        BarPeriod.IntradayChartBarPeriodParameter1 = nextTF;
        break;
      }
    }

    // set the new timeframe
    if (nextTF != 0) {
      sc.SetBarPeriodParameters(BarPeriod);
      if (useSpacing.GetYesNo() == 1)
        sc.ChartBarSpacing =
            maxThreshold.GetInt() - (int)((maxThreshold.GetInt() - minThreshold.GetInt()) * 0.2);
      // if(useSpacing.GetYesNo() == 1) sc.ChartBarSpacing =
      // minThreshold.GetInt() + (int) ( (maxThreshold.GetInt() -
      // minThreshold.GetInt() ) * 0.2);
    }

    // deactivate custom button
    sc.SetCustomStudyControlBarButtonEnable(1, 0);
  }

  // set DaysToLoad according to new timeframe
  if (nextTF > 0) // days to load for historical bars has been explicitly set above
  {
    // if setting with number of bars in chart
    if (setDaysToLoad.GetIndex() == 1) {
      // int secsPerDay = sc.UseSecondStartEndTimes == 1
      //                      ? 60 * 60 * 24
      //                      : sc.EndTime1 - sc.StartTime1 + 1;
      //
      // secsPerDay = secsPerDay > 1 ? secsPerDay : 60 * 60 * 24;
      int secsPerDay = SecondsPerDay(sc.UseSecondStartEndTimes, sc.StartTime1, sc.EndTime1);
      int barsPerDay = sc.Round(((float)secsPerDay) / nextTF);

      int nDaysToLoad = sc.Round(((float)barsInChart.GetInt()) / barsPerDay);
      nDaysToLoad = max(1, nDaysToLoad);    // take today into account
      nDaysToLoad += (nDaysToLoad / 5) * 2; // add weekends

      sc.DaysToLoadInChart = nDaysToLoad;
    }
    // if setting depending on timeframe
    else if (setDaysToLoad.GetIndex() == 2) {
      sc.DaysToLoadInChart = nextTF > 3900   ? sc.Input[toLoadStart + 4].GetInt()
                             : nextTF > 1800 ? sc.Input[toLoadStart + 3].GetInt()
                             : nextTF > 300  ? sc.Input[toLoadStart + 2].GetInt()
                             : nextTF > 60   ? sc.Input[toLoadStart + 1].GetInt()
                                             : sc.Input[toLoadStart].GetInt();
    }
  }

  //**WORK WITH OTHER STUDIES
  if (nextTF > 0) {
    // set daily VbP study
    int studyId = vbpID.GetStudyID();
    if (studyId != 0) {
      int separateEvening = nextTF > 1800 ? 0 : 1;

      sc.SetChartStudyInputInt(sc.ChartNumber, studyId, 41, separateEvening);
    }

    // hide || show studies
    for (int x = hideStudiesStart; x < hideStudiesStart + hideStudiesAmt * 2; x = x + 2) {
      studyId = sc.Input[x].GetStudyID();
      if (studyId != 0) {
        int visib = nextTF >= sc.Input[x + 1].GetInt() ? 0 : 1;
        sc.SetStudyVisibilityState(studyId, visib);
      }
    }
  }
}

SCSFExport scsf_RealHisLos(SCStudyInterfaceRef sc) {
  SCSubgraphRef hiGraph = sc.Subgraph[59];
  SCSubgraphRef loGraph = sc.Subgraph[58];
  SCSubgraphRef tHiGraph = sc.Subgraph[57];
  SCSubgraphRef tLoGraph = sc.Subgraph[56];
  SCSubgraphRef prevHisGraphs = sc.Subgraph[55];
  SCSubgraphRef prevLosGraphs = sc.Subgraph[54];

  SCInputRef nTicks = sc.Input[0];
  SCInputRef nHiLos = sc.Input[1];
  SCInputRef interpColors = sc.Input[2];
  SCInputRef clear = sc.Input[3];

  // CONSTANTS FOR SUBGRAPH INDEXES OF PREVIOUS HIGHS AND LOWS
  // old hi&los will be automatically created and numbered from
  // subgraph[FIRST_P_HIGH] downwards, copying the properties specified for the
  // prevHis and prevLos subgraphs ([54] and [55])
  const int FIRST_P_LOW = 52; // even number lower than any other subgraph
  const int FIRST_P_HIGH = FIRST_P_LOW + 1;
  const int MAX_P_HISLOS = (FIRST_P_LOW / 2) + 1;

  if (sc.SetDefaults) {
    // SUBGRAPHS
    sc.GraphName = "Real His & Los";
    sc.StudyDescription = "Indicator shows new swing Highs and Lows";
    sc.AutoLoop = 1;
    sc.GraphRegion = 0;
    sc.MaintainVolumeAtPriceData = 1;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    // set initial transparency
    sc.SetChartStudyTransparencyLevel(sc.ChartNumber, sc.StudyGraphInstanceID, 92);

    hiGraph.Name = "Last High";
    hiGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    hiGraph.LineWidth = 10;
    hiGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_FAR_RIGHT | LL_NAME_REVERSE_COLORS;
    hiGraph.PrimaryColor = RGB(0, 0, 0);
    hiGraph.DrawZeros = 0;
    hiGraph.ShortName = "▼ HI";

    loGraph.Name = "Last Low";
    loGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    loGraph.LineWidth = 10;
    loGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_FAR_RIGHT | LL_NAME_REVERSE_COLORS;
    loGraph.PrimaryColor = RGB(0, 0, 0);
    loGraph.DrawZeros = 0;
    loGraph.ShortName = "▲ LO";

    tHiGraph.Name = "Temporary High";
    tHiGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    tHiGraph.LineWidth = 6;
    tHiGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_FAR_RIGHT | LL_NAME_REVERSE_COLORS;
    tHiGraph.PrimaryColor = RGB(0, 145, 215);
    tHiGraph.DrawZeros = 0;
    tHiGraph.ShortName = "▼t.hi";

    tLoGraph.Name = "Temporary Low";
    tLoGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    tLoGraph.LineWidth = 6;
    tLoGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_FAR_RIGHT | LL_NAME_REVERSE_COLORS;
    tLoGraph.PrimaryColor = RGB(255, 0, 0);
    tLoGraph.DrawZeros = 0;
    tLoGraph.ShortName = "▲t.lo";

    // old highs will copy these properties
    prevHisGraphs.Name = "Previous Highs";
    prevHisGraphs.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP; // DRAWSTYLE_TRIANGLE_DOWN;
    prevHisGraphs.LineWidth = 8;
    prevHisGraphs.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_FAR_RIGHT;
    prevHisGraphs.PrimaryColor = RGB(255, 0, 0);
    prevHisGraphs.SecondaryColor = RGB(255, 150, 150);
    prevHisGraphs.SecondaryColorUsed = 1;
    prevHisGraphs.DrawZeros = 0;
    prevHisGraphs.ShortName = "▼";
    prevHisGraphs.UseTransparentLabelBackground = 1;

    // old lows will copy these properties
    prevLosGraphs.Name = "Previous Lows";
    prevLosGraphs.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM; // DRAWSTYLE_TRIANGLE_UP;
    prevLosGraphs.LineWidth = 8;
    prevLosGraphs.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_FAR_RIGHT;
    prevLosGraphs.PrimaryColor = RGB(0, 145, 215);
    prevLosGraphs.SecondaryColor = RGB(190, 170, 170);
    prevLosGraphs.SecondaryColorUsed = 1;
    prevLosGraphs.DrawZeros = 0;
    prevLosGraphs.ShortName = "▲";
    prevLosGraphs.UseTransparentLabelBackground = 1;
    prevLosGraphs.UseLabelsColor = 1;
    prevLosGraphs.LabelsColor = RGB(0, 145, 215);

    // INPUTS
    nTicks.Name = "Number of ticks to confirm new swing Hi&Lo";
    nTicks.SetInt(12);

    nHiLos.Name = "Number of previous Hi&Los to display";
    nHiLos.SetInt(12);
    nHiLos.SetIntLimits(0, MAX_P_HISLOS);

    interpColors.Name = "Interp. Prim/Sec Colors for Prev HiLos";
    interpColors.SetYesNo(0);

    clear.Name = "Clear Current Trades on break of curr. Hi/Lo";
    clear.SetYesNo(0);

    return;
  }

  // SET REFERENCES TO PERSISTENT VARIABLES
  float &newLo = sc.GetPersistentFloat(0);
  float &newHi = sc.GetPersistentFloat(1);
  float &lastTradeAtBid = sc.GetPersistentFloat(2);
  float &lastTradeAtAsk = sc.GetPersistentFloat(3);

  int &lastIndex = sc.GetPersistentInt(0);
  int &highBroken = sc.GetPersistentInt(1);
  int &lowBroken = sc.GetPersistentInt(2);

  int64_t &LastProcessedSequence = sc.GetPersistentInt64(0);
  int i = sc.Index;

  // Get the Time and Sales
  c_SCTimeAndSalesArray TimeSales;
  sc.GetTimeAndSales(TimeSales);

  // on start, configure all subgraphs acc. to inputs and create enough
  // subgraphs for nHiLos to display
  if (sc.IsFullRecalculation && i == 0) {
    // old hi&los will be automatically created and numbered from subgraph[53]
    // downwards, copying the properties specified for the prevHis and prevLos
    // subgraphs ([54] and [55])
    for (int x = 1; x <= MAX_P_HISLOS; x++) {
      if (x <= nHiLos.GetInt()) {
        // new subgraphs for previous highs
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].Name.Format("High %i", x);
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].DrawStyle = prevHisGraphs.DrawStyle;
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].LineWidth = prevHisGraphs.LineWidth;
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].LineLabel = prevHisGraphs.LineLabel;
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].PrimaryColor =
            nHiLos.GetInt() == 1 || interpColors.GetYesNo() == 0
                ? prevHisGraphs.PrimaryColor
                : sc.RGBInterpolate(prevHisGraphs.PrimaryColor, prevHisGraphs.SecondaryColor,
                                    sqrt((float)(x - 1) / (nHiLos.GetInt() - 1)));
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].DrawZeros = 0;
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].ShortName.Format(
            prevHisGraphs.ShortName + " %i " + prevHisGraphs.ShortName, x);
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].UseLabelsColor = prevHisGraphs.UseLabelsColor;
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].LabelsColor = prevHisGraphs.LabelsColor;
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].UseTransparentLabelBackground =
            prevHisGraphs.UseTransparentLabelBackground;

        // new subgraphs for previous lows
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].Name.Format("Low %i", x);
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].DrawStyle = prevLosGraphs.DrawStyle;
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].LineWidth = prevLosGraphs.LineWidth;
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].LineLabel = prevLosGraphs.LineLabel;
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].PrimaryColor =
            nHiLos.GetInt() == 1 || interpColors.GetYesNo() == 0
                ? prevLosGraphs.PrimaryColor
                : sc.RGBInterpolate(prevLosGraphs.PrimaryColor, prevLosGraphs.SecondaryColor,
                                    sqrt((float)(x - 1) / (nHiLos.GetInt() - 1)));
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].DrawZeros = 0;
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].ShortName.Format(
            prevLosGraphs.ShortName + " %i " + prevLosGraphs.ShortName, x);
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].UseLabelsColor = prevLosGraphs.UseLabelsColor;
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].LabelsColor = prevLosGraphs.LabelsColor;
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].UseTransparentLabelBackground =
            prevLosGraphs.UseTransparentLabelBackground;
      } else
      // hide unneeded previous hi&los
      {
        sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)].Name = "";
        sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)].Name = "";
      }
    }

    // set on first calculation
    LastProcessedSequence = 0;
    lastIndex = 0;
    newLo = sc.Low[0];
    newHi = 0;
    lastTradeAtAsk = sc.Low[0];
    lastTradeAtBid = sc.Low[0];
  }

  // if new candle, copy last subgraph values and delete previous candle's
  // values
  if (i != lastIndex)
    for (int x = 0; x < 60; x++) {
      sc.Subgraph[x][i] = sc.Subgraph[x][i - 1];
    }

  // READ T&S ARRAY TO SEE ALL PRICES
  // first find first T&S entry from which to calculate in this update
  // or skip if calculating on bars at start of indicator's life
  int TSIndex = 0, limit = 1;
  if (!sc.IsFullRecalculation) {
    limit = TimeSales.Size();
    if (limit == 0)
      return; // No Time and Sales data available for the symbol
    int TSIndex = limit - 1;
    for (TSIndex; TSIndex > 0; TSIndex--) {
      if (TimeSales[TSIndex - 1].Sequence <= LastProcessedSequence)
        break;
    }
  }

  // now, iterate over the new Time&Sales entries
  // if calculating on bars (at start) the for loop will be executed only once
  // as per previous code
  for (TSIndex; TSIndex < limit; TSIndex++) {
    // calculation on historical bars: read bars' VAP to look for last trades at
    // bid and ask
    if (sc.IsFullRecalculation) {
      // GET VAP BAR DATA TO GET THE HIGHEST TRADE AT BID AND THE LOWEST AT ASK
      // IN THIS BAR define VAP for historical calculations
      s_VolumeAtPriceV2 *p_VolumeAtPrice = NULL;
      // check there is VolumeAtPrice data
      if (static_cast<int>(sc.VolumeAtPriceForBars->GetNumberOfBars()) < sc.ArraySize)
        return;
      int VAPSizeAtBarIndex = sc.VolumeAtPriceForBars->GetSizeAtBarIndex(i);

      for (int VAPIndex = 0; VAPIndex < VAPSizeAtBarIndex; VAPIndex++) {
        if (!sc.VolumeAtPriceForBars->GetVAPElementAtIndex(i, VAPIndex, &p_VolumeAtPrice))
          break;

        if (p_VolumeAtPrice)
          if (p_VolumeAtPrice->AskVolume > 0) {
            lastTradeAtAsk = p_VolumeAtPrice->PriceInTicks * sc.TickSize;
            break;
          }
      }

      for (int VAPIndex = VAPSizeAtBarIndex - 1; VAPIndex >= 0; VAPIndex--) {
        if (!sc.VolumeAtPriceForBars->GetVAPElementAtIndex(i, VAPIndex, &p_VolumeAtPrice))
          break;

        if (p_VolumeAtPrice)
          if (p_VolumeAtPrice->BidVolume > 0) {
            lastTradeAtBid = p_VolumeAtPrice->PriceInTicks * sc.TickSize;
            break;
          }
      }
    }
    // ACTUAL READING IF THE TIME&SALES IN REAL-TIME CALCULATIONS
    else {
      // if there haven't been any new T&S records || this record is
      // a bid-ask update, continue
      if (TimeSales[TSIndex].Sequence <= LastProcessedSequence ||
          TimeSales[TSIndex].Type == SC_TS_BIDASKVALUES)
        continue;

      // see whether this trade took place at the bid || at the ask
      if (TimeSales[TSIndex].Type == SC_TS_BID) {
        lastTradeAtBid = TimeSales[TSIndex].Bid * sc.RealTimePriceMultiplier;

        // see if this trade broke the current low and clear current trades
        if (clear.GetYesNo() && lastTradeAtBid < loGraph[i] && lowBroken == false) {
          sc.ClearCurrentTradedBidAskVolume();
          lowBroken = true;
        }
      } else if (TimeSales[TSIndex].Type == SC_TS_ASK) {
        lastTradeAtAsk = TimeSales[TSIndex].Ask * sc.RealTimePriceMultiplier;

        // see if this trade borke the current high and clear current trades
        if (clear.GetYesNo() && lastTradeAtAsk > hiGraph[i] && highBroken == false) {
          sc.ClearCurrentTradedBidAskVolume();
          highBroken = true;
        }
      }
    }

    // HAS A NEW HIGH || LOW BEEN CONFIRMED?
    // if there is a new confirmed high (there must be trades at ask at newHi -
    // nTicks)
    if (newHi != 0 && lastTradeAtAsk <= (newHi - nTicks.GetInt() * sc.TickSize)) {
      // new confirmed Hi
      // move highs to previous highs subgraphs and change their number display
      for (int x = nHiLos.GetInt() - 1; x > 0; x--) {
        sc.Subgraph[FIRST_P_HIGH - 2 * x][i] = sc.Subgraph[FIRST_P_HIGH - 2 * (x - 1)][i];
      }

      // move current high to first old high
      if (nHiLos.GetInt() > 0)
        sc.Subgraph[FIRST_P_HIGH][i] = hiGraph[i];

      // set subgraph for just confirmed high
      hiGraph[i] = newHi;

      // reset "new" variables to look for next lo
      newLo = lastTradeAtAsk;
      newHi = 0;
      highBroken = false;
    } else
      // if there is a new confirmed low (there must be trades at bid at newLo +
      // nTicks)
      if (newLo != 0 && lastTradeAtBid >= (newLo + nTicks.GetInt() * sc.TickSize)) {
        // new confirmed Lo
        for (int x = nHiLos.GetInt() - 1; x > 0; x--) {
          sc.Subgraph[FIRST_P_LOW - 2 * x][i] = sc.Subgraph[FIRST_P_LOW - 2 * (x - 1)][i];
        }

        // move current high to first old high
        if (nHiLos.GetInt() > 0)
          sc.Subgraph[FIRST_P_LOW][i] = loGraph[i];

        // set subgraph for just confirmed low
        loGraph[i] = newLo;

        // reset "new" variables to look for next high
        newHi = lastTradeAtBid;
        newLo = 0;
        lowBroken = false;
      } else
        // check temporary highs || lows
        if (newLo != 0 && lastTradeAtAsk < newLo) {
          newLo = lastTradeAtAsk; // a new temp low will be created with trades
                                  // at ask
        } else if (newHi != 0 && lastTradeAtBid > newHi) {
          newHi = lastTradeAtBid; // a new temp high will be created with trades
                                  // at bid
        }

    // save temporary hi || low into subgraphs
    tHiGraph[i] = newHi;
    tLoGraph[i] = newLo;
  }

  // SAVE VALUES FOR NEXT UPDATE
  LastProcessedSequence = TimeSales[TimeSales.Size() - 1].Sequence;
  // save candle index
  lastIndex = i;
}

SCSFExport scsf_AutoMA(SCStudyInterfaceRef sc) {
  SCSubgraphRef Subgraph_TopMovAvg = sc.Subgraph[0];
  SCSubgraphRef Subgraph_BottomMovAvg = sc.Subgraph[1];
  SCSubgraphRef Subgraph_FillTop3 = sc.Subgraph[2];
  SCSubgraphRef Subgraph_FillTop2 = sc.Subgraph[3];
  SCSubgraphRef Subgraph_FillBottom2 = sc.Subgraph[4];
  SCSubgraphRef Subgraph_FillBottom3 = sc.Subgraph[5];
  SCSubgraphRef Subgraph_Top1 = sc.Subgraph[6];
  SCSubgraphRef Subgraph_Top2 = sc.Subgraph[7];
  SCSubgraphRef Subgraph_Top3 = sc.Subgraph[8];
  SCSubgraphRef Subgraph_Bottom1 = sc.Subgraph[9];
  SCSubgraphRef Subgraph_Bottom2 = sc.Subgraph[10];
  SCSubgraphRef Subgraph_Bottom3 = sc.Subgraph[11];

  SCSubgraphRef Subgraph_Temp4 = sc.Subgraph[12];
  SCSubgraphRef Subgraph_Temp5 = sc.Subgraph[13];

  SCInputRef Input_MovAvgType = sc.Input[0];
  SCInputRef Input_TopBandInputData = sc.Input[1];
  SCInputRef Input_BottomBandInputData = sc.Input[2];

  if (sc.SetDefaults) {
    sc.GraphName = "Auto MA";
    sc.GraphRegion = 0;
    sc.DisplayStudyName = 1;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    sc.ScaleRangeType = SCALE_SAMEASREGION;
    sc.AutoLoop = false;

    Input_TopBandInputData.Name = "Top Band Input Data";
    Input_TopBandInputData.SetInputDataIndex(SC_LAST);

    Input_BottomBandInputData.Name = "Bottom Band Input Data";
    Input_BottomBandInputData.SetInputDataIndex(SC_LAST);

    Input_MovAvgType.Name = "Moving Average Type";
    Input_MovAvgType.SetMovAvgType(MOVAVGTYPE_EXPONENTIAL);

    Subgraph_TopMovAvg.Name = "Top MA";
    Subgraph_TopMovAvg.DrawStyle = DRAWSTYLE_LINE;
    Subgraph_TopMovAvg.PrimaryColor = RGB(255, 0, 0);
    Subgraph_TopMovAvg.ShortName = "MA";

    Subgraph_BottomMovAvg.Name = "Bottom MA";
    Subgraph_BottomMovAvg.DrawStyle = DRAWSTYLE_LINE;
    Subgraph_BottomMovAvg.PrimaryColor = RGB(255, 0, 0);
    Subgraph_BottomMovAvg.ShortName = "MA";

    // lines
    for (int x = 1; x <= 3; x++) {
      sc.Subgraph[5 + x].Name.Format("MA +%istd", x);
      sc.Subgraph[5 + x].ShortName = sc.Subgraph[5 + x].Name;
      sc.Subgraph[5 + x].DrawStyle = DRAWSTYLE_LINE;
      sc.Subgraph[5 + x].PrimaryColor = RGB(255, 160, 160);

      sc.Subgraph[8 + x].Name.Format("MA -%istd", x);
      sc.Subgraph[8 + x].ShortName = sc.Subgraph[8 + x].Name;
      sc.Subgraph[8 + x].DrawStyle = DRAWSTYLE_LINE;
      sc.Subgraph[8 + x].PrimaryColor = RGB(135, 190, 135);
    }

    // fills
    Subgraph_FillTop3.Name = "Fill +3sd";
    Subgraph_FillTop3.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_TOP;
    Subgraph_FillTop3.PrimaryColor = RGB(255, 215, 215);
    Subgraph_FillTop2.Name = "Fill +2sd";
    Subgraph_FillTop2.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_BOTTOM;
    Subgraph_FillTop2.PrimaryColor = RGB(255, 215, 215);

    Subgraph_FillBottom2.Name = "Fill -2sd";
    Subgraph_FillBottom2.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_TOP;
    Subgraph_FillBottom2.PrimaryColor = RGB(195, 225, 195);
    Subgraph_FillBottom3.Name = "Fill -3sd";
    Subgraph_FillBottom3.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_BOTTOM;
    Subgraph_FillBottom3.PrimaryColor = RGB(195, 225, 195);

    return;
  }

  // persistent variables
  int &MAperiod = sc.GetPersistentInt(0);

  //*** SET WHAT MA PERIOD TO DISPLAY AT INITIALIZATION
  if (sc.IsFullRecalculation) {
    // iniital values form MA variables
    MAperiod = 1;
    SCString maType = "Daily";

    // get period parameters
    n_ACSIL::s_BarPeriod BarPeriod;
    sc.GetBarPeriodParameters(BarPeriod);

    // if intraday
    if (BarPeriod.ChartDataType == INTRADAY_DATA) {
      // how many bars in the last day and set that to the MA period
      SCDateTime lastBarTime = sc.BaseDateTimeIn[sc.ArraySize - 1];
      SCDateTime yesterday = lastBarTime.SubtractDays(1);

      for (int x = sc.ArraySize - 2; x >= 0; x--) {
        if (sc.BaseDateTimeIn[x] <= yesterday &&
            sc.BaseDateTimeIn[x].GetTimeInSeconds() <= lastBarTime.GetTimeInSeconds())
          break;
        MAperiod++;
        // if no new day found in the whole array, set period to zero to return
        // later
        if (x == 0)
          MAperiod = 0;
      }

      // if intraday time-based chart, set the MA period depending on chart
      // timeframe rest of chart types will display daily MA
      if (BarPeriod.IntradayChartBarPeriodType == IBPT_DAYS_MINS_SECS) {
        // what is the chart timeframe
        int tf = 0;
        tf = BarPeriod.IntradayChartBarPeriodParameter1;
        /*int secsPerDay =
                sc.UseSecondStartEndTimes == 1 ? sc.EndTime2 - sc.StartTime2 + 1
        + sc.EndTime1 - sc.StartTime1 + 1: sc.EndTime1 - sc.StartTime1 + 1;
        MAperiod = max(sc.Round( ( (float) secsPerDay) / tf ) , 1); */

        // set the MA period depending on chart timeframe
        if (tf >= 120 * 60) {
          maType = "Quarterly";
          MAperiod *= 62;
        } else if (tf >= 60 * 60) {
          maType = "Monthly";
          MAperiod *= 21;
        } else if (tf >= 15 * 60) {
          maType = "Weekly";
          MAperiod *= 5;
        }
      }
    }
    // if historical chart
    else {
      if (BarPeriod.HistoricalChartBarPeriodType == 1) {
        maType = "Quarterly";
        MAperiod = 62;
      } else if (BarPeriod.HistoricalChartBarPeriodType == 2) {
        maType = "Yearly";
        MAperiod = 52;
      } else {
        maType = "ThreeYearly";
        MAperiod = 36;
      }
    }

    // if input series for middle band is the same, hide bottom band
    if (Input_TopBandInputData.GetInputDataIndex() ==
        Input_BottomBandInputData.GetInputDataIndex()) {
      Subgraph_BottomMovAvg.DrawStyle = DRAWSTYLE_HIDDEN;
    }

    // set study name to include the MA period
    sc.GraphName.Format("AutoMA: %s(%i)", maType.GetChars(), MAperiod);
  }

  // return if MA period wasn't found
  if (MAperiod <= 1)
    return;

  //**** CALCULATE: same code as the Standard Deviation study with very minor
  // modifications
  sc.DataStartIndex = MAperiod - 1;

  int CalculationStartIndex = sc.GetCalculationStartIndexForStudy();
  if (CalculationStartIndex > sc.UpdateStartIndex)
    CalculationStartIndex = sc.UpdateStartIndex;

  sc.EarliestUpdateSubgraphDataArrayIndex = CalculationStartIndex;

  for (int BarIndex = CalculationStartIndex; BarIndex < sc.ArraySize; BarIndex++) {
    SCFloatArrayRef HighArray = sc.BaseDataIn[Input_TopBandInputData.GetInputDataIndex()];
    SCFloatArrayRef LowArray = sc.BaseDataIn[Input_BottomBandInputData.GetInputDataIndex()];

    sc.MovingAverage(HighArray, Subgraph_TopMovAvg, Input_MovAvgType.GetMovAvgType(), BarIndex,
                     MAperiod);
    sc.StdDeviation(HighArray, Subgraph_Temp4, BarIndex, MAperiod);

    sc.MovingAverage(LowArray, Subgraph_BottomMovAvg, Input_MovAvgType.GetMovAvgType(), BarIndex,
                     MAperiod);
    sc.StdDeviation(LowArray, Subgraph_Temp5, BarIndex, MAperiod);

    for (int x = 1; x <= 3; x++) {
      // subgraph values for lines
      sc.Subgraph[5 + x][BarIndex] = (Subgraph_Temp4[BarIndex] * x) + Subgraph_TopMovAvg[BarIndex];
      sc.Subgraph[8 + x][BarIndex] =
          (Subgraph_Temp5[BarIndex] * (-x)) + Subgraph_BottomMovAvg[BarIndex];
      // subgraph values for fills
      if (x > 1) {
        sc.Subgraph[0 + x][BarIndex] =
            (Subgraph_Temp4[BarIndex] * x) + Subgraph_TopMovAvg[BarIndex];
        sc.Subgraph[2 + x][BarIndex] =
            (Subgraph_Temp5[BarIndex] * (-x)) + Subgraph_BottomMovAvg[BarIndex];
      }
    }
  }
}

SCSFExport scsf_Levels(SCStudyInterfaceRef sc) {
  SCInputRef levelType = sc.Input[0];
  SCInputRef nBarsHL = sc.Input[1];
  SCInputRef percent = sc.Input[2];
  SCInputRef resetCurrTrades = sc.Input[3];
  SCInputRef showNunfilled = sc.Input[4];
  SCInputRef showNfilled = sc.Input[5];
  SCInputRef showMidline = sc.Input[6];

  SCSubgraphRef highsLows = sc.Subgraph[0];
  SCSubgraphRef hiLoExtension = sc.Subgraph[1];
  SCSubgraphRef levelLines = sc.Subgraph[2];
  SCSubgraphRef levelFills = sc.Subgraph[3];

  // last calculated bar
  int &lastBar = sc.GetPersistentInt(0);

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "Chart Levels";
    sc.AutoLoop = 1;
    sc.GraphRegion = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;

    levelType.Name = "Create Levels from";
    levelType.SetCustomInputStrings(
        "Fair Value Gaps;Highs & Lows +/- swing %;High & Low candle;Highs & "
        "Lows plus candle height;OrderBlock after High/Low");
    levelType.SetCustomInputIndex(4);

    nBarsHL.Name = "Number of Bars for new High || Low";
    nBarsHL.SetInt(6);

    percent.Name = "Swing % inwards/outwards of High || Low";
    percent.SetInt(15);
    percent.SetIntLimits(-100, 100);

    resetCurrTrades.Name = "Reset Curr.Trades on Break of Last HiLo";
    resetCurrTrades.SetYesNo(0);

    showNunfilled.Name = "How many Unfilled Levels to display";
    showNunfilled.SetInt(20);
    showNunfilled.SetIntLimits(1, 10);

    showNfilled.Name = "How many Filled Levels to display";
    showNfilled.SetInt(50);
    showNfilled.SetIntLimits(0, 1000);

    showMidline.Name = "Show Midline";
    showMidline.SetYesNo(0);

    highsLows.Name = "Highs & Lows Zigzag";
    highsLows.DrawStyle = DRAWSTYLE_HIDDEN;
    highsLows.PrimaryColor = RGB(0, 0, 255);
    highsLows.LineStyle = LINESTYLE_SOLID;
    highsLows.LineWidth = 2;
    highsLows.DrawZeros = false;

    hiLoExtension.Name = "Hi&Lo Extension Lines";
    hiLoExtension.DrawStyle = DRAWSTYLE_LINE;
    hiLoExtension.LineStyle = LINESTYLE_DOT;
    hiLoExtension.SecondaryColorUsed = 1;
    hiLoExtension.LineWidth = 2;
    hiLoExtension.PrimaryColor = RGB(234, 49, 109);
    hiLoExtension.SecondaryColor = RGB(14, 115, 239);

    levelLines.Name = "Level Borderlines (0 width to hide)";
    levelLines.DrawStyle = DRAWSTYLE_LINE;
    levelLines.LineStyle = LINESTYLE_SOLID;
    levelLines.SecondaryColorUsed = 1;
    levelLines.LineWidth = 1;
    levelLines.PrimaryColor = RGB(252, 220, 231);
    levelLines.SecondaryColor = RGB(219, 234, 253);

    levelFills.Name = "Level Fills (set to Hidden to hide)";
    levelFills.DrawStyle = DRAWSTYLE_LINE;
    levelFills.LineStyle = LINESTYLE_SOLID;
    levelFills.SecondaryColorUsed = 1;
    levelFills.LineWidth = 1;
    levelFills.PrimaryColor = RGB(244, 143, 177);
    levelFills.SecondaryColor = RGB(144, 191, 249);

    // set initial transparency for extension fills
    sc.SetChartStudyTransparencyLevel(sc.ChartNumber, sc.StudyGraphInstanceID, 90);

    // reset lastBar
    lastBar = -1;

    return;
  }

  // persistent variables
  float &lastHigh = sc.GetPersistentFloat(-1);
  int &lastHighIndex = sc.GetPersistentInt(-1);
  float &lastLow = sc.GetPersistentFloat(1);
  int &lastLowIndex = sc.GetPersistentInt(1);
  int &lookFor = sc.GetPersistentInt(2); // lookForHigh = 1 ; lookForLow = -1;
  int &lastHiLoBroken = sc.GetPersistentInt(-2);

  // where do GetPersistentInt() pointers to store IDs of filled and unfilled
  // FVGs start
  const int firstTested = 5;
  int firstUntested = firstTested + showNfilled.GetInt() + 10;
  // GetPersistentInt() pointer for the ID of the next/coming UP || DOWN
  // unfilled FVG
  int &upFVGsNextID = sc.GetPersistentInt(4);
  int &downFVGsNextID = sc.GetPersistentInt(-4);

  // full recalculation
  if (sc.Index == 0 || lastBar == -1) // This indicates a study is being recalculated.
  {
    // When there is a full recalculation of the study,
    // reset the persistent variables we are using
    upFVGsNextID = firstUntested;
    downFVGsNextID = -firstUntested;
  }

  // if last high || low is broken reset CurrentTrades
  if (resetCurrTrades.GetYesNo() == 1 && lastHiLoBroken == 0 &&
      sc.GetBarHasClosedStatus() == BHCS_BAR_HAS_NOT_CLOSED) {
    if (sc.High[sc.Index] > lastHigh || sc.Low[sc.Index] < lastLow) {
      sc.ClearCurrentTradedBidAskVolume();
      lastHiLoBroken = 1;
    }
  }

  //**** BEGIN
  if (!sc.HideStudy && sc.Index != lastBar && sc.Index >= 3) {
    // CHECK FOR LEVEL RETESTS
    int counterUntestedShown = 0;
    // testing levels above?
    // loop through the unfilled DOWN-FVGs array
    for (int x = downFVGsNextID + 1; x <= -firstUntested; x++) {
      s_UseTool Rect;
      Rect.Clear();
      bool getLine = sc.GetACSDrawingByLineNumber(0, sc.GetPersistentInt(x), Rect);
      if (getLine) {
        double begin = Rect.BeginValue;
        // if this FVG has just been filled
        if (sc.High[sc.Index - 1] >= begin) // && sc.High[sc.Index-2] < begin)
        {
          // modify rectangle to make it finish in this bar
          Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
          Rect.EndIndex = sc.Index - 1;
          Rect.AddMethod = UTAM_ADD_OR_ADJUST;
          sc.UseTool(Rect);

          // delete its ID from the untested array and decrement the unfilled
          // pointer
          sc.GetPersistentInt(x) = 0;
          for (int y = x; y >= downFVGsNextID + 2; y--) {
            sc.GetPersistentInt(y) = sc.GetPersistentInt(y - 1);
          }
          downFVGsNextID++;

          // if showing filled FVGs, delete filled FVGs older than showNfilled
          if (showNfilled.GetInt() > 0) {
            for (int i = -firstTested; i > -firstTested - showNfilled.GetInt(); i--) {
              if (i == -firstTested)
                sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, sc.GetPersistentInt(i));
              if (i > -firstTested - showNfilled.GetInt() + 1)
                sc.GetPersistentInt(i) = sc.GetPersistentInt(i - 1);
            }

            // save the newly filled FVG ID to be able to delete it later
            sc.GetPersistentInt(-firstTested - showNfilled.GetInt() + 1) = Rect.LineNumber;
          }
          // if not, delete the newly filled gap
          else
            sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, Rect.LineNumber);

        }
        // if not filled
        else {
          if (counterUntestedShown < showNunfilled.GetInt()) {
            // show drawing
            Rect.HideDrawing = 0;
            Rect.AddMethod = UTAM_ADD_OR_ADJUST;
            sc.UseTool(Rect);
            counterUntestedShown++;
          }
        }
      }
    }

    // testing levels below?
    counterUntestedShown = 0;
    // loop through the unfilled UP-FVGs array
    for (int x = upFVGsNextID - 1; x >= firstUntested; x--) {
      s_UseTool Rect;
      Rect.Clear();
      bool getLine = sc.GetACSDrawingByLineNumber(0, sc.GetPersistentInt(x), Rect);
      if (getLine) {
        double begin = Rect.BeginValue;
        // if this FVG has just been filled
        if (sc.Low[sc.Index - 1] <= begin) // && sc.Low[sc.Index-2] > begin)
        {
          // modify rectangle to make it finish in this bar
          Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
          Rect.EndIndex = sc.Index - 1;
          Rect.AddMethod = UTAM_ADD_OR_ADJUST;
          sc.UseTool(Rect);

          // delete its ID from the untested array and decrement the unfilled
          // pointer
          sc.GetPersistentInt(x) = 0;
          for (int y = x; y <= upFVGsNextID - 2; y++) {
            sc.GetPersistentInt(y) = sc.GetPersistentInt(y + 1);
          }
          upFVGsNextID--;

          // if showing filled FVGs, delete filled FVGs older than showNfilled
          if (showNfilled.GetInt() > 0) {
            for (int i = firstTested; i < firstTested + showNfilled.GetInt(); i++) {
              if (i == firstTested)
                sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, sc.GetPersistentInt(i));
              if (i < firstTested + showNfilled.GetInt() - 1)
                sc.GetPersistentInt(i) = sc.GetPersistentInt(i + 1);
            }

            // save the newly filled FVG ID to be able to delete it later
            sc.GetPersistentInt(firstTested + showNfilled.GetInt() - 1) = Rect.LineNumber;
          }
          // if not, delete the newly filled gap
          else
            sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, Rect.LineNumber);
        }
        // if not filled
        else {
          if (counterUntestedShown < showNunfilled.GetInt()) {
            // show drawing
            Rect.HideDrawing = 0;
            Rect.AddMethod = UTAM_ADD_OR_ADJUST;
            sc.UseTool(Rect);
            counterUntestedShown++;
          }
        }
      }
    }

    //**** HIGHS & LOWS CALCULATION
    int mmIndex = sc.Index - nBarsHL.GetInt();
    bool newLow = false, newHigh = false;
    int sinceHigh =
        sc.NumberOfBarsSinceHighestValue(sc.BaseDataIn[SC_HIGH], sc.Index - 1, nBarsHL.GetInt());
    int sinceLow =
        sc.NumberOfBarsSinceLowestValue(sc.BaseDataIn[SC_LOW], sc.Index - 1, nBarsHL.GetInt());

    // seteo highs lows buffer initially to zero in the just completed bar
    highsLows[sc.Index - 1] = 0;

    // if there is a new high || low nBarsHL ago
    if (sinceHigh == nBarsHL.GetInt() - 1 && lookFor >= 0) {
      highsLows[mmIndex] = sc.High[mmIndex];
      lastHigh = sc.High[mmIndex];
      lastHighIndex = mmIndex;
      // draw extension line
      if (hiLoExtension.DrawStyle != DRAWSTYLE_IGNORE &&
          hiLoExtension.DrawStyle != DRAWSTYLE_HIDDEN) {
        n_ACSIL::s_LineUntilFutureIntersection Line;
        Line.StartBarIndex = mmIndex;
        Line.LineValue = sc.High[mmIndex];
        Line.LineColor = hiLoExtension.PrimaryColor;
        Line.LineStyle = hiLoExtension.LineStyle;
        Line.LineWidth = hiLoExtension.LineWidth;

        sc.AddLineUntilFutureIntersectionEx(Line);
      }
      // save flag to now look for new low
      lookFor = -1;
      newHigh = true;
      // reset flag to reset current trades only if price has not ALREADY broken
      // the last low while confirming the new high
      if (sc.GetLowest(sc.BaseDataIn[SC_LOW], nBarsHL.GetInt()) >= lastLow)
        lastHiLoBroken = 0;
    } else if (sinceLow == nBarsHL.GetInt() - 1 && lookFor <= 0) {
      highsLows[mmIndex] = sc.Low[mmIndex];
      lastLow = sc.Low[mmIndex];
      lastLowIndex = mmIndex;
      // draw extension line
      if (hiLoExtension.DrawStyle != DRAWSTYLE_IGNORE &&
          hiLoExtension.DrawStyle != DRAWSTYLE_HIDDEN) {
        n_ACSIL::s_LineUntilFutureIntersection Line;
        Line.StartBarIndex = mmIndex;
        Line.LineValue = sc.Low[mmIndex];
        Line.LineColor = hiLoExtension.SecondaryColor;
        Line.LineStyle = hiLoExtension.LineStyle;
        Line.LineWidth = hiLoExtension.LineWidth;

        sc.AddLineUntilFutureIntersectionEx(Line);
      }
      // save flag to now look for new high
      lookFor = 1;
      newLow = true;
      // reset flag to reset current trades only if price has not ALREADY broken
      // the last high while confirming the new low
      if (sc.GetHighest(sc.BaseDataIn[SC_HIGH], nBarsHL.GetInt()) <= lastHigh)
        lastHiLoBroken = 0;
    }

    //**** LOOK FOR AND DRAW LEVELS: FVGs || HIGHS & LOWS
    // GAPUP || NEW LOW
    if ((levelType.GetIndex() == 0 && sc.Low[sc.Index - 1] > sc.High[sc.Index - 3] &&
         sc.Low[sc.Index - 2] < sc.Low[sc.Index - 1] &&
         sc.High[sc.Index - 2] >= sc.Low[sc.Index - 1]) ||
        (levelType.GetIndex() > 0 && newLow)) {
      // define a s_UseTool variable
      s_UseTool Rect;
      Rect.Clear();

      // create level
      Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
      if (levelType.GetIndex() == 0) {
        Rect.BeginIndex = sc.Index - 3;
        // set the rectangle levels
        Rect.BeginValue = sc.High[sc.Index - 3];
        Rect.EndValue = sc.Low[sc.Index - 1];
      } else {
        int type = levelType.GetIndex();
        // look for order block after the low: candle close > high of the candle
        // that marks the low
        int obIndex;
        for (obIndex = mmIndex + 1; obIndex <= sc.Index; obIndex++) {
          if (sc.Close[obIndex] > sc.High[mmIndex])
            break;
        }
        // define start and end of the extension
        float begin =
            type == 1 && lastHigh != 0 && percent.GetInt() > 0
                ? sc.Low[mmIndex] - (lastHigh - sc.Low[mmIndex]) * (percent.GetInt() / 100.0)
            : type == 2 ? sc.Low[mmIndex]
            : type == 3 ? 2 * sc.Low[mmIndex] - sc.High[mmIndex]
            : type == 4 ? sc.Low[obIndex]
                        : sc.Low[mmIndex];

        float end =
            type == 1 && percent.GetInt() < 0
                ? sc.Low[mmIndex] - (lastHigh - sc.Low[mmIndex]) * (percent.GetInt() / 100.0)
            : type == 2 ? sc.High[mmIndex]
            : type == 4 ? sc.Close[obIndex]
                        : sc.Low[mmIndex];

        // set the rectangle
        Rect.BeginIndex = type == 4 ? obIndex : mmIndex;
        Rect.BeginValue =
            type != 4 || obIndex != sc.Index ? begin : 0; // if looking for order block and none has
                                                          // been found, set to zero
        Rect.EndValue = end;
        // if highlighting order blocks and any bar since the low has already
        // broken the order block, do not extend rectangle if(type == 4 &&
        // sc.GetLowest(sc.BaseDataIn[SC_LOW], nBarsHL.GetInt() - 1) <
        // Rect.BeginValue) 	Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
      }

      if (Rect.BeginValue != 0) // execute only if begin value is set
      {
        Rect.EndIndex = sc.Index;
        Rect.Color = levelLines.SecondaryColor;
        Rect.SecondaryColor = levelFills.SecondaryColor;
        Rect.TransparencyLevel =
            levelFills.DrawStyle != DRAWSTYLE_HIDDEN && levelFills.DrawStyle != DRAWSTYLE_IGNORE
                ? sc.GetChartStudyTransparencyLevel(sc.ChartNumber, sc.StudyGraphInstanceID)
                : 100;
        Rect.NoVerticalOutline = 1;
        Rect.LineWidth = levelLines.LineWidth;
        Rect.LineStyle = levelLines.LineStyle;
        Rect.DrawMidline = showMidline.GetYesNo();

        sc.UseTool(Rect);

        // save level's ID in untested
        sc.GetPersistentInt(upFVGsNextID) = Rect.LineNumber;

        // hide previous untested FVGs
        if (upFVGsNextID >= firstUntested + showNunfilled.GetInt()) {
          Rect.Clear();
          bool getLine = sc.GetACSDrawingByLineNumber(
              0, sc.GetPersistentInt(upFVGsNextID - showNunfilled.GetInt()), Rect);
          Rect.HideDrawing = 1;
          Rect.AddMethod = UTAM_ADD_OR_ADJUST;
          sc.UseTool(Rect);
        }

        // increment untested IDs counter
        upFVGsNextID++;
      }
    }

    // GAPDOWN || NEW HIGH
    if ((levelType.GetIndex() == 0 && sc.High[sc.Index - 1] < sc.Low[sc.Index - 3] &&
         sc.High[sc.Index - 2] > sc.High[sc.Index - 1] &&
         sc.Low[sc.Index - 2] <= sc.High[sc.Index - 1]) ||
        (levelType.GetIndex() > 0 && newHigh)) {
      // define a s_UseTool variable
      s_UseTool Rect;
      Rect.Clear();

      // create level
      Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
      if (levelType.GetIndex() == 0) {
        Rect.BeginIndex = sc.Index - 3;
        // set the rectangle levels
        Rect.BeginValue = sc.Low[sc.Index - 3];
        Rect.EndValue = sc.High[sc.Index - 1];
      } else {
        int type = levelType.GetIndex();
        // look for order block after the high: candle close < low of the candle
        // that marks the high
        int obIndex;
        for (obIndex = mmIndex + 1; obIndex <= sc.Index; obIndex++) {
          if (sc.Close[obIndex] < sc.Low[mmIndex])
            break;
        }
        // define start and end of the extension
        float begin =
            type == 1 && lastLow != 0 && percent.GetInt() > 0
                ? sc.High[mmIndex] + (sc.High[mmIndex] - lastLow) * (percent.GetInt() / 100.0)
            : type == 2 ? sc.High[mmIndex]
            : type == 3 ? 2 * sc.High[mmIndex] - sc.Low[mmIndex]
            : type == 4 ? sc.High[obIndex]
                        : sc.High[mmIndex];

        float end =
            type == 1 && percent.GetInt() < 0
                ? sc.High[mmIndex] + (sc.High[mmIndex] - lastLow) * (percent.GetInt() / 100.0)
            : type == 2 ? sc.Low[mmIndex]
            : type == 4 ? sc.Close[obIndex]
                        : sc.High[mmIndex];

        // set the rectangle
        Rect.BeginIndex = type == 4 ? obIndex : mmIndex;
        Rect.BeginValue =
            type != 4 || obIndex != sc.Index ? begin : 0; // if looking for order block and none has
                                                          // been found, set to zero
        Rect.EndValue = end;
        // if highlighting order blocks and any bar since the high has already
        // broken the order block, do not extend rectangle if(type == 4 &&
        // sc.GetHighest(sc.BaseDataIn[SC_HIGH], nBarsHL.GetInt() - 1) >
        // Rect.BeginValue) 	Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
      }

      if (Rect.BeginValue != 0) // execute only if begin value is set
      {
        Rect.EndIndex = sc.Index;
        Rect.Color = levelLines.PrimaryColor;
        Rect.SecondaryColor = levelFills.PrimaryColor;
        Rect.TransparencyLevel =
            levelFills.DrawStyle != DRAWSTYLE_HIDDEN && levelFills.DrawStyle != DRAWSTYLE_IGNORE
                ? sc.GetChartStudyTransparencyLevel(sc.ChartNumber, sc.StudyGraphInstanceID)
                : 100;
        Rect.NoVerticalOutline = 1;
        Rect.LineWidth = levelLines.LineWidth;
        Rect.LineStyle = levelLines.LineStyle;
        Rect.DrawMidline = showMidline.GetYesNo();

        sc.UseTool(Rect);

        // save level's ID in untested
        sc.GetPersistentInt(downFVGsNextID) = Rect.LineNumber;

        // hide previous untested FVGs
        if (downFVGsNextID <= -firstUntested - showNunfilled.GetInt()) {
          Rect.Clear();
          bool getLine = sc.GetACSDrawingByLineNumber(
              0, sc.GetPersistentInt(downFVGsNextID + showNunfilled.GetInt()), Rect);
          Rect.HideDrawing = 1;
          Rect.AddMethod = UTAM_ADD_OR_ADJUST;
          sc.UseTool(Rect);
        }

        // increment untested IDs counter
        downFVGsNextID--;
      }
    }
  }

  // save Index
  lastBar = sc.Index;
}

SCSFExport scsf_HisLos(SCStudyInterfaceRef sc) {
  SCSubgraphRef highsLows = sc.Subgraph[0];
  SCSubgraphRef extensionLines = sc.Subgraph[1];

  SCInputRef nBarsHL = sc.Input[0];
  SCInputRef extendLines = sc.Input[1];
  SCInputRef resetCurrTrades = sc.Input[2];

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "HIs&LOs";
    sc.StudyDescription = "";
    sc.AutoLoop = 1; // true
    sc.GraphRegion = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;

    nBarsHL.Name = "Number of Bars for High&Low";
    nBarsHL.SetInt(6);

    extendLines.Name = "Draw Extension Lines";
    extendLines.SetYesNo(1);

    resetCurrTrades.Name = "Reset Curr.Trades on Break of Last HiLo";
    resetCurrTrades.SetYesNo(0);

    highsLows.Name = "Highs&Lows";
    highsLows.DrawStyle = DRAWSTYLE_IGNORE;
    highsLows.PrimaryColor = RGB(0, 0, 255);
    highsLows.LineStyle = LINESTYLE_SOLID;
    highsLows.LineWidth = 2;
    highsLows.DrawZeros = false;

    extensionLines.Name = "Extension Lines";
    extensionLines.DrawStyle = DRAWSTYLE_LINE;
    extensionLines.LineStyle = LINESTYLE_DOT;
    extensionLines.SecondaryColorUsed = 1;
    extensionLines.LineWidth = 2;
    extensionLines.PrimaryColor = RGB(240, 108, 151);
    extensionLines.SecondaryColor = RGB(73, 151, 245);
  }

  //**** PERSISTENT VARS
  float &lastHigh = sc.GetPersistentFloat(0);
  float &lastLow = sc.GetPersistentFloat(1);
  int &lastHiLoBroken = sc.GetPersistentInt(1);

  // if last high || low is broken reset CurrentTrades
  if (resetCurrTrades.GetYesNo() == 1 && lastHiLoBroken == 0 &&
      sc.GetBarHasClosedStatus() == BHCS_BAR_HAS_NOT_CLOSED) {
    if (sc.High[sc.Index] > lastHigh || sc.Low[sc.Index] < lastLow) {
      sc.ClearCurrentTradedBidAskVolume();
      lastHiLoBroken = 1;
    }
  }

  //**** CALCULATE ONLY ON BAR CLOSE
  if (sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED) {
    // defaults para barra nueva
    highsLows[sc.Index] = 0;

    //**** CÁLCULO DE HIGHS Y LOWS
    int mmIndex = sc.Index - nBarsHL.GetInt();
    ;

    float highest = sc.GetHighest(sc.BaseDataIn[SC_HIGH], 2 * nBarsHL.GetInt() + 1);
    float lowest = sc.GetLowest(sc.BaseDataIn[SC_LOW], 2 * nBarsHL.GetInt() + 1);

    //** new high
    if (highest == sc.High[mmIndex]) {
      // guardo en el buffer el nuevo high
      highsLows[mmIndex] = sc.High[mmIndex];
      lastHigh = sc.High[mmIndex];

      // draw lines
      if (extendLines.GetYesNo() == 1) {
        n_ACSIL::s_LineUntilFutureIntersection Line;
        Line.StartBarIndex = mmIndex;
        Line.LineValue = sc.High[mmIndex];
        Line.LineColor = extensionLines.PrimaryColor;
        Line.LineStyle = extensionLines.LineStyle;
        Line.LineWidth = extensionLines.LineWidth;

        sc.AddLineUntilFutureIntersectionEx(Line);
      }

      // reset flag to reset current trades only if price has not ALREADY broken
      // the last low while confirming the new high
      if (sc.GetLowest(sc.BaseDataIn[SC_LOW], nBarsHL.GetInt()) >= lastLow)
        lastHiLoBroken = 0;
    }

    //**new low
    if (lowest == sc.Low[mmIndex]) {
      // guardo en el buffer el nuevo low
      highsLows[mmIndex] = sc.Low[mmIndex];
      lastLow = sc.Low[mmIndex];

      // draw lines
      if (extendLines.GetYesNo() == 1) {
        n_ACSIL::s_LineUntilFutureIntersection Line;
        Line.StartBarIndex = mmIndex;
        Line.LineValue = sc.Low[mmIndex];
        Line.LineColor = extensionLines.SecondaryColor;
        Line.LineStyle = extensionLines.LineStyle;
        Line.LineWidth = extensionLines.LineWidth;

        sc.AddLineUntilFutureIntersectionEx(Line);
      }

      // reset flag to reset current trades only if price has not ALREADY broken
      // the last high while confirming the new low
      if (sc.GetHighest(sc.BaseDataIn[SC_HIGH], nBarsHL.GetInt()) <= lastHigh)
        lastHiLoBroken = 0;
    }
  }
}

SCSFExport scsf_FairValueGapsMultiTF(SCStudyInterfaceRef sc) {
  SCInputRef TFtype = sc.Input[0];
  SCInputRef TFmult = sc.Input[1];

  SCInputRef upFVGfills = sc.Input[21];
  SCInputRef downFVGfills = sc.Input[22];
  SCInputRef fillsTransparency = sc.Input[23];
  SCInputRef outlineWidth = sc.Input[24];
  SCInputRef upFVGlines = sc.Input[25];
  SCInputRef downFVGlines = sc.Input[26];
  SCInputRef showMidline = sc.Input[27];
  SCInputRef showNunfilled = sc.Input[28];
  SCInputRef showNfilled = sc.Input[29];

  // constant for sc.Input to specify the start of the list contanining the
  // desired higher timeframes
  const int TFlistStart = 10;

  // last calculated bar
  int &lastBar = sc.GetPersistentInt(-1);

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "Fair Value Gaps - MultiTimeFrame";
    sc.AutoLoop = 1;
    sc.GraphRegion = 0;
    sc.DrawStudyUnderneathMainPriceGraph = 1;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;

    TFtype.Name = "Timeframe to calculate FVGs for";
    TFtype.SetCustomInputStrings("Chart Timeframe;Higher TF from list > mult * chartTF");
    TFtype.SetCustomInputIndex(0);

    TFmult.Name = "---HTF: Chart TF multiplier: HTF > mult*chart TF";
    TFmult.SetInt(3);
    TFmult.SetIntLimits(1, 1000);

    sc.Input[TFlistStart].Name = "---HTF: First Intraday HTF (secs/vol/trades, 0=don't use)";
    sc.Input[TFlistStart].SetInt(60);
    sc.Input[TFlistStart].SetIntLimits(0, 1000000000);

    sc.Input[TFlistStart + 1].Name = "---HTF: Second Intraday HTF (secs/vol/trades, 0=don't use)";
    sc.Input[TFlistStart + 1].SetInt(300);
    sc.Input[TFlistStart + 1].SetIntLimits(0, 1000000000);

    sc.Input[TFlistStart + 2].Name = "---HTF: Third Intraday HTF (secs/vol/trades, 0=don't use)";
    sc.Input[TFlistStart + 2].SetInt(900);
    sc.Input[TFlistStart + 2].SetIntLimits(0, 1000000000);

    sc.Input[TFlistStart + 3].Name = "---HTF: Fourth Intraday HTF (secs/vol/trades, 0=don't use)";
    sc.Input[TFlistStart + 3].SetInt(1800);
    sc.Input[TFlistStart + 3].SetIntLimits(0, 1000000000);

    sc.Input[TFlistStart + 4].Name = "---HTF: Fifth Intraday HTF (secs/vol/trades, 0=don't use)";
    sc.Input[TFlistStart + 4].SetInt(3600);
    sc.Input[TFlistStart + 4].SetIntLimits(0, 1000000000);

    sc.Input[TFlistStart + 5].Name =
        "---HTF: If intraday chart TF is above all used intradays, HTF is";
    sc.Input[TFlistStart + 5].SetCustomInputStrings("Daily;Weekly;Monthly;Quarterly;Don't use");
    sc.Input[TFlistStart + 5].SetCustomInputIndex(0);

    sc.Input[TFlistStart + 6].Name = "---HTF: For daily charts HTF is";
    sc.Input[TFlistStart + 6].SetCustomInputStrings("Weekly;Monthly;Quarterly;Don't use");
    sc.Input[TFlistStart + 6].SetCustomInputIndex(0);

    sc.Input[TFlistStart + 7].Name = "---HTF: For weekly charts HTF is";
    sc.Input[TFlistStart + 7].SetCustomInputStrings("Monthly;Quarterly;Don't use");
    sc.Input[TFlistStart + 7].SetCustomInputIndex(0);

    sc.Input[TFlistStart + 8].Name = "---HTF: For monthly charts HTF is";
    sc.Input[TFlistStart + 8].SetCustomInputStrings("Quarterly;Yearly;Don't use");
    sc.Input[TFlistStart + 8].SetCustomInputIndex(0);

    upFVGfills.Name = "Up FVG FillColor";
    upFVGfills.SetColor(144, 191, 249);

    downFVGfills.Name = "Down FVG FillColor";
    downFVGfills.SetColor(244, 143, 177);

    fillsTransparency.Name = "Transparency for Fills";
    fillsTransparency.SetInt(90);

    outlineWidth.Name = "FVGs Outline Width (0=hidden)";
    outlineWidth.SetInt(1);

    upFVGlines.Name = "Up FVG LineColor";
    upFVGlines.SetColor(187, 216, 251);

    downFVGlines.Name = "Down FVG LineColor";
    downFVGlines.SetColor(250, 201, 217);

    showMidline.Name = "Show Midline";
    showMidline.SetYesNo(0);

    showNunfilled.Name = "How many Unfilled FVGs to show";
    showNunfilled.SetInt(10);
    showNunfilled.SetIntLimits(1, 1000);

    showNfilled.Name = "How many Filled FVGs to show";
    showNfilled.SetInt(100);
    showNfilled.SetIntLimits(0, 1000);

    // reset lastBar
    lastBar = -1;

    return;
  }

  // PERSISTENT VARIABLES
  // BUILD OUR OWN BARS
  // HIs => persistent 1(current bar) -> 3
  // LOs => persistent -1(current bar) -> -3
  float &bar1hi = sc.GetPersistentFloat(1);
  float &bar1lo = sc.GetPersistentFloat(-1);
  float &bar2hi = sc.GetPersistentFloat(2);
  float &bar2lo = sc.GetPersistentFloat(-2);
  float &bar3hi = sc.GetPersistentFloat(3);
  float &bar3lo = sc.GetPersistentFloat(-3);
  // indexes => persistent 1(current bar) -> 3
  int &bar1index = sc.GetPersistentInt(1);
  int &bar2index = sc.GetPersistentInt(2);
  int &bar3index = sc.GetPersistentInt(3);

  // timeframe vars
  int &chartType = sc.GetPersistentInt(4);
  int &chartTF = sc.GetPersistentInt(5);
  int &HTF = sc.GetPersistentInt(6);
  int &HTFbarCounter = sc.GetPersistentInt(7);
  int &resetTime = sc.GetPersistentInt(8);

  // untested levels
  float &nearestUntestedLevelBelow = sc.GetPersistentFloat(5);
  float &nearestUntestedLevelAbove = sc.GetPersistentFloat(-5);

  // where do GetPersistentInt() pointers to store IDs of filled and unfilled
  // FVGs start
  const int firstTested = 10;
  int firstUntested =
      firstTested + showNfilled.GetInt() + 10; // leave 10 spaces in the persistent variables
                                               // between filled and unfilled IDs just in case
  // GetPersistentInt() pointer for the ID of the next/coming UP || DOWN
  // unfilled FVG
  int &upFVGsNextID = sc.GetPersistentInt(firstTested - 1);
  int &downFVGsNextID = sc.GetPersistentInt(-(firstTested - 1));

  // full recalculation
  if (sc.Index == 0 || lastBar == -1) // This indicates a study is being recalculated.
  {
    // When there is a full recalculation of the study,
    // reset the persistent variables we are using
    nearestUntestedLevelAbove = 0;
    nearestUntestedLevelBelow = 0;
    upFVGsNextID = firstUntested;
    downFVGsNextID = -firstUntested;

    // reset our custom bars
    for (int x = 1; x <= 3; x++) {
      sc.GetPersistentFloat(x) = 0;
      sc.GetPersistentFloat(-x) = 0;
      sc.GetPersistentInt(x) = 0;
    }

    HTF = 0; // defaults to zero = no higher timeframe will be used
    // SET TIMEFRAME AGGREGATION IF WE ARE DISPLAYING FVGs FOR A HIGHER
    // TIMEFRAME
    if (TFtype.GetIndex() == 1) {
      chartType = 0; // used to check that the chart is time-based, volume-based
                     // || number-of-trades-based. If not, no intraday higher
                     // timeframe will be used, only from daily upwards
      // get period parameters
      n_ACSIL::s_BarPeriod BarPeriod;
      sc.GetBarPeriodParameters(BarPeriod);

      // set reset time for HTF bars: start of new day || session
      resetTime = sc.UseSecondStartEndTimes == 0 ? sc.StartTime1 : sc.StartTime2;

      // if intraday
      if (BarPeriod.ChartDataType == INTRADAY_DATA) {
        chartType = 10 + static_cast<int>(
                             BarPeriod.IntradayChartBarPeriodType); // 10= chart is time based,
                                                                    // 11=volume based, 12=number
                                                                    // of trades based

        // for intraday HTF calculations we only admit time, volume || number of
        // trades based charts
        if (chartType <= 12) {
          chartTF = BarPeriod.IntradayChartBarPeriodParameter1;

          // loop through the list of selected TFs to find the next higher
          // timeframe that is higher than TFmult*chartTF and that is evenly
          // divisible by the current chart timeframe
          for (int i = TFlistStart; i <= TFlistStart + 4; i++) {
            int nextTF = sc.Input[i].GetInt();
            if (nextTF > TFmult.GetInt() * chartTF && nextTF % chartTF == 0) {
              HTF = nextTF;
              break;
            }
          }
        }
        // if a higher timeframe hasn't been found because the chart TF is
        // higher than all intrdays or because the chart type is not one of the
        // admitted types refer to timeframe set in "Above all intraday
        // timeframes"
        if (HTF == 0) {
          if (sc.Input[TFlistStart + 5].GetIndex() == 0)
            HTF = -1; // daily
          if (sc.Input[TFlistStart + 5].GetIndex() == 1)
            HTF = -2; // weekly
          if (sc.Input[TFlistStart + 5].GetIndex() == 2)
            HTF = -3; // monthly
          if (sc.Input[TFlistStart + 5].GetIndex() == 3)
            HTF = -4; // quarterly
        }
      }
      // if chart is daily data
      else {
        if (BarPeriod.HistoricalChartBarPeriodType == HISTORICAL_CHART_PERIOD_DAYS) {
          if (sc.Input[TFlistStart + 6].GetIndex() == 0)
            HTF = -2; // weekly
          if (sc.Input[TFlistStart + 6].GetIndex() == 1)
            HTF = -3; // monthly
          if (sc.Input[TFlistStart + 6].GetIndex() == 2)
            HTF = -4; // quarterly
        }
        if (BarPeriod.HistoricalChartBarPeriodType == HISTORICAL_CHART_PERIOD_WEEKLY) {
          if (sc.Input[TFlistStart + 7].GetIndex() == 0)
            HTF = -3; // monthly
          if (sc.Input[TFlistStart + 7].GetIndex() == 1)
            HTF = -4; // quarterly
        }
        if (BarPeriod.HistoricalChartBarPeriodType == HISTORICAL_CHART_PERIOD_MONTHLY) {
          if (sc.Input[TFlistStart + 8].GetIndex() == 0)
            HTF = -4; // quarterly
          if (sc.Input[TFlistStart + 8].GetIndex() == 1)
            HTF = -5; // yearly
        }
      }
    }
  }

  //**** BEGIN
  // if chart bar just closed
  if (!sc.HideStudy && sc.Index != lastBar) {
    // check if the closed chart bar is the high || low of the HTF bar
    if (sc.High[sc.Index - 1] > bar1hi || bar1hi == 0)
      bar1hi = sc.High[sc.Index - 1];
    if (sc.Low[sc.Index - 1] < bar1lo || bar1lo == 0)
      bar1lo = sc.Low[sc.Index - 1];
    HTFbarCounter += chartTF; // add to the counter the number of seconds ||
                              // trades || volume that comprises this bar

    // IF A NEW BAR TO CALCULATE FVGs FOR HAS JUST COMPLETED
    if (TFtype.GetIndex() == 0               // we are using the chart timeframe
        || (HTF > 0 && HTFbarCounter >= HTF) // or the HTF bar has been completed
        || (HTF > 0 && chartType == 10 &&
            (sc.BaseDateTimeIn[sc.Index].GetTimeInSeconds() - resetTime) % HTF ==
                0) // or a new time-based HTF bar has just completed (time-based
                   // bars, specially seconds bars, sometimes are missing, and the
                   // HTF counter doesn't work. This way, it will)
        || (HTF == -1 && sc.BaseDateTimeIn[sc.Index].GetTimeInSeconds() - resetTime <
                             sc.BaseDateTimeIn[sc.Index - 1].GetTimeInSeconds() -
                                 resetTime) // or a new day has started
        || (HTF == -2 &&
            sc.BaseDateTimeIn[sc.Index].GetDayOfWeek() <
                sc.BaseDateTimeIn[sc.Index - 1].GetDayOfWeek()) // or a new week has started
        ||
        (HTF == -3 && sc.BaseDateTimeIn[sc.Index].GetMonth() !=
                          sc.BaseDateTimeIn[sc.Index - 1].GetMonth()) // or a new month has started
        || (HTF == -4 &&
            sc.BaseDateTimeIn[sc.Index].GetMonth() != sc.BaseDateTimeIn[sc.Index - 1].GetMonth() &&
            sc.BaseDateTimeIn[sc.Index - 1].GetMonth() % 3 == 0) // or a new quarter has started
        ||
        (HTF == -5 && sc.BaseDateTimeIn[sc.Index].GetYear() !=
                          sc.BaseDateTimeIn[sc.Index - 1].GetYear())) // or a new year has started
    {
      // IF TESTING NEAREST UNFILLED LEVELS ABOVE AND BELOW
      // if testing the nearest level above
      if (bar1hi >= nearestUntestedLevelAbove && bar2hi < nearestUntestedLevelAbove &&
          nearestUntestedLevelAbove != 0) {
        // loop through the unfilled DOWN-FVGs array
        for (int x = downFVGsNextID + 1; x <= -firstUntested; x++) {
          s_UseTool Rect;
          Rect.Clear();
          bool getLine = sc.GetACSDrawingByLineNumber(0, sc.GetPersistentInt(x), Rect);
          if (getLine) {
            double begin = Rect.BeginValue;
            // if this FVG has just been filled
            if (bar1hi >= begin && bar2hi < begin) {
              // modify rectangle to make it finish in this bar
              Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
              Rect.EndIndex = sc.Index - 1;
              Rect.AddMethod = UTAM_ADD_OR_ADJUST;
              sc.UseTool(Rect);

              // delete its ID from the untested array and decrement the
              // unfilled pointer
              sc.GetPersistentInt(x) = 0;
              nearestUntestedLevelAbove = 0;
              downFVGsNextID++;

              // if showing filled FVGs, delete filled FVGs older than
              // showNfilled
              if (showNfilled.GetInt() > 0) {
                for (int i = -firstTested; i > -firstTested - showNfilled.GetInt(); i--) {
                  if (i == -firstTested)
                    sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, sc.GetPersistentInt(i));
                  if (i > -firstTested - showNfilled.GetInt() + 1)
                    sc.GetPersistentInt(i) = sc.GetPersistentInt(i - 1);
                }

                // save the newly filled FVG ID to be able to delete it later
                sc.GetPersistentInt(-firstTested - showNfilled.GetInt() + 1) = Rect.LineNumber;
              }
              // if not, delete the newly filled gap
              else
                sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, Rect.LineNumber);

            }
            // show next untested showNunfilled FVGs
            else {
              if (x < downFVGsNextID + 1 + showNunfilled.GetInt()) {
                // show drawing
                Rect.HideDrawing = 0;
                Rect.AddMethod = UTAM_ADD_OR_ADJUST;
                sc.UseTool(Rect);

                // search for next untested level
                if (Rect.BeginValue < nearestUntestedLevelAbove || nearestUntestedLevelAbove == 0)
                  nearestUntestedLevelAbove = Rect.BeginValue;
              }
            }
          }
        }
      }

      // if testing the nearest level below
      if (bar1lo <= nearestUntestedLevelBelow && bar2lo > nearestUntestedLevelBelow &&
          nearestUntestedLevelBelow != 0) {
        // loop through the unfilled UP-FVGs array
        for (int x = upFVGsNextID - 1; x >= firstUntested; x--) {
          s_UseTool Rect;
          Rect.Clear();
          bool getLine = sc.GetACSDrawingByLineNumber(0, sc.GetPersistentInt(x), Rect);
          if (getLine) {
            double begin = Rect.BeginValue;
            // if this FVG has just been filled
            if (bar1lo <= begin && bar2lo > begin) {
              // modify rectangle to make it finish in this bar
              Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
              Rect.EndIndex = sc.Index - 1;
              Rect.AddMethod = UTAM_ADD_OR_ADJUST;
              sc.UseTool(Rect);

              // delete its ID from the untested array and decrement the
              // unfilled pointer
              sc.GetPersistentInt(x) = 0;
              nearestUntestedLevelBelow = 0;
              upFVGsNextID--;

              // if showing filled FVGs, delete filled FVGs older than
              // showNfilled
              if (showNfilled.GetInt() > 0) {
                for (int i = firstTested; i < firstTested + showNfilled.GetInt(); i++) {
                  if (i == firstTested)
                    sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, sc.GetPersistentInt(i));
                  if (i < firstTested + showNfilled.GetInt() - 1)
                    sc.GetPersistentInt(i) = sc.GetPersistentInt(i + 1);
                }

                // save the newly filled FVG ID to be able to delete it later
                sc.GetPersistentInt(firstTested + showNfilled.GetInt() - 1) = Rect.LineNumber;
              }
              // if not, delete the newly filled gap
              else
                sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, Rect.LineNumber);
            }
            // show next untested showNunfilled FVGs
            else {
              if (x > upFVGsNextID - 1 - showNunfilled.GetInt()) {
                // show drawing
                Rect.HideDrawing = 0;
                Rect.AddMethod = UTAM_ADD_OR_ADJUST;
                sc.UseTool(Rect);

                // search for next untested level
                if (Rect.BeginValue > nearestUntestedLevelBelow || nearestUntestedLevelBelow == 0)
                  nearestUntestedLevelBelow = Rect.BeginValue;
              }
            }
          }
        }
      }

      //**** LOOK FOR FAIR VALUE GAPS
      // GAPUP
      if (bar1lo > bar3hi && bar2lo < bar1lo && bar2hi >= bar1lo) {
        // define a s_UseTool variable
        s_UseTool Rect;
        Rect.Clear();

        // create level
        Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
        Rect.BeginIndex = bar3index;
        Rect.EndIndex = sc.Index - 1;
        // set the rectangle levels
        Rect.BeginValue = bar3hi;
        Rect.EndValue = bar1lo;
        Rect.Color = upFVGlines.GetColor();
        Rect.SecondaryColor = upFVGfills.GetColor();
        Rect.TransparencyLevel = fillsTransparency.GetInt();
        Rect.NoVerticalOutline = 1;
        Rect.LineWidth = outlineWidth.GetInt();
        Rect.DrawMidline = showMidline.GetYesNo();

        sc.UseTool(Rect);

        // save FVG ID in untested and its level in nearestGap
        sc.GetPersistentInt(upFVGsNextID) = Rect.LineNumber;
        nearestUntestedLevelBelow = bar3hi;

        // hide previous untested FVGs
        if (upFVGsNextID >= firstUntested + showNunfilled.GetInt()) {
          Rect.Clear();
          bool getLine = sc.GetACSDrawingByLineNumber(
              0, sc.GetPersistentInt(upFVGsNextID - showNunfilled.GetInt()), Rect);
          Rect.HideDrawing = 1;
          Rect.AddMethod = UTAM_ADD_OR_ADJUST;
          sc.UseTool(Rect);
        }

        // increment untested IDs counter
        upFVGsNextID++;
      }

      // GAPDOWN
      if (bar1hi < bar3lo && bar2hi > bar1hi && bar2lo <= bar1hi) {
        // define a s_UseTool variable
        s_UseTool Rect;
        Rect.Clear();

        // create level
        Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
        Rect.BeginIndex = bar3index;
        Rect.EndIndex = sc.Index - 1;
        // set the rectangle levels
        Rect.BeginValue = bar3lo;
        Rect.EndValue = bar1hi;
        Rect.Color = downFVGlines.GetColor();
        Rect.SecondaryColor = downFVGfills.GetColor();
        Rect.TransparencyLevel = fillsTransparency.GetInt();
        Rect.NoVerticalOutline = 1;
        Rect.LineWidth = outlineWidth.GetInt();
        Rect.DrawMidline = showMidline.GetYesNo();

        sc.UseTool(Rect);

        // save FVG ID in untested and its level in nearestGap
        sc.GetPersistentInt(downFVGsNextID) = Rect.LineNumber;
        nearestUntestedLevelAbove = bar3lo;

        // hide previous untested FVGs
        if (downFVGsNextID <= -firstUntested - showNunfilled.GetInt()) {
          Rect.Clear();
          bool getLine = sc.GetACSDrawingByLineNumber(
              0, sc.GetPersistentInt(downFVGsNextID + showNunfilled.GetInt()), Rect);
          Rect.HideDrawing = 1;
          Rect.AddMethod = UTAM_ADD_OR_ADJUST;
          sc.UseTool(Rect);
        }

        // increment untested IDs counter
        downFVGsNextID--;
      }

      //*** CALCULATIONS ON THE HTF BAR HAVE BEEN COMPLETED, PREPARE A NEW HTF
      // BAR roll HTFbars values
      bar3hi = bar2hi;
      bar2hi = bar1hi;
      bar1hi = 0;
      bar3lo = bar2lo;
      bar2lo = bar1lo;
      bar1lo = 0;
      bar3index = bar2index;
      bar2index = bar1index;
      bar1index = sc.Index;
      // reset the HTF counter
      HTFbarCounter = 0;
    }
  }

  // save Index
  lastBar = sc.Index;
}

SCSFExport scsf_AutoVbP(SCStudyInterfaceRef sc) {
  SCInputRef useVap = sc.Input[0];
  // multiplier that will determine the granularity of VP bars we'll see as we
  // switch symbols the lower the number, the thicker and fewer bars there will
  // be the higher the magic number, the thinner and more VP bars there will be
  SCInputRef i_DetailLevel = sc.Input[1];
  SCInputRef i_Step = sc.Input[2];

  // number of inputs to set target VBP studies
  const int MAX_VBP_STUDIES = 8;

  // Configuration
  if (sc.SetDefaults) {
    sc.GraphRegion = 0;
    sc.GraphName = "Auto set VbPs ticks per bar";
    sc.AutoLoop = 0;
    // update always so we don't have to wait for incoming ticks (market closed)
    sc.UpdateAlways = 1;

    // INPUTS CONFIG
    useVap.Name = "Use VAP setting in Chart Settings";
    useVap.SetYesNo(0);
    i_DetailLevel.Name = "VbPs' detail level";
    i_DetailLevel.SetInt(100);
    i_Step.Name = "Tick-size step to increase/decrease detail";
    i_Step.SetInt(4);

    // target TPO study
    sc.Input[9].Name = "Target TPO study";
    sc.Input[9].SetStudyID(0);

    // target VPO studies
    for (int x = 0; x < MAX_VBP_STUDIES; x++) {
      sc.Input[10 + x].Name.Format("%i.Target VbP study", x + 1);
      sc.Input[10 + x].SetStudyID(0);
    }

    return;
  }

  if (sc.IsFullRecalculation) {
    // set step to the nearest multiplier of sc.VolumeAtPriceMultiplier to avoid
    // VbP redraw problems
    int step = i_Step.GetInt();
    i_Step.SetInt(max(step - step % sc.VolumeAtPriceMultiplier, sc.VolumeAtPriceMultiplier));
  }

  // VbP Ticks Per Volume Bar is input 32, ID 31
  int inputIdx = 31;

  // if setting VbP's Ticks-per-bar according to Volume At Price Multiplier in
  // ChartSettings
  if (useVap.GetInt() == 1) {

    for (int x = 0; x < MAX_VBP_STUDIES; x++) {
      int studyId = sc.Input[10 + x].GetStudyID();
      if (studyId != 0) {
        int prevValue;
        sc.GetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, prevValue);
        if (prevValue != sc.VolumeAtPriceMultiplier)
          sc.SetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, sc.VolumeAtPriceMultiplier);
      }
    }

    return;
  }

  // if using detail level specified in inputs
  float vHigh, vLow; //, vDiff;
  int vDiff;
  int detail = i_DetailLevel.GetInt();

  // fetch the graph's price scale's high and low value so we can automate the
  // Ticks setting on VbP
  sc.GetMainGraphVisibleHighAndLow(vHigh, vLow);

  // calc the range of visible prices
  // DIVIDE BY TICK-SIZE TO GET NUMBER OF VISIBLE TICKS
  // vDiff = (vHigh - vLow) / sc.TickSize;
  vDiff = sc.Round((vHigh - vLow) / sc.TickSize);

  // divide range by detail level to get the desired VbP Ticks Per Bar value and
  // don't allow it to be less than the VAP multiplier
  // int targetTicksPerBar = max(sc.Round(vDiff / detail),
  // sc.VolumeAtPriceMultiplier);
  int targetTicksPerBar = sc.Round(vDiff / detail);

  // adapt targetTicksPerBar to the specified step
  if (targetTicksPerBar >= i_Step.GetInt())
    targetTicksPerBar -= targetTicksPerBar % i_Step.GetInt();
  // if targetTicksPerBar is less than the step but greater than VAP multiplier,
  // then modify it as a VAP multiplier multiple else if(targetTicksPerBar >
  // sc.VolumeAtPriceMultiplier) 	targetTicksPerBar -= targetTicksPerBar %
  // sc.VolumeAtPriceMultiplier;
  else
    targetTicksPerBar = sc.VolumeAtPriceMultiplier;

  // flag to redraw chart in case we change any target-VbP
  bool redraw = false;

  // set ticks for TPO study
  int studyId = sc.Input[9].GetStudyID();
  if (studyId != 0) {
    int prevValue;
    sc.GetChartStudyInputInt(sc.ChartNumber, studyId, 0, prevValue);

    if (targetTicksPerBar != prevValue) {
      sc.SetChartStudyInputInt(sc.ChartNumber, studyId, 0, targetTicksPerBar);
      redraw = true;
    }
  }

  // set ticks for VbP studies
  for (int x = 0; x < MAX_VBP_STUDIES; x++) {
    int studyId = sc.Input[10 + x].GetStudyID();
    if (studyId != 0) {
      int prevValue;
      sc.GetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, prevValue);

      if (targetTicksPerBar != prevValue) {
        sc.SetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, targetTicksPerBar);
        redraw = true;
      }
    }
  }

  if (redraw)
    sc.RecalculateChart(sc.ChartNumber);
}

SCSFExport scsf_StopsOnChart(SCStudyInterfaceRef sc) {
  SCSubgraphRef StopBuys = sc.Subgraph[0];
  SCSubgraphRef StopSells = sc.Subgraph[1];

  SCInputRef nTicks = sc.Input[0];

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "Stops drawing";
    sc.StudyDescription = "";
    sc.AutoLoop = 0;
    sc.GraphRegion = 0;

    StopBuys.Name = "Stopfor Buys";
    StopBuys.DrawStyle = DRAWSTYLE_RIGHT_SIDE_TICK_SIZE_RECTANGLE;
    StopBuys.PrimaryColor = RGB(0, 255, 255);
    StopBuys.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
    StopBuys.ShortName = "STOP";
    StopBuys.DrawZeros = 0;
    StopBuys.LineWidth = 2;

    StopSells.Name = "Stopfor Buys";
    StopSells.DrawStyle = DRAWSTYLE_RIGHT_SIDE_TICK_SIZE_RECTANGLE;
    StopSells.PrimaryColor = RGB(0, 255, 255);
    StopSells.LineLabel =
        LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
    StopSells.ShortName = "STOP";
    StopSells.DrawZeros = 0;
    StopSells.LineWidth = 2;

    nTicks.Name = "Stop in ticks";
    nTicks.SetInt(12);
  }

  int &lastIndex = sc.GetPersistentInt(0);
  int i = sc.UpdateStartIndex;

  // on start, set subgraphs names
  if (sc.IsFullRecalculation && i == 0) {
    StopBuys.ShortName.Format("-%i T", nTicks.GetInt());
    StopSells.ShortName.Format("%i T", nTicks.GetInt());
  }

  // if new candle, copy last values
  if (sc.UpdateStartIndex != lastIndex) {
    StopSells[i] = StopSells[i - 1];
    StopBuys[i] = StopBuys[i - 1];
    StopSells[i - 1] = 0;
    StopBuys[i - 1] = 0;
  }

  StopSells[i] = sc.Bid + nTicks.GetInt() * sc.TickSize;
  StopBuys[i] = sc.Ask - nTicks.GetInt() * sc.TickSize;

  lastIndex = i;
}

SCSFExport scsf_BetterDOM(SCStudyInterfaceRef sc) {
  // old hi&los will be automatically created and numbered from subgraph[40]
  // backwards, copying the properties specified for the prevHis and prevLos
  // subgraphs ([42] and [43])
  SCSubgraphRef hiGraph = sc.Subgraph[40];
  SCSubgraphRef loGraph = sc.Subgraph[41];
  SCSubgraphRef prevHisGraphs = sc.Subgraph[42];
  SCSubgraphRef prevLosGraphs = sc.Subgraph[43];

  SCSubgraphRef swingDataGraph = sc.Subgraph[54];
  SCSubgraphRef askSizeGraph = sc.Subgraph[55];
  SCSubgraphRef bidSizeGraph = sc.Subgraph[56];
  SCSubgraphRef cumPSgraph = sc.Subgraph[57];
  SCSubgraphRef cumDeltaGraph = sc.Subgraph[58];
  SCSubgraphRef cumSizeGraph = sc.Subgraph[59];

  SCInputRef nTicks = sc.Input[0];
  SCInputRef clear = sc.Input[1];
  SCInputRef nHiLos = sc.Input[2];
  SCInputRef showSizes = sc.Input[3];
  SCInputRef cumSizeColoring = sc.Input[4];
  SCInputRef showPSinLabel = sc.Input[5];
  SCInputRef whereDisplayData = sc.Input[6];
  SCInputRef dataToDisplay = sc.Input[7];
  SCInputRef deltaType = sc.Input[8];
  SCInputRef showMaxMin = sc.Input[9];
  SCInputRef noDataDisplay = sc.Input[10];
  SCInputRef interpColorsOldHiLos = sc.Input[11];
  SCInputRef opaqueBackground = sc.Input[12];
  SCInputRef colorSwingData = sc.Input[13];
  SCInputRef fontSizeSwingData = sc.Input[14];
  SCInputRef fontSizeLastHiLo = sc.Input[15];
  SCInputRef fontSizeOldHiLos = sc.Input[16];
  SCInputRef spacing = sc.Input[17];

  // 0 decimal places for volume data display
  sc.ValueFormat = 0;

  if (sc.SetDefaults) {
    // SUBGRAPHS
    sc.GraphName = "Better DOM";
    sc.StudyDescription = "Indicator shows new swing Highs and Lows on the DOM "
                          "label column along with volume, delta and "
                          "cumulative pulling&stacking values for each swing";
    sc.AutoLoop = 0;
    sc.GraphRegion = 0;

    hiGraph.Name = "Last High";
    hiGraph.DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
    hiGraph.LineWidth = 10;
    hiGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
    hiGraph.PrimaryColor = RGB(255, 255, 0);
    hiGraph.DrawZeros = 0;
    hiGraph.ShortName = "HI";

    loGraph.Name = "Last Low";
    loGraph.DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
    loGraph.LineWidth = 10;
    loGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
    loGraph.PrimaryColor = RGB(255, 255, 0);
    loGraph.DrawZeros = 0;
    loGraph.ShortName = "LO";

    cumSizeGraph.Name = "Cumulative Size";
    cumSizeGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    cumSizeGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN |
                             LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
    cumSizeGraph.PrimaryColor = RGB(255, 255, 255);
    cumSizeGraph.SecondaryColor = RGB(255, 255, 255);
    cumSizeGraph.SecondaryColorUsed = 1;

    cumDeltaGraph.Name = "Delta per Price";
    cumDeltaGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    cumDeltaGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN |
                              LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
    cumDeltaGraph.PrimaryColor = RGB(100, 255, 100);
    cumDeltaGraph.SecondaryColor = RGB(255, 100, 100);
    cumDeltaGraph.SecondaryColorUsed = 1;
    cumDeltaGraph.AutoColoring = AUTOCOLOR_POSNEG;
    cumPSgraph.DrawZeros = 1;

    cumPSgraph.Name = "Pulling & Stacking per Price";
    cumPSgraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    cumPSgraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN |
                           LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
    cumPSgraph.PrimaryColor = RGB(100, 255, 100);
    cumPSgraph.SecondaryColor = RGB(255, 100, 100);
    cumPSgraph.SecondaryColorUsed = 1;
    cumPSgraph.AutoColoring = AUTOCOLOR_POSNEG;
    cumPSgraph.DrawZeros = 1;

    askSizeGraph.Name = "Recent Ask Size/Up color";
    askSizeGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    askSizeGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN |
                             LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
    askSizeGraph.PrimaryColor = RGB(100, 255, 100);

    bidSizeGraph.Name = "Recent Bid Size/Down color";
    bidSizeGraph.DrawStyle = DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
    bidSizeGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN |
                             LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
    bidSizeGraph.PrimaryColor = RGB(255, 100, 100);

    swingDataGraph.Name = "Current Swing's Data";
    swingDataGraph.DrawStyle = DRAWSTYLE_TEXT;
    swingDataGraph.LineLabel =
        LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
    swingDataGraph.LineWidth = 10;
    swingDataGraph.PrimaryColor = RGB(255, 255, 255);

    prevHisGraphs.Name = "Previous Highs";
    prevHisGraphs.DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
    prevHisGraphs.LineWidth = 10;
    prevHisGraphs.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN;
    prevHisGraphs.PrimaryColor = RGB(200, 0, 0);
    prevHisGraphs.SecondaryColor = RGB(100, 0, 0);
    prevHisGraphs.SecondaryColorUsed = 1;
    prevHisGraphs.DrawZeros = 0;
    prevHisGraphs.ShortName = "hi";

    prevLosGraphs.Name = "Previous Lows";
    prevLosGraphs.DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
    prevLosGraphs.LineWidth = 10;
    prevLosGraphs.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN;
    prevLosGraphs.PrimaryColor = RGB(0, 200, 0);
    prevLosGraphs.SecondaryColor = RGB(0, 100, 0);
    prevLosGraphs.SecondaryColorUsed = 1;
    prevLosGraphs.DrawZeros = 0;
    prevLosGraphs.ShortName = "lo";

    // INPUTS
    nTicks.Name = "Number of ticks to confirm new swing Hi&Lo";
    nTicks.SetInt(12);

    clear.Name = "Clear Current Trades on new confirmed Hi/Lo";
    clear.SetYesNo(1);

    showSizes.Name = "Show Last Sizes";
    showSizes.SetCustomInputStrings("No;Cumulative Last Size;Cum.Size + DeltaPerPrice;Only "
                                    "DeltaPerPrice;Bid/Ask sizes");
    showSizes.SetCustomInputIndex(2);

    cumSizeColoring.Name = "Cumulative Size Coloring";
    cumSizeColoring.SetCustomInputStrings("Subgraph color;Uptick/Downtick;Delta per Price");
    cumSizeColoring.SetCustomInputIndex(2);

    showPSinLabel.Name = "Show Pulling&Stacking per price in Label Column";
    showPSinLabel.SetYesNo(1);

    whereDisplayData.Name = "Where to display swing data";
    whereDisplayData.SetCustomInputStrings(
        "None;Only Current Swing;Curr.Swing and Only Last High & "
        "Low;Curr.Swing and All Highs and Lows");
    whereDisplayData.SetCustomInputIndex(2);

    opaqueBackground.Name = "Opaque text background for data display in";
    opaqueBackground.SetCustomInputStrings(
        "None;Only Current Swing;Curr.Swing and Only Last High & "
        "Low;Curr.Swing and All Highs and Lows");
    opaqueBackground.SetCustomInputIndex(0);

    noDataDisplay.Name = "Display Highs & Lows with no data as";
    noDataDisplay.SetCustomInputStrings("Set in Subgraphs tab;Up/Down Triangles of fontSize");
    noDataDisplay.SetCustomInputIndex(1);

    dataToDisplay.Name = "Swing data to display";
    dataToDisplay.SetCustomInputStrings("Volume;V + Delta;V + D + Pulling&Stacking");
    dataToDisplay.SetCustomInputIndex(2);

    deltaType.Name = "Delta Format";
    deltaType.SetCustomInputStrings("Absolute Value;Volume Percentage");
    deltaType.SetCustomInputIndex(0);

    spacing.Name = "Vertical spacing between data in ticks";
    spacing.SetInt(1);

    nHiLos.Name = "Number of previous Hi&Los to display";
    nHiLos.SetInt(5);

    interpColorsOldHiLos.Name = "Interpolate Primary/Secondary colors for Old Hi&Los";
    interpColorsOldHiLos.SetYesNo(1);

    showMaxMin.Name = "Show current swing's max/min Delta and P&S values";
    showMaxMin.SetYesNo(1);

    colorSwingData.Name = "Color Swing Data acc. to swing delta (w Bid/Ask colors)";
    colorSwingData.SetYesNo(1);

    fontSizeSwingData.Name = "Font size for swing data";
    fontSizeSwingData.SetInt(12);

    fontSizeLastHiLo.Name = "Font size for Last High and Low";
    fontSizeLastHiLo.SetInt(10);

    fontSizeOldHiLos.Name = "Font size for Old Highs and Lows";
    fontSizeOldHiLos.SetInt(8);
  }

  // SET REFERENCES TO PERSISTENT VARIABLES
  float &newLo = sc.GetPersistentFloat(0);
  float &newHi = sc.GetPersistentFloat(1);
  float &lastTradedPrice = sc.GetPersistentFloat(2);
  float &lastTradedBid = sc.GetPersistentFloat(3);
  float &lastTradedAsk = sc.GetPersistentFloat(4);
  float &lastAsk = sc.GetPersistentFloat(5);
  float &lastBid = sc.GetPersistentFloat(6);

  int &lastIndex = sc.GetPersistentInt(0);
  int &swingPS = sc.GetPersistentInt(1);
  int &newPS = sc.GetPersistentInt(2);
  int &swingMaxPS = sc.GetPersistentInt(3);
  int &swingMinPS = sc.GetPersistentInt(4);
  int &newMaxPS = sc.GetPersistentInt(5);
  int &newMinPS = sc.GetPersistentInt(6);
  int &origColorCumSize = sc.GetPersistentInt(7);
  int &cmB = sc.GetPersistentInt(8);
  int &cmA = sc.GetPersistentInt(9);
  // int& cmPS = sc.GetPersistentInt(10);
  int &swingVol = sc.GetPersistentInt(11);
  int &swingDelta = sc.GetPersistentInt(12);
  int &newVol = sc.GetPersistentInt(13);
  int &newDelta = sc.GetPersistentInt(14);
  int &swingMaxDelta = sc.GetPersistentInt(15);
  int &swingMinDelta = sc.GetPersistentInt(16);
  int &newMaxDelta = sc.GetPersistentInt(17);
  int &newMinDelta = sc.GetPersistentInt(18);

  int64_t &LastProcessedSequence = sc.GetPersistentInt64(0);

  // set some variables
  int i = sc.UpdateStartIndex;
  float lineSpacing = spacing.GetInt() * sc.TickSize / 100;

  // Get the Time and Sales
  c_SCTimeAndSalesArray TimeSales;
  sc.GetTimeAndSales(TimeSales);
  if (TimeSales.Size() == 0)
    return; // No Time and Sales data available for the symbol

  // on start, configure all subgraphs acc. to inputs and create enough
  // subgraphs for nHiLos to display
  if (sc.IsFullRecalculation && i == 0) {
    // save original color for the cumulative size subgraph if it is set with
    // tick up/down coloring
    if (cumSizeColoring.GetIndex() == 1)
      origColorCumSize = cumSizeGraph.PrimaryColor;

    // set highs and lows subgraphs
    if (noDataDisplay.GetIndex() > 0) {
      prevHisGraphs.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
      prevHisGraphs.LineWidth = fontSizeOldHiLos.GetInt();
      prevLosGraphs.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
      prevLosGraphs.LineWidth = fontSizeOldHiLos.GetInt();

      hiGraph.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
      hiGraph.LineWidth = fontSizeLastHiLo.GetInt();
      loGraph.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
      loGraph.LineWidth = fontSizeLastHiLo.GetInt();
    }

    if (whereDisplayData.GetIndex() >= 2) {
      hiGraph.DrawStyle =
          opaqueBackground.GetIndex() >= 2 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
      hiGraph.LineWidth = fontSizeLastHiLo.GetInt();
      loGraph.DrawStyle =
          opaqueBackground.GetIndex() >= 2 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
      loGraph.LineWidth = fontSizeLastHiLo.GetInt();
      if (whereDisplayData.GetIndex() == 3) {
        prevHisGraphs.DrawStyle =
            opaqueBackground.GetIndex() == 3 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
        prevHisGraphs.LineWidth = fontSizeOldHiLos.GetInt();
        prevLosGraphs.DrawStyle =
            opaqueBackground.GetIndex() == 3 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
        prevLosGraphs.LineWidth = fontSizeOldHiLos.GetInt();
      }
    }

    // set current swing data subgraph
    if (whereDisplayData.GetIndex() > 0) {
      swingDataGraph.DrawStyle =
          opaqueBackground.GetIndex() >= 1 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
      swingDataGraph.LineWidth = fontSizeSwingData.GetInt();
    }

    // create enough subgraphs for old Highs and lows on the basis of the
    // "previous" subgraphs
    for (int x = 1; x < 21; x++) {
      if (x < nHiLos.GetInt()) {
        // new subgraphs for previous highs
        sc.Subgraph[40 - 2 * x].Name.Format("High %i", x);
        sc.Subgraph[40 - 2 * x].DrawStyle = prevHisGraphs.DrawStyle;
        sc.Subgraph[40 - 2 * x].LineWidth = prevHisGraphs.LineWidth;
        sc.Subgraph[40 - 2 * x].LineLabel = prevHisGraphs.LineLabel;
        sc.Subgraph[40 - 2 * x].PrimaryColor =
            interpColorsOldHiLos.GetYesNo()
                ? sc.RGBInterpolate(prevHisGraphs.PrimaryColor, prevHisGraphs.SecondaryColor,
                                    (float)(x - 1) / (nHiLos.GetInt() - 2))
                : prevHisGraphs.PrimaryColor;
        sc.Subgraph[40 - 2 * x].DrawZeros = 0;
        sc.Subgraph[40 - 2 * x].ShortName.Format(prevHisGraphs.ShortName + " %i", x);

        // new subgraphs for previous lows
        sc.Subgraph[41 - 2 * x].Name.Format("Low %i", x);
        sc.Subgraph[41 - 2 * x].DrawStyle = prevLosGraphs.DrawStyle;
        sc.Subgraph[41 - 2 * x].LineWidth = prevLosGraphs.LineWidth;
        sc.Subgraph[41 - 2 * x].LineLabel = prevLosGraphs.LineLabel;
        sc.Subgraph[41 - 2 * x].PrimaryColor =
            interpColorsOldHiLos.GetYesNo()
                ? sc.RGBInterpolate(prevLosGraphs.PrimaryColor, prevLosGraphs.SecondaryColor,
                                    (float)(x - 1) / (nHiLos.GetInt() - 2))
                : prevLosGraphs.PrimaryColor;
        sc.Subgraph[41 - 2 * x].DrawZeros = 0;
        sc.Subgraph[41 - 2 * x].ShortName.Format(prevLosGraphs.ShortName + " %i", x);
      } else {
        // do not show possible hi&los subgraphs if we are now showing a lesser
        // number of them
        sc.Subgraph[40 - 2 * x].Name = "";
        sc.Subgraph[41 - 2 * x].Name = "";
      }
    }

    // if cum size primary color has been changed because of the cum.size
    // coloring method, revert it back to the backeup secondary color
    cumSizeGraph.PrimaryColor = cumSizeGraph.SecondaryColor;

    // set on first calculation
    LastProcessedSequence = 0;
    lastIndex = sc.ArraySize - 1;
    newLo = TimeSales[0].Bid;
    newHi = 0;
    // set name for swing data subgraph's label: it will be making new possible
    // lows
    swingDataGraph.ShortName = "t.lo";
  }

  // calculate only in real time
  else {
    // if new candle, copy last subgraph values
    if (i != lastIndex)
      for (int x = 0; x < 60; x++) {
        sc.Subgraph[x].Data[i] = sc.Subgraph[x].Data[i - 1];
        sc.Subgraph[x].Arrays[0][i] = sc.Subgraph[x].Arrays[0][i - 1];
        sc.Subgraph[x].Data[i - 1] = 0;
        sc.Subgraph[x].Arrays[0][i - 1] = 0;
      }

    // READ T&S FOR VOLUME, DELTA AND PULLING & STACKING
    // first find first to calculate in this update
    int TSIndex = TimeSales.Size() - 1;
    for (TSIndex; TSIndex > 0; TSIndex--) {
      if (TimeSales[TSIndex - 1].Sequence <= LastProcessedSequence)
        break;
    }

    // now, iterate over the Time&Sales array
    int bidV = 0, askV = 0, updatePS = 0;
    float updateTradedPrice = 0, updateTradedBid = 0, updateTradedAsk = 0;

    for (TSIndex; TSIndex < TimeSales.Size(); TSIndex++) {
      // if there haven't been any new T&S records, do not reprocess the last
      // one
      if (TimeSales[TSIndex].Sequence <= LastProcessedSequence)
        continue;

      // flag to check if price has changed in this iteration
      float prevPrice = updateTradedPrice;

      // ADD UP ALL BID VOLUME DURING THIS UPDATE AND SAVE LAST PRICES
      if (TimeSales[TSIndex].Type == SC_TS_BID) {
        bidV += TimeSales[TSIndex].Volume;
        updateTradedBid = TimeSales[TSIndex].Bid * sc.RealTimePriceMultiplier;
        updateTradedPrice = updateTradedBid;

        swingVol += TimeSales[TSIndex].Volume;
        newVol += TimeSales[TSIndex].Volume;
        swingDelta -= TimeSales[TSIndex].Volume;
        newDelta -= TimeSales[TSIndex].Volume;
      }

      // ADD UP ALL ASK VOLUME DURING THIS UPDATE AND SAVE LAST PRICES
      if (TimeSales[TSIndex].Type == SC_TS_ASK) {
        askV += TimeSales[TSIndex].Volume;
        updateTradedAsk = TimeSales[TSIndex].Ask * sc.RealTimePriceMultiplier;
        updateTradedPrice = updateTradedAsk;

        swingVol += TimeSales[TSIndex].Volume;
        newVol += TimeSales[TSIndex].Volume;
        swingDelta += TimeSales[TSIndex].Volume;
        newDelta += TimeSales[TSIndex].Volume;
      }

      // calculate max&min delta for this swing and for potential new swing
      if (showMaxMin.GetYesNo())
        if (TimeSales[TSIndex].Type == SC_TS_ASK || TimeSales[TSIndex].Type == SC_TS_BID) {
          if (swingDelta > swingMaxDelta || swingMaxDelta == 0)
            swingMaxDelta = swingDelta;
          if (swingDelta < swingMinDelta || swingMinDelta == 0)
            swingMinDelta = swingDelta;
          if (newDelta > newMaxDelta || newMaxDelta == 0)
            newMaxDelta = newDelta;
          if (newDelta < newMinDelta || newMinDelta == 0)
            newMinDelta = newDelta;
        }

      // PULLING & STACKING
      if (TimeSales[TSIndex].Type == SC_TS_BIDASKVALUES &&
          TimeSales[TSIndex - 1].Type == SC_TS_BIDASKVALUES) {
        int ps = 0;

        if (TimeSales[TSIndex].Bid == TimeSales[TSIndex - 1].Bid)
          ps += (TimeSales[TSIndex].BidSize - TimeSales[TSIndex - 1].BidSize);
        else if (TimeSales[TSIndex].Bid > TimeSales[TSIndex - 1].Bid)
          ps += TimeSales[TSIndex].BidSize;
        else
          ps -= TimeSales[TSIndex - 1].BidSize;

        if (TimeSales[TSIndex].Ask == TimeSales[TSIndex - 1].Ask)
          ps -= (TimeSales[TSIndex].AskSize - TimeSales[TSIndex - 1].AskSize);
        else if (TimeSales[TSIndex].Ask < TimeSales[TSIndex - 1].Ask)
          ps -= TimeSales[TSIndex].AskSize;
        else
          ps += TimeSales[TSIndex - 1].AskSize;

        updatePS += ps;
        swingPS += ps;
        newPS += ps;

        // calculate max&min P&S values for this swing and for potential new
        // swing if(maxMinPSGraph.DrawStyle != DRAWSTYLE_HIDDEN &&
        // maxMinPSGraph.DrawStyle != DRAWSTYLE_IGNORE)
        if (showMaxMin.GetYesNo()) {
          if (swingPS > swingMaxPS || swingMaxPS == 0)
            swingMaxPS = swingPS;
          if (swingPS < swingMinPS || swingMinPS == 0)
            swingMinPS = swingPS;
          if (newPS > newMaxPS || newMaxPS == 0)
            newMaxPS = newPS;
          if (newPS < newMinPS || newMinPS == 0)
            newMinPS = newPS;
        }
      }

      // HAS A NEW HIGH || LOW BEEN CONFIRMED?
      if (updateTradedPrice != prevPrice && updateTradedPrice != 0) {
        // if there is a new confirmed high
        if (updateTradedPrice <= (newHi - nTicks.GetInt() * sc.TickSize) && newLo == 0 &&
            newHi != 0) {
          // new confirmed Hi
          // move highs to previous highs subgraphs and change their number
          // display
          for (int x = nHiLos.GetInt() - 1; x > 0; x--) {
            sc.Subgraph[40 - 2 * x][i] = sc.Subgraph[40 - 2 * (x - 1)][i];
            SCString prevHiLoData = sc.Subgraph[40 - 2 * (x - 1)].TextDrawStyleText;
            sc.Subgraph[40 - 2 * x].TextDrawStyleText.Format(
                "high %i%s", x,
                prevHiLoData.GetSubString(100, prevHiLoData.IndexOf('\n', 0)).GetChars());
          }

          // set subgraph
          hiGraph[i] = newHi;
          SCString swingData = "HIGH";

          swingData.AppendFormat("\nV: %i", swingVol - newVol);
          if (dataToDisplay.GetIndex() > 0) {
            if (deltaType.GetIndex() == 0)
              swingData.AppendFormat("\nD: %i", swingDelta - newDelta);
            else
              swingData.AppendFormat(
                  "\nD: %i%%", (int)(100 * (swingDelta - newDelta) / ((float)(swingVol - newVol))));
            if (dataToDisplay.GetIndex() == 2)
              swingData.AppendFormat("\nPS: %i", swingPS - newPS);
          }

          hiGraph.TextDrawStyleText = swingData;

          // new swing starts: new swing's values count from the now confirmed
          // high
          swingVol = newVol;
          swingDelta = newDelta;
          swingMaxDelta = newMaxDelta;
          swingMinDelta = newMinDelta;
          swingPS = newPS;
          swingMaxPS = newMaxPS;
          swingMinPS = newMinPS;

          // reset "new" variables to look for next lo
          newLo = updateTradedPrice;
          newHi = 0;
          newVol = 0;
          newDelta = 0;
          newMaxDelta = 0;
          newMinDelta = 0;
          newPS = 0;
          newMaxPS = 0;
          newMinPS = 0;

          // set name for swing data subgraph's label: it will be making new
          // possible lows
          swingDataGraph.ShortName = "t.lo";

          // clear current trades
          if (clear.GetYesNo())
            sc.ClearCurrentTradedBidAskVolume();
        } else
          // if there is a new confirmed low
          if (updateTradedPrice >= (newLo + nTicks.GetInt() * sc.TickSize) && newHi == 0 &&
              newLo != 0) {
            // new confirmed Lo
            for (int x = nHiLos.GetInt() - 1; x > 0; x--) {
              sc.Subgraph[41 - 2 * x][i] = sc.Subgraph[41 - 2 * (x - 1)][i];
              SCString prevHiLoData = sc.Subgraph[41 - 2 * (x - 1)].TextDrawStyleText;
              sc.Subgraph[41 - 2 * x].TextDrawStyleText.Format(
                  "low %i%s", x,
                  prevHiLoData.GetSubString(100, prevHiLoData.IndexOf('\n', 0)).GetChars());
            }
            // set subgraph
            loGraph[i] = newLo;
            SCString swingData = "LOW";

            swingData.AppendFormat("\nV: %i", swingVol - newVol);
            if (dataToDisplay.GetIndex() > 0) {
              if (deltaType.GetIndex() == 0)
                swingData.AppendFormat("\nD: %i", swingDelta - newDelta);
              else
                swingData.AppendFormat("\nD: %i%%", (int)(100 * (swingDelta - newDelta) /
                                                          ((float)(swingVol - newVol))));
              if (dataToDisplay.GetIndex() == 2)
                swingData.AppendFormat("\nPS: %i", swingPS - newPS);
            }

            loGraph.TextDrawStyleText = swingData;

            // new swing: new values count from the now confirmed low
            swingVol = newVol;
            swingDelta = newDelta;
            swingMaxDelta = newMaxDelta;
            swingMinDelta = newMinDelta;
            swingPS = newPS;
            swingMaxPS = newMaxPS;
            swingMinPS = newMinPS;

            // reset "new" variables to look for next high
            newHi = updateTradedPrice;
            newLo = 0;
            newVol = 0;
            newDelta = 0;
            newMaxDelta = 0;
            newMinDelta = 0;
            newPS = 0;
            newMaxPS = 0;
            newMinPS = 0;

            // set name for swing data subgraph's label: it will be making new
            // possible highs
            swingDataGraph.ShortName = "t.hi";

            // clear current trades
            if (clear.GetYesNo())
              sc.ClearCurrentTradedBidAskVolume();
          } else
            // now searching for next high || low and resetting new volume,
            // delta and PS values each time a potential new high || low is not
            // confirmed
            if (newLo != 0 && updateTradedPrice < newLo) {
              newLo = updateTradedPrice;
              newVol = 0;
              newDelta = 0;
              newMaxDelta = 0;
              newMinDelta = 0;
              newPS = 0;
              newMaxPS = 0;
              newMinPS = 0;
            } else if (newHi != 0 && updateTradedPrice > newHi) {
              newHi = updateTradedPrice;
              newVol = 0;
              newDelta = 0;
              newMaxDelta = 0;
              newMinDelta = 0;
              newPS = 0;
              newMaxPS = 0;
              newMinPS = 0;
            }
      }
    }
    //---END OF TIME & SALES LOOP

    int nRows = 0; // count of number of rows printed on the label column
    float thisLabelPrice = sc.Bid;
    // after looping over Time&Sales, if there have been any trades,
    // updateTradedPrice will be != 0, then get the trades sizes and display
    // them
    if (updateTradedPrice != 0) {
      // SHOW LAST SIZES AS CUMULATIVE SIZE AT PRICE AND/OR DELTA PER PRICE
      if (showSizes.GetIndex() >= 1 && showSizes.GetIndex() <= 3) {
        if (updateTradedPrice != lastTradedPrice) {
          cmB = 0;
          cmA = 0;
        }

        cmB += bidV;
        cmA += askV;

        if (showSizes.GetIndex() < 3) // showing cum. size at price
        {
          cumSizeGraph.Data[i] = cmB + cmA;
          cumSizeGraph.Arrays[0][i] = updateTradedPrice;
          thisLabelPrice = updateTradedPrice;
          nRows++;

          // coloring mode for last cumulative size
          if (cumSizeColoring.GetIndex() == 1) {
            // cumSizeGraph.PrimaryColor = updateTradedPrice == lastTradedPrice
            // ? origColorCumSize : updateTradedPrice > lastTradedPrice ?
            // askSizeGraph.PrimaryColor : bidSizeGraph.PrimaryColor;
            // updateTradedBid = sc.Bid;
            // updateTradedAsk = sc.Ask;

            // cumSizeGraph.PrimaryColor = (updateTradedBid > lastTradedBid &&
            // updateTradedAsk > lastTradedAsk) ? askSizeGraph.PrimaryColor :
            // (updateTradedBid < lastTradedBid && updateTradedAsk <
            // lastTradedAsk) ? bidSizeGraph.PrimaryColor : origColorCumSize;
            cumSizeGraph.PrimaryColor =
                (sc.Bid > lastBid && sc.Ask > lastAsk)   ? askSizeGraph.PrimaryColor
                : (sc.Bid < lastBid && sc.Ask < lastAsk) ? bidSizeGraph.PrimaryColor
                                                         : origColorCumSize;
            // lastTradedBid = updateTradedBid;
            // lastTradedAsk = updateTradedAsk;
          } else if (cumSizeColoring.GetIndex() == 2)
            cumSizeGraph.PrimaryColor =
                cmA - cmB > 0 ? cumDeltaGraph.PrimaryColor : cumDeltaGraph.SecondaryColor;
        }

        // if showing delta per price
        if (showSizes.GetIndex() >= 2) {
          cumDeltaGraph.Data[i] = cmA - cmB;
          /* 					//if expressing delta per price
             as percentage if(showSizes.GetIndex() == 3)
                                                  {
                                                          cumDeltaGraph.Data[i]
             /= (cmA + cmB); cumDeltaGraph.Data[i] *= 100;
                                                  } */
          thisLabelPrice = updateTradedPrice - lineSpacing;
          cumDeltaGraph.Arrays[0][i] = thisLabelPrice; // updateTradedPrice - nRows*lineSpacing;
          nRows++;
        }
      }

      // SHOW LAST SIZES AS BID & ASK SIZES
      else if (showSizes.GetIndex() == 4) {
        /* if(bidV != 0)
        {
                //if(updateTradedBid != lastTradedBid) bidSizeGraph.Data[i] = 0;
                if(sc.Bid != lastTradedBid) bidSizeGraph.Data[i] = 0;
                bidSizeGraph.Data[i] += bidV;
                bidSizeGraph.Arrays[0][i] = sc.Bid;
                //lastTradedBid = updateTradedBid;
                lastTradedBid = sc.Bid;
                nRows++;
        }
        else if(sc.Bid != lastBid) bidSizeGraph.Data[i] = 0;

        if(askV != 0)
        {
                //if(updateTradedAsk != lastTradedAsk) askSizeGraph.Data[i] = 0;
                if(sc.Ask != lastTradedAsk) askSizeGraph.Data[i] = 0;
                askSizeGraph.Data[i] += askV;
                askSizeGraph.Arrays[0][i] = spacing.GetInt() > 1 ? sc.Bid +
        lineSpacing : sc.Ask;
                //lastTradedAsk = updateTradedAsk;
                lastTradedAsk = sc.Ask;
        }
        else if(sc.Ask != lastAsk) askSizeGraph.Data[i] = 0; */

        if (sc.Bid != lastBid)
          bidSizeGraph.Data[i] = 0;
        bidSizeGraph.Data[i] += bidV;
        bidSizeGraph.Arrays[0][i] = thisLabelPrice;
        // lastTradedBid = updateTradedBid;
        // lastTradedBid = sc.Bid;
        nRows++;

        if (sc.Ask != lastAsk)
          askSizeGraph.Data[i] = 0;
        askSizeGraph.Data[i] += askV;
        askSizeGraph.Arrays[0][i] = spacing.GetInt() > 1 ? thisLabelPrice + lineSpacing : sc.Ask;
      }

      /* //SHOW ALSO PULLING & STACKING PER PRICE ON LABEL COLUMN
      if(showSizes.GetIndex() >= 1 && showPSinLabel.GetYesNo() == 1)
      {
              //if price has changed in this update reset the cumulative
      pulling&stacking if(updateTradedPrice != lastTradedPrice)
      cumPSgraph.Data[i] = 0;

              //value of the P&S graph will be set outside this if-block, just
      below, so that it always update

              //leave a blank row below the last print before the P&S
              cumPSgraph.Arrays[0][i] = showSizes.GetIndex() < 4 ?
      updateTradedPrice : sc.Bid; cumPSgraph.Arrays[0][i] -= (nRows + 1) *
      lineSpacing;
      } */

      // save for next update
      lastTradedPrice = updateTradedPrice;
    }

    // SHOW ALSO PULLING & STACKING PER PRICE ON LABEL COLUMN
    if (showSizes.GetIndex() >= 1 && showPSinLabel.GetYesNo() == 1) {
      // if price has changed in this update reset the cumulative
      // pulling&stacking if(updateTradedPrice != lastTradedPrice)
      // cumPSgraph.Data[i] = 0;
      if (sc.Bid != lastBid && sc.Ask != lastAsk)
        cumPSgraph.Data[i] = 0;

      cumPSgraph.Data[i] += updatePS;

      // leave a blank row below the last print before the P&S
      // cumPSgraph.Arrays[0][i] = showSizes.GetIndex() < 4 ? updateTradedPrice
      // : sc.Bid;
      cumPSgraph.Arrays[0][i] = thisLabelPrice - 2 * lineSpacing;
      // if(showSizes.GetInt() == 2) cumPSgraph.Arrays[0][i] -= lineSpacing;
    }
    // set now the P&S PER PRICE label value if shown
    // if(showSizes.GetIndex() >= 1 && showPSinLabel.GetYesNo() == 1)
    // cumPSgraph.Data[i] += updatePS;

    // WRITE CURRENT SWING VALUES
    SCString swingData = "";
    if (whereDisplayData.GetIndex() > 0) {
      swingData = newLo != 0 ? "temp LO" : "temp HI";
      swingData.AppendFormat("\nV: %i", swingVol);
      if (dataToDisplay.GetIndex() > 0) {
        swingData.Append("\n------------");
        if (deltaType.GetIndex() == 0) {
          swingData.AppendFormat("\nDELTA: %i", swingDelta);
          if (showMaxMin.GetYesNo())
            swingData.AppendFormat("\ndmax: %i\ndmin: %i", swingMaxDelta, swingMinDelta);
        } else {
          swingData.AppendFormat("\nDELTA: %i%%", (int)(100 * swingDelta / ((float)swingVol)));
          if (showMaxMin.GetYesNo())
            swingData.AppendFormat("\ndmax: %i%%\ndmin: %i%%",
                                   (int)(100 * swingMaxDelta / ((float)swingVol)),
                                   (int)(100 * swingMinDelta / ((float)swingVol)));
        }
        if (dataToDisplay.GetIndex() == 2) {
          swingData.Append("\n------------");
          swingData.AppendFormat("\nPL&ST: %i", swingPS);
          if (showMaxMin.GetYesNo())
            swingData.AppendFormat("\npsmax: %i\npsmin: %i", swingMaxPS, swingMinPS);
        }
      }
    }
    swingDataGraph.TextDrawStyleText = swingData;
    swingDataGraph[i] = newLo == 0 && newHi == 0 ? sc.Ask : newLo != 0 ? newLo : newHi;
    if (colorSwingData.GetYesNo())
      swingDataGraph.PrimaryColor =
          swingDelta > 0 ? askSizeGraph.PrimaryColor : bidSizeGraph.PrimaryColor;

    // SAVE VALUES FOR NEXT UPDATE
    LastProcessedSequence = TimeSales[TimeSales.Size() - 1].Sequence;
    lastBid = sc.Bid;
    lastAsk = sc.Ask;
    // save candle index
    lastIndex = i;
  }
}

SCSFExport scsf_ColorDeltaBars(SCStudyInterfaceRef sc) {
  SCSubgraphRef ColorDeltaBar = sc.Subgraph[0];
  SCSubgraphRef AskPOC = sc.Subgraph[1];
  SCSubgraphRef BidPOC = sc.Subgraph[2];

  SCInputRef level_3 = sc.Input[0];
  SCInputRef colorAsk_3 = sc.Input[1];
  SCInputRef colorBid_3 = sc.Input[2];
  SCInputRef level_2 = sc.Input[3];
  SCInputRef colorAsk_2 = sc.Input[4];
  SCInputRef colorBid_2 = sc.Input[5];
  SCInputRef level_1 = sc.Input[6];
  SCInputRef colorAsk_1 = sc.Input[7];
  SCInputRef colorBid_1 = sc.Input[8];
  SCInputRef barColorMode = sc.Input[9];
  SCInputRef useBaseColor = sc.Input[10];
  SCInputRef baseColor = sc.Input[11];
  SCInputRef displayPOCs = sc.Input[12];
  SCInputRef extendPOCs = sc.Input[13];

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "Color Bars per Delta %";
    sc.StudyDescription = "";
    sc.AutoLoop = 1; // true
    sc.GraphRegion = 0;

    sc.MaintainVolumeAtPriceData = 1;

    ColorDeltaBar.Name = "Color";
    ColorDeltaBar.DrawStyle = DRAWSTYLE_COLOR_BAR;
    ColorDeltaBar.DrawZeros = false;

    AskPOC.Name = "Ask Side POCs";
    AskPOC.DrawStyle = DRAWSTYLE_DASH;
    AskPOC.DrawZeros = false;
    AskPOC.PrimaryColor = RGB(255, 255, 0);
    AskPOC.LineWidth = 3;

    BidPOC.Name = "Bid Side POCs";
    BidPOC.DrawStyle = DRAWSTYLE_DASH;
    BidPOC.DrawZeros = false;
    BidPOC.PrimaryColor = RGB(255, 0, 255);
    BidPOC.LineWidth = 3;

    level_3.Name = " % Threshold 3";
    level_3.SetInt(24);
    colorAsk_3.Name = "Ask Delta 3";
    colorAsk_3.SetColor(140, 255, 140);
    colorBid_3.Name = "Bid Delta 3";
    colorBid_3.SetColor(255, 140, 140);

    level_2.Name = " % Threshold 2";
    level_2.SetInt(16);
    colorAsk_2.Name = "Ask Delta 2";
    colorAsk_2.SetColor(0, 255, 0);
    colorBid_2.Name = "Bid Delta 2";
    colorBid_2.SetColor(255, 0, 0);

    level_1.Name = " % Threshold 1";
    level_1.SetInt(8);
    colorAsk_1.Name = "Ask Delta 1";
    colorAsk_1.SetColor(0, 170, 0);
    colorBid_1.Name = "Bid Delta 2";
    colorBid_1.SetColor(170, 0, 0);

    barColorMode.Name = "Bar Color Mode";
    barColorMode.SetCustomInputStrings("Use thresholds; Use gradient: Base-Threshold 3");
    barColorMode.SetCustomInputIndex(1);

    useBaseColor.Name = "Color candles below threshold 1";
    useBaseColor.SetYesNo(0);

    baseColor.Name = "Base Color (for gradients || candles below thres. 1)";
    baseColor.SetColor(170, 170, 170);

    displayPOCs.Name = "Display Thresholds 2-3 bar-POCs";
    displayPOCs.SetYesNo(1);

    extendPOCs.Name = "Extend bar POCs";
    extendPOCs.SetYesNo(1);

    return;
  }

  ColorDeltaBar.Name = "Color";
  ColorDeltaBar.DrawStyle = DRAWSTYLE_COLOR_BAR;

  float deltaPerc = 100 * ((float)sc.AskVolume[sc.Index] - (float)sc.BidVolume[sc.Index]) /
                    (float)sc.Volume[sc.Index];

  // if using threshold colors
  if (barColorMode.GetIndex() == 0) {
    // at the beginning color bar with base color || don't color
    ColorDeltaBar.DataColor[sc.Index] = useBaseColor.GetYesNo() ? baseColor.GetColor() : 0;

    // sepecify color depending on delta threshold
    if (deltaPerc > level_3.GetInt())
      ColorDeltaBar.DataColor[sc.Index] = colorAsk_3.GetColor();
    else if (deltaPerc > level_2.GetInt())
      ColorDeltaBar.DataColor[sc.Index] = colorAsk_2.GetColor();
    else if (deltaPerc > level_1.GetInt())
      ColorDeltaBar.DataColor[sc.Index] = colorAsk_1.GetColor();
    else if (deltaPerc < -level_3.GetInt())
      ColorDeltaBar.DataColor[sc.Index] = colorBid_3.GetColor();
    else if (deltaPerc < -level_2.GetInt())
      ColorDeltaBar.DataColor[sc.Index] = colorBid_2.GetColor();
    else if (deltaPerc < -level_1.GetInt())
      ColorDeltaBar.DataColor[sc.Index] = colorBid_1.GetColor();

    // activate bar coloring if .DataColor contains any color
    ColorDeltaBar[sc.Index] = ColorDeltaBar.DataColor[sc.Index] != 0 ? 1 : 0;
  } else // using gradient
  {
    ColorDeltaBar[sc.Index] = 1;                     // always color candles
    float interpPerc = deltaPerc / level_3.GetInt(); // deltaPercent threshold 3 ratio
    if (deltaPerc > 0)                               // set the candle color
      ColorDeltaBar.DataColor[sc.Index] =
          sc.RGBInterpolate(baseColor.GetColor(), colorAsk_3.GetColor(), min(interpPerc, 1));
    else
      ColorDeltaBar.DataColor[sc.Index] =
          sc.RGBInterpolate(baseColor.GetColor(), colorBid_3.GetColor(), -max(interpPerc, -1));
  }

  // si la barra ha cerrado, calulo POC de cada lado
  if (sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED) {
    if (deltaPerc > level_2.GetInt()) {
      const s_VolumeAtPriceV2 *p_VolumeAtPrice = NULL;
      int VAPSizeAtBarIndex = sc.VolumeAtPriceForBars->GetSizeAtBarIndex(sc.Index);
      unsigned int maxAskVol = 0;
      float maxAskPrice = 0;
      for (int VAPIndex = 0; VAPIndex < VAPSizeAtBarIndex; VAPIndex++) {
        if (!sc.VolumeAtPriceForBars->GetVAPElementAtIndex(sc.Index, VAPIndex, &p_VolumeAtPrice))
          break;

        if (p_VolumeAtPrice->AskVolume > maxAskVol) {
          maxAskVol = p_VolumeAtPrice->AskVolume;
          maxAskPrice = p_VolumeAtPrice->PriceInTicks * sc.TickSize;
        }
      }

      if (displayPOCs.GetYesNo())
        AskPOC[sc.Index] = maxAskPrice;
      if (extendPOCs.GetYesNo())
        sc.AddLineUntilFutureIntersection(sc.Index, 0, maxAskPrice, AskPOC.PrimaryColor, 1,
                                          LINESTYLE_DOT, 0, 0, "");
    }

    if (deltaPerc < -level_2.GetInt()) {
      const s_VolumeAtPriceV2 *p_VolumeAtPrice = NULL;
      int VAPSizeAtBarIndex = sc.VolumeAtPriceForBars->GetSizeAtBarIndex(sc.Index);
      unsigned int maxBidVol = 0;
      float maxBidPrice = 0;
      for (int VAPIndex = 0; VAPIndex < VAPSizeAtBarIndex; VAPIndex++) {
        if (!sc.VolumeAtPriceForBars->GetVAPElementAtIndex(sc.Index, VAPIndex, &p_VolumeAtPrice))
          break;

        if (p_VolumeAtPrice->BidVolume > maxBidVol) {
          maxBidVol = p_VolumeAtPrice->BidVolume;
          maxBidPrice = p_VolumeAtPrice->PriceInTicks * sc.TickSize;
        }
      }

      if (displayPOCs.GetYesNo())
        BidPOC[sc.Index] = maxBidPrice;
      if (extendPOCs.GetYesNo())
        sc.AddLineUntilFutureIntersection(sc.Index, 0, maxBidPrice, BidPOC.PrimaryColor, 1,
                                          LINESTYLE_DOT, 0, 0, "");
    }
  }
}

SCSFExport scsf_FairValueGaps(SCStudyInterfaceRef sc) {
  SCInputRef upFVGfills = sc.Input[0];
  SCInputRef downFVGfills = sc.Input[1];
  SCInputRef fillsTransparency = sc.Input[2];
  SCInputRef outlineWidth = sc.Input[3];
  SCInputRef downFVGlines = sc.Input[4];
  SCInputRef upFVGlines = sc.Input[5];
  SCInputRef showMidline = sc.Input[6];
  SCInputRef showNunfilled = sc.Input[7];
  SCInputRef showNfilled = sc.Input[8];

  // last calculated bar
  int &lastBar = sc.GetPersistentInt(-1);

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "Fair Value Gaps (FVGs)";
    sc.AutoLoop = 1;
    sc.GraphRegion = 0;

    upFVGfills.Name = "Up FVG FillColor";
    upFVGfills.SetColor(144, 191, 249);

    downFVGfills.Name = "Down FVG FillColor";
    downFVGfills.SetColor(244, 143, 177);

    fillsTransparency.Name = "Transparency for Fills";
    fillsTransparency.SetInt(90);

    outlineWidth.Name = "FVGs Outline Width (0=hidden)";
    outlineWidth.SetInt(1);

    upFVGlines.Name = "Up FVG LineColor";
    upFVGlines.SetColor(187, 216, 251);

    downFVGlines.Name = "Down FVG LineColor";
    downFVGlines.SetColor(250, 201, 217);

    showMidline.Name = "Show Midline";
    showMidline.SetYesNo(0);

    showNunfilled.Name = "How many Unfilled FVGs to show";
    showNunfilled.SetInt(10);
    showNunfilled.SetIntLimits(1, 1000);

    showNfilled.Name = "How many Filled FVGs to show";
    showNfilled.SetInt(50);
    showNfilled.SetIntLimits(0, 1000);

    // reset lastBar
    lastBar = -1;

    return;
  }

  // persistent variables
  float &nearestUntestedLevelBelow = sc.GetPersistentFloat(0);
  float &nearestUntestedLevelAbove = sc.GetPersistentFloat(1);

  // where do GetPersistentInt() pointers to store IDs of filled and unfilled
  // FVGs start
  const int firstTested = 5;
  int firstUntested = firstTested + showNfilled.GetInt() + 10;
  // GetPersistentInt() pointer for the ID of the next/coming UP || DOWN
  // unfilled FVG
  int &upFVGsNextID = sc.GetPersistentInt(4);
  int &downFVGsNextID = sc.GetPersistentInt(-4);

  // full recalculation
  if ((sc.IsFullRecalculation && sc.Index == 0) ||
      lastBar == -1) // This indicates a study is being recalculated.
  {
    // When there is a full recalculation of the study,
    // reset the persistent variables we are using
    nearestUntestedLevelAbove = 0;
    nearestUntestedLevelBelow = 0;
    upFVGsNextID = firstUntested;
    downFVGsNextID = -firstUntested;
  }

  //**** BEGIN
  if (!sc.HideStudy && sc.Index != lastBar && sc.Index >= 3) {
    // IF TESTING NEAREST UNFILLED LEVELS ABOVE AND BELOW
    // if testing the nearest level below
    if (sc.High[sc.Index - 1] >= nearestUntestedLevelAbove &&
        sc.High[sc.Index - 2] < nearestUntestedLevelAbove && nearestUntestedLevelAbove != 0) {
      // loop through the unfilled DOWN-FVGs array
      for (int x = downFVGsNextID + 1; x <= -firstUntested; x++) {
        s_UseTool Rect;
        Rect.Clear();
        bool getLine = sc.GetACSDrawingByLineNumber(0, sc.GetPersistentInt(x), Rect);
        if (getLine) {
          double begin = Rect.BeginValue;
          // if this FVG has just been filled
          if (sc.High[sc.Index - 1] >= begin && sc.High[sc.Index - 2] < begin) {
            // modify rectangle to make it finish in this bar
            Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
            Rect.EndIndex = sc.Index - 1;
            Rect.AddMethod = UTAM_ADD_OR_ADJUST;
            sc.UseTool(Rect);

            // delete its ID from the untested array and decrement the unfilled
            // pointer
            sc.GetPersistentInt(x) = 0;
            nearestUntestedLevelAbove = 0;
            downFVGsNextID++;

            // if showing filled FVGs, delete filled FVGs older than showNfilled
            if (showNfilled.GetInt() > 0) {
              for (int i = -firstTested; i > -firstTested - showNfilled.GetInt(); i--) {
                if (i == -firstTested)
                  sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, sc.GetPersistentInt(i));
                if (i > -firstTested - showNfilled.GetInt() + 1)
                  sc.GetPersistentInt(i) = sc.GetPersistentInt(i - 1);
              }

              // save the newly filled FVG ID to be able to delete it later
              sc.GetPersistentInt(-firstTested - showNfilled.GetInt() + 1) = Rect.LineNumber;
            }
            // if not, delete the newly filled gap
            else
              sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, Rect.LineNumber);

          }
          // show next untested showNunfilled FVGs
          else {
            if (x < downFVGsNextID + 1 + showNunfilled.GetInt()) {
              // show drawing
              Rect.HideDrawing = 0;
              Rect.AddMethod = UTAM_ADD_OR_ADJUST;
              sc.UseTool(Rect);

              // search for next untested level
              if (Rect.BeginValue < nearestUntestedLevelAbove || nearestUntestedLevelAbove == 0)
                nearestUntestedLevelAbove = Rect.BeginValue;
            }
          }
        }
      }
    }

    // if testing the nearest level below
    if (sc.Low[sc.Index - 1] <= nearestUntestedLevelBelow &&
        sc.Low[sc.Index - 2] > nearestUntestedLevelBelow && nearestUntestedLevelBelow != 0) {
      // loop through the unfilled UP-FVGs array
      for (int x = upFVGsNextID - 1; x >= firstUntested; x--) {
        s_UseTool Rect;
        Rect.Clear();
        bool getLine = sc.GetACSDrawingByLineNumber(0, sc.GetPersistentInt(x), Rect);
        if (getLine) {
          double begin = Rect.BeginValue;
          // if this FVG has just been filled
          if (sc.Low[sc.Index - 1] <= begin && sc.Low[sc.Index - 2] > begin) {
            // modify rectangle to make it finish in this bar
            Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
            Rect.EndIndex = sc.Index - 1;
            Rect.AddMethod = UTAM_ADD_OR_ADJUST;
            sc.UseTool(Rect);

            // delete its ID from the untested array and decrement the unfilled
            // pointer
            sc.GetPersistentInt(x) = 0;
            nearestUntestedLevelBelow = 0;
            upFVGsNextID--;

            // if showing filled FVGs, delete filled FVGs older than showNfilled
            if (showNfilled.GetInt() > 0) {
              for (int i = firstTested; i < firstTested + showNfilled.GetInt(); i++) {
                if (i == firstTested)
                  sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, sc.GetPersistentInt(i));
                if (i < firstTested + showNfilled.GetInt() - 1)
                  sc.GetPersistentInt(i) = sc.GetPersistentInt(i + 1);
              }

              // save the newly filled FVG ID to be able to delete it later
              sc.GetPersistentInt(firstTested + showNfilled.GetInt() - 1) = Rect.LineNumber;
            }
            // if not, delete the newly filled gap
            else
              sc.DeleteACSChartDrawing(0, TOOL_DELETE_CHARTDRAWING, Rect.LineNumber);
          }
          // show next untested showNunfilled FVGs
          else {
            if (x > upFVGsNextID - 1 - showNunfilled.GetInt()) {
              // show drawing
              Rect.HideDrawing = 0;
              Rect.AddMethod = UTAM_ADD_OR_ADJUST;
              sc.UseTool(Rect);

              // search for next untested level
              if (Rect.BeginValue > nearestUntestedLevelBelow || nearestUntestedLevelBelow == 0)
                nearestUntestedLevelBelow = Rect.BeginValue;
            }
          }
        }
      }
    }

    //**** LOOK FOR FAIR VALUE GAPS
    // GAPUP
    if (sc.Low[sc.Index - 1] > sc.High[sc.Index - 3] &&
        sc.Low[sc.Index - 2] < sc.Low[sc.Index - 1] &&
        sc.High[sc.Index - 2] >= sc.Low[sc.Index - 1]) {
      // define a s_UseTool variable
      s_UseTool Rect;
      Rect.Clear();

      // create level
      Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
      Rect.BeginIndex = sc.Index - 3;
      Rect.EndIndex = sc.Index - 1;
      // set the rectangle levels
      Rect.BeginValue = sc.High[sc.Index - 3];
      Rect.EndValue = sc.Low[sc.Index - 1];
      Rect.Color = upFVGlines.GetColor();
      Rect.SecondaryColor = upFVGfills.GetColor();
      Rect.TransparencyLevel = fillsTransparency.GetInt();
      Rect.NoVerticalOutline = 1;
      Rect.LineWidth = outlineWidth.GetInt();
      Rect.DrawMidline = showMidline.GetYesNo();

      sc.UseTool(Rect);

      // save FVG ID in untested and its level in nearestGap
      sc.GetPersistentInt(upFVGsNextID) = Rect.LineNumber;
      nearestUntestedLevelBelow = sc.High[sc.Index - 3];

      // hide previous untested FVGs
      if (upFVGsNextID >= firstUntested + showNunfilled.GetInt()) {
        Rect.Clear();
        bool getLine = sc.GetACSDrawingByLineNumber(
            0, sc.GetPersistentInt(upFVGsNextID - showNunfilled.GetInt()), Rect);
        Rect.HideDrawing = 1;
        Rect.AddMethod = UTAM_ADD_OR_ADJUST;
        sc.UseTool(Rect);
      }

      // increment untested IDs counter
      upFVGsNextID++;
    }

    // GAPDOWN
    if (sc.High[sc.Index - 1] < sc.Low[sc.Index - 3] &&
        sc.High[sc.Index - 2] > sc.High[sc.Index - 1] &&
        sc.Low[sc.Index - 2] <= sc.High[sc.Index - 1]) {
      // define a s_UseTool variable
      s_UseTool Rect;
      Rect.Clear();

      // create level
      Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
      Rect.BeginIndex = sc.Index - 3;
      Rect.EndIndex = sc.Index - 1;
      // set the rectangle levels
      Rect.BeginValue = sc.Low[sc.Index - 3];
      Rect.EndValue = sc.High[sc.Index - 1];
      Rect.Color = downFVGlines.GetColor();
      Rect.SecondaryColor = downFVGfills.GetColor();
      Rect.TransparencyLevel = fillsTransparency.GetInt();
      Rect.NoVerticalOutline = 1;
      Rect.LineWidth = outlineWidth.GetInt();
      Rect.DrawMidline = showMidline.GetYesNo();

      sc.UseTool(Rect);

      // save FVG ID in untested and its level in nearestGap
      sc.GetPersistentInt(downFVGsNextID) = Rect.LineNumber;
      nearestUntestedLevelAbove = sc.Low[sc.Index - 3];

      // hide previous untested FVGs
      if (downFVGsNextID <= -firstUntested - showNunfilled.GetInt()) {
        Rect.Clear();
        bool getLine = sc.GetACSDrawingByLineNumber(
            0, sc.GetPersistentInt(downFVGsNextID + showNunfilled.GetInt()), Rect);
        Rect.HideDrawing = 1;
        Rect.AddMethod = UTAM_ADD_OR_ADJUST;
        sc.UseTool(Rect);
      }

      // increment untested IDs counter
      downFVGsNextID--;
    }
  }

  // save Index
  lastBar = sc.Index;
}

SCSFExport scsf_GapBars(SCStudyInterfaceRef sc) {
  SCSubgraphRef GapUpBar = sc.Subgraph[0];
  SCSubgraphRef GapDownBar = sc.Subgraph[1];

  if (sc.SetDefaults) {
    // Set the configuration and defaults
    sc.GraphName = "Gap Bars";
    sc.StudyDescription = "Gap bars as per Al Brooks";
    sc.AutoLoop = 1; // true
    sc.GraphRegion = 0;

    GapUpBar.Name = "GapUp Bar Color";
    GapUpBar.DrawStyle = DRAWSTYLE_COLOR_BAR;
    GapUpBar.PrimaryColor = RGB(57, 140, 242);
    GapUpBar.DrawZeros = false;

    GapDownBar.Name = "GapDown Bar Color";
    GapDownBar.DrawStyle = DRAWSTYLE_COLOR_BAR;
    GapDownBar.PrimaryColor = RGB(251, 32, 125);
    GapDownBar.DrawZeros = false;

    return;
  }

  //*** SI LA BARRA HA CERRADO, BUSCO SI HA HABIDO GAPS
  if (!sc.HideStudy && sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED) {
    // VEO SI HAY GAPUP
    if (sc.Low[sc.Index] > sc.High[sc.Index - 2]) {
      // coloreo la barra/candle
      GapUpBar[sc.Index - 1] = 1;
    }

    // VEO SI HAY GAPDOWN
    if (sc.High[sc.Index] < sc.Low[sc.Index - 2]) {
      // coloreo la barra/candle
      GapDownBar[sc.Index - 1] = 1;
    }
  }
}

SCSFExport scsf_InitialBalanceSession(SCStudyInterfaceRef sc) {
  SCSubgraphRef Subgraph_IBHExt6 = sc.Subgraph[0];
  SCSubgraphRef Subgraph_IBHExt5 = sc.Subgraph[1];
  SCSubgraphRef Subgraph_IBHExt4 = sc.Subgraph[2];
  SCSubgraphRef Subgraph_IBHExt3 = sc.Subgraph[3];
  SCSubgraphRef Subgraph_IBHExt2 = sc.Subgraph[4];
  SCSubgraphRef Subgraph_IBHExt1 = sc.Subgraph[5];
  SCSubgraphRef Subgraph_IBHigh = sc.Subgraph[6];
  SCSubgraphRef Subgraph_IBMid = sc.Subgraph[7];
  SCSubgraphRef Subgraph_IBLow = sc.Subgraph[8];
  SCSubgraphRef Subgraph_IBLExt1 = sc.Subgraph[9];
  SCSubgraphRef Subgraph_IBLExt2 = sc.Subgraph[10];
  SCSubgraphRef Subgraph_IBLExt3 = sc.Subgraph[11];
  SCSubgraphRef Subgraph_IBLExt4 = sc.Subgraph[12];
  SCSubgraphRef Subgraph_IBLExt5 = sc.Subgraph[13];
  SCSubgraphRef Subgraph_IBLExt6 = sc.Subgraph[14];

  SCInputRef Input_IBType = sc.Input[0];
  SCInputRef Input_PeriodEndAsMinutesFromSessionStart = sc.Input[1];
  SCInputRef Input_PeriodEndAsSecondsFromSessionStart = sc.Input[2];
  SCInputRef Input_RoundExt = sc.Input[3];
  SCInputRef Input_NumberDaysToCalculate = sc.Input[4];
  SCInputRef custom_hour = sc.Input[5];
  SCInputRef custom_min = sc.Input[6];

  SCInputRef Input_Multiplier1 = sc.Input[7];
  SCInputRef Input_Multiplier2 = sc.Input[8];
  SCInputRef Input_Multiplier3 = sc.Input[9];
  SCInputRef Input_Multiplier4 = sc.Input[10];
  SCInputRef Input_Multiplier5 = sc.Input[11];
  SCInputRef Input_Multiplier6 = sc.Input[12];

  if (sc.SetDefaults) {
    sc.GraphName = "Session's Initial Balance";
    sc.DrawZeros = 0;
    sc.GraphRegion = 0;
    sc.AutoLoop = 1;

    sc.ScaleRangeType = SCALE_SAMEASREGION;

    Subgraph_IBHExt6.Name = "IB High Ext 6";
    Subgraph_IBHExt6.PrimaryColor = RGB(0, 255, 0);
    Subgraph_IBHExt6.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBHExt6.DrawZeros = false;

    Subgraph_IBHExt5.Name = "IB High Ext 5";
    Subgraph_IBHExt5.PrimaryColor = RGB(0, 255, 0);
    Subgraph_IBHExt5.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBHExt5.DrawZeros = false;

    Subgraph_IBHExt4.Name = "IB High Ext 4";
    Subgraph_IBHExt4.PrimaryColor = RGB(0, 255, 0);
    Subgraph_IBHExt4.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBHExt4.DrawZeros = false;

    Subgraph_IBHExt3.Name = "IB High Ext 3";
    Subgraph_IBHExt3.PrimaryColor = RGB(0, 255, 0);
    Subgraph_IBHExt3.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBHExt3.DrawZeros = false;

    Subgraph_IBHExt2.Name = "IB High Ext 2";
    Subgraph_IBHExt2.PrimaryColor = RGB(0, 255, 0);
    Subgraph_IBHExt2.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    Subgraph_IBHExt2.DrawZeros = false;

    Subgraph_IBHExt1.Name = "IB High Ext 1";
    Subgraph_IBHExt1.PrimaryColor = RGB(0, 255, 0);
    Subgraph_IBHExt1.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    Subgraph_IBHExt1.DrawZeros = false;

    Subgraph_IBHigh.Name = "IB High";
    Subgraph_IBHigh.PrimaryColor = RGB(128, 255, 128);
    Subgraph_IBHigh.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    Subgraph_IBHigh.DrawZeros = false;

    Subgraph_IBMid.Name = "IB Mid";
    Subgraph_IBMid.PrimaryColor = RGB(255, 255, 255);
    Subgraph_IBMid.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    Subgraph_IBMid.DrawZeros = false;

    Subgraph_IBLow.Name = "IB Low";
    Subgraph_IBLow.PrimaryColor = RGB(255, 128, 128);
    Subgraph_IBLow.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    Subgraph_IBLow.DrawZeros = false;

    Subgraph_IBLExt1.Name = "IB Low Ext 1";
    Subgraph_IBLExt1.PrimaryColor = RGB(255, 0, 0);
    Subgraph_IBLExt1.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    Subgraph_IBLExt1.DrawZeros = false;

    Subgraph_IBLExt2.Name = "IB Low Ext 2";
    Subgraph_IBLExt2.PrimaryColor = RGB(255, 0, 0);
    Subgraph_IBLExt2.DrawStyle = DRAWSTYLE_LINE_SKIP_ZEROS;
    Subgraph_IBLExt2.DrawZeros = false;

    Subgraph_IBLExt3.Name = "IB Low Ext 3";
    Subgraph_IBLExt3.PrimaryColor = RGB(255, 0, 0);
    Subgraph_IBLExt3.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBLExt3.DrawZeros = false;

    Subgraph_IBLExt4.Name = "IB Low Ext 4";
    Subgraph_IBLExt4.PrimaryColor = RGB(255, 0, 0);
    Subgraph_IBLExt4.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBLExt4.DrawZeros = false;

    Subgraph_IBLExt5.Name = "IB Low Ext 5";
    Subgraph_IBLExt5.PrimaryColor = RGB(255, 0, 0);
    Subgraph_IBLExt5.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBLExt5.DrawZeros = false;

    Subgraph_IBLExt6.Name = "IB Low Ext 6";
    Subgraph_IBLExt6.PrimaryColor = RGB(255, 0, 0);
    Subgraph_IBLExt6.DrawStyle = DRAWSTYLE_IGNORE;
    Subgraph_IBLExt6.DrawZeros = false;

    // Inputs
    Input_IBType.Name = "Initial Balance For Session";
    Input_IBType.SetCustomInputStrings("Day Session (RTH);Evening Session (ETH);Custom Session");
    Input_IBType.SetCustomInputIndex(0);

    Input_PeriodEndAsMinutesFromSessionStart.Name = "Period End As Minutes from Session Start";
    Input_PeriodEndAsMinutesFromSessionStart.SetInt(30);

    Input_PeriodEndAsSecondsFromSessionStart.Name = "And/or Seconds (Opening Range)";
    Input_PeriodEndAsSecondsFromSessionStart.SetInt(0);

    Input_RoundExt.Name = "Round Extensions to TickSize";
    Input_RoundExt.SetYesNo(1);

    Input_NumberDaysToCalculate.Name = "Number of Days to Calculate";
    Input_NumberDaysToCalculate.SetInt(100);
    Input_NumberDaysToCalculate.SetIntLimits(1, INT_MAX);

    custom_hour.Name = "Custom Session-Hour";
    custom_hour.SetInt(9);
    custom_min.Name = "Custom Session-Hour";
    custom_min.SetInt(9);

    Input_Multiplier1.Name = "Extension Multiplier 1";
    Input_Multiplier1.SetFloat(.5f);
    Input_Multiplier2.Name = "Extension Multiplier 2";
    Input_Multiplier2.SetFloat(1.0f);
    Input_Multiplier3.Name = "Extension Multiplier 3";
    Input_Multiplier3.SetFloat(1.5f);
    Input_Multiplier4.Name = "Extension Multiplier 4";
    Input_Multiplier4.SetFloat(2.0f);
    Input_Multiplier5.Name = "Extension Multiplier 5";
    Input_Multiplier5.SetFloat(2.5f);
    Input_Multiplier6.Name = "Extension Multiplier 6";
    Input_Multiplier6.SetFloat(3.0f);

    return;
  }

  // Persist vars
  int &PeriodFirstIndex = sc.GetPersistentInt(1);

  SCDateTime &PeriodStartDateTime = sc.GetPersistentSCDateTime(1);
  SCDateTime &PeriodEndDateTime = sc.GetPersistentSCDateTime(2);

  float &PeriodHigh = sc.GetPersistentFloat(1);
  float &PeriodLow = sc.GetPersistentFloat(2);
  float &PeriodMid = sc.GetPersistentFloat(3);
  float &PeriodHighExt1 = sc.GetPersistentFloat(4);
  float &PeriodHighExt2 = sc.GetPersistentFloat(5);
  float &PeriodHighExt3 = sc.GetPersistentFloat(6);
  float &PeriodHighExt4 = sc.GetPersistentFloat(7);
  float &PeriodHighExt5 = sc.GetPersistentFloat(8);
  float &PeriodHighExt6 = sc.GetPersistentFloat(9);
  float &PeriodLowExt1 = sc.GetPersistentFloat(10);
  float &PeriodLowExt2 = sc.GetPersistentFloat(11);
  float &PeriodLowExt3 = sc.GetPersistentFloat(12);
  float &PeriodLowExt4 = sc.GetPersistentFloat(13);
  float &PeriodLowExt5 = sc.GetPersistentFloat(14);
  float &PeriodLowExt6 = sc.GetPersistentFloat(15);

  // Reset persistent variables upon full calculation
  if (sc.Index == 0) {
    PeriodFirstIndex = -1;
    PeriodStartDateTime = 0;
    PeriodEndDateTime = 0;
    PeriodHigh = -FLT_MAX;
    PeriodLow = FLT_MAX;
  }

  SCDateTimeMS LastBarDateTime = sc.BaseDateTimeIn[sc.ArraySize - 1];
  SCDateTimeMS FirstCalculationDate =
      LastBarDateTime.GetDate() - SCDateTime::DAYS(Input_NumberDaysToCalculate.GetInt() - 1);

  SCDateTimeMS CurrentBarDateTime = sc.BaseDateTimeIn[sc.Index];

  SCDateTimeMS PrevBarDateTime;

  if (sc.Index > 0)
    PrevBarDateTime = sc.BaseDateTimeIn[sc.Index - 1];

  if (CurrentBarDateTime.GetDate() < FirstCalculationDate) // Limit calculation to specified number
                                                           // of days back
    return;

  SCDateTimeMS StartDateTime = CurrentBarDateTime;

  // Set StartTime for selected session
  if (Input_IBType.GetIndex() == 0)
    StartDateTime.SetTime(sc.StartTime1);
  else if (Input_IBType.GetIndex() == 1)
    StartDateTime.SetTime(sc.StartTime2);
  else
    StartDateTime.SetTime(custom_hour.GetInt() * 3600 + custom_min.GetInt() * 60);

  if (PrevBarDateTime < StartDateTime && CurrentBarDateTime >= StartDateTime) {
    PeriodFirstIndex = sc.Index;
    PeriodHigh = -FLT_MAX;
    PeriodLow = FLT_MAX;

    PeriodStartDateTime = StartDateTime;

    // Set end of Initial Balance as minutes from session start
    PeriodEndDateTime = PeriodStartDateTime;
    PeriodEndDateTime +=
        SCDateTime::SECONDS(Input_PeriodEndAsMinutesFromSessionStart.GetInt() * SECONDS_PER_MINUTE +
                            Input_PeriodEndAsSecondsFromSessionStart.GetInt() - 1);

    // set prev bar values to zero to avoid weird line jumps
    for (int x = 0; x < 15; x++) {
      sc.Subgraph[x][sc.Index - 1] = 0;
    }
  }

  // Check end of period
  if (PeriodFirstIndex >= 0) {
    if (CurrentBarDateTime > PeriodEndDateTime) {
      PeriodFirstIndex = -1;
    }
  }

  // Collecting data, back propagate if changed
  if (PeriodFirstIndex >= 0) {
    bool Changed = false;

    if (sc.High[sc.Index] > PeriodHigh) {
      PeriodHigh = sc.High[sc.Index];
      Changed = true;
    }

    if (sc.Low[sc.Index] < PeriodLow) {
      PeriodLow = sc.Low[sc.Index];
      Changed = true;
    }

    if (Changed) {
      PeriodMid = (PeriodHigh + PeriodLow) / 2.0f;

      float Range = PeriodHigh - PeriodLow;

      PeriodHighExt1 = PeriodHigh + Input_Multiplier1.GetFloat() * Range;
      PeriodHighExt2 = PeriodHigh + Input_Multiplier2.GetFloat() * Range;
      PeriodHighExt3 = PeriodHigh + Input_Multiplier3.GetFloat() * Range;
      PeriodHighExt4 = PeriodHigh + Input_Multiplier4.GetFloat() * Range;
      PeriodHighExt5 = PeriodHigh + Input_Multiplier5.GetFloat() * Range;
      PeriodHighExt6 = PeriodHigh + Input_Multiplier6.GetFloat() * Range;

      PeriodLowExt1 = PeriodLow - Input_Multiplier1.GetFloat() * Range;
      PeriodLowExt2 = PeriodLow - Input_Multiplier2.GetFloat() * Range;
      PeriodLowExt3 = PeriodLow - Input_Multiplier3.GetFloat() * Range;
      PeriodLowExt4 = PeriodLow - Input_Multiplier4.GetFloat() * Range;
      PeriodLowExt5 = PeriodLow - Input_Multiplier5.GetFloat() * Range;
      PeriodLowExt6 = PeriodLow - Input_Multiplier6.GetFloat() * Range;

      if (Input_RoundExt.GetYesNo()) {
        PeriodHighExt1 = sc.RoundToTickSize(PeriodHighExt1, sc.TickSize);
        PeriodHighExt2 = sc.RoundToTickSize(PeriodHighExt2, sc.TickSize);
        PeriodHighExt3 = sc.RoundToTickSize(PeriodHighExt3, sc.TickSize);
        PeriodHighExt4 = sc.RoundToTickSize(PeriodHighExt4, sc.TickSize);
        PeriodHighExt5 = sc.RoundToTickSize(PeriodHighExt5, sc.TickSize);
        PeriodHighExt6 = sc.RoundToTickSize(PeriodHighExt6, sc.TickSize);

        PeriodLowExt1 = sc.RoundToTickSize(PeriodLowExt1, sc.TickSize);
        PeriodLowExt2 = sc.RoundToTickSize(PeriodLowExt2, sc.TickSize);
        PeriodLowExt3 = sc.RoundToTickSize(PeriodLowExt3, sc.TickSize);
        PeriodLowExt4 = sc.RoundToTickSize(PeriodLowExt4, sc.TickSize);
        PeriodLowExt5 = sc.RoundToTickSize(PeriodLowExt5, sc.TickSize);
        PeriodLowExt6 = sc.RoundToTickSize(PeriodLowExt6, sc.TickSize);
      }

      for (int Index = PeriodFirstIndex; Index < sc.Index; Index++) {
        Subgraph_IBHigh[Index] = PeriodHigh;
        Subgraph_IBLow[Index] = PeriodLow;
        Subgraph_IBMid[Index] = PeriodMid;
        Subgraph_IBHExt1[Index] = PeriodHighExt1;
        Subgraph_IBHExt2[Index] = PeriodHighExt2;
        Subgraph_IBHExt3[Index] = PeriodHighExt3;
        Subgraph_IBHExt4[Index] = PeriodHighExt4;
        Subgraph_IBHExt5[Index] = PeriodHighExt5;
        Subgraph_IBHExt6[Index] = PeriodHighExt6;
        Subgraph_IBLExt1[Index] = PeriodLowExt1;
        Subgraph_IBLExt2[Index] = PeriodLowExt2;
        Subgraph_IBLExt3[Index] = PeriodLowExt3;
        Subgraph_IBLExt4[Index] = PeriodLowExt4;
        Subgraph_IBLExt5[Index] = PeriodLowExt5;
        Subgraph_IBLExt6[Index] = PeriodLowExt6;
      }

      sc.EarliestUpdateSubgraphDataArrayIndex = PeriodFirstIndex;
    }
  }

  // Plot current values
  if (PeriodLow != FLT_MAX) {
    Subgraph_IBHigh[sc.Index] = PeriodHigh;
    Subgraph_IBLow[sc.Index] = PeriodLow;
    Subgraph_IBMid[sc.Index] = PeriodMid;
    Subgraph_IBHExt1[sc.Index] = PeriodHighExt1;
    Subgraph_IBHExt2[sc.Index] = PeriodHighExt2;
    Subgraph_IBHExt3[sc.Index] = PeriodHighExt3;
    Subgraph_IBHExt4[sc.Index] = PeriodHighExt4;
    Subgraph_IBHExt5[sc.Index] = PeriodHighExt5;
    Subgraph_IBHExt6[sc.Index] = PeriodHighExt6;
    Subgraph_IBLExt1[sc.Index] = PeriodLowExt1;
    Subgraph_IBLExt2[sc.Index] = PeriodLowExt2;
    Subgraph_IBLExt3[sc.Index] = PeriodLowExt3;
    Subgraph_IBLExt4[sc.Index] = PeriodLowExt4;
    Subgraph_IBLExt5[sc.Index] = PeriodLowExt5;
    Subgraph_IBLExt6[sc.Index] = PeriodLowExt6;
  }
}

SCSFExport scsf_HighLowSession(SCStudyInterfaceRef sc) {
  SCSubgraphRef Subgraph_IBHigh = sc.Subgraph[0];
  SCSubgraphRef Subgraph_IBMid = sc.Subgraph[1];
  SCSubgraphRef Subgraph_IBLow = sc.Subgraph[2];

  SCInputRef Input_Session = sc.Input[0];
  SCInputRef Input_NumberDaysToCalculate = sc.Input[1];

  if (sc.SetDefaults) {
    sc.GraphName = "Session's High/Low";
    sc.DrawZeros = 0;
    sc.GraphRegion = 0;
    sc.AutoLoop = 1;

    sc.ScaleRangeType = SCALE_SAMEASREGION;

    Subgraph_IBHigh.Name = "High";
    Subgraph_IBHigh.PrimaryColor = RGB(128, 255, 128);
    Subgraph_IBHigh.DrawStyle = DRAWSTYLE_DASH;
    Subgraph_IBHigh.DrawZeros = false;

    Subgraph_IBMid.Name = "Mid";
    Subgraph_IBMid.PrimaryColor = RGB(255, 255, 255);
    Subgraph_IBMid.DrawStyle = DRAWSTYLE_DASH;
    Subgraph_IBMid.DrawZeros = false;

    Subgraph_IBLow.Name = "Low";
    Subgraph_IBLow.PrimaryColor = RGB(255, 128, 128);
    Subgraph_IBLow.DrawStyle = DRAWSTYLE_DASH;
    Subgraph_IBLow.DrawZeros = false;

    // Inputs
    Input_Session.Name = "High/Low for Session";
    Input_Session.SetCustomInputStrings("Day Session (RTH); Evening Session (ETH)");
    Input_Session.SetCustomInputIndex(0);

    Input_NumberDaysToCalculate.Name = "Number of Days to Calculate";
    Input_NumberDaysToCalculate.SetInt(100);
    Input_NumberDaysToCalculate.SetIntLimits(1, INT_MAX);

    return;
  }

  // Persist vars
  int &PeriodFirstIndex = sc.GetPersistentInt(1);

  SCDateTime &PeriodStartDateTime = sc.GetPersistentSCDateTime(1);
  SCDateTime &PeriodEndDateTime = sc.GetPersistentSCDateTime(2);

  float &PeriodHigh = sc.GetPersistentFloat(1);
  float &PeriodLow = sc.GetPersistentFloat(2);
  float &PeriodMid = sc.GetPersistentFloat(3);

  // Reset persistent variables upon full calculation
  if (sc.Index == 0) {
    PeriodFirstIndex = -1;
    PeriodStartDateTime = 0;
    PeriodEndDateTime = 0;
    PeriodHigh = -FLT_MAX;
    PeriodLow = FLT_MAX;
  }

  SCDateTimeMS LastBarDateTime = sc.BaseDateTimeIn[sc.ArraySize - 1];
  SCDateTimeMS FirstCalculationDate =
      LastBarDateTime.GetDate() - SCDateTime::DAYS(Input_NumberDaysToCalculate.GetInt() - 1);

  SCDateTimeMS CurrentBarDateTime = sc.BaseDateTimeIn[sc.Index];

  SCDateTimeMS PrevBarDateTime;

  if (sc.Index > 0)
    PrevBarDateTime = sc.BaseDateTimeIn[sc.Index - 1];

  if (CurrentBarDateTime.GetDate() < FirstCalculationDate) // Limit calculation to specified number
                                                           // of days back
    return;

  SCDateTimeMS StartDateTime = CurrentBarDateTime;

  // Set StartTime for selected session
  if (Input_Session.GetIndex() == 0)
    StartDateTime.SetTime(sc.StartTime1);
  else
    StartDateTime.SetTime(sc.StartTime2);

  if (PrevBarDateTime < StartDateTime && CurrentBarDateTime >= StartDateTime) {
    PeriodFirstIndex = sc.Index;
    PeriodHigh = -FLT_MAX;
    PeriodLow = FLT_MAX;

    PeriodStartDateTime = StartDateTime;

    // Set Period End Time as end of selected session
    PeriodEndDateTime = PeriodStartDateTime;
    if (Input_Session.GetIndex() == 0)
      PeriodEndDateTime.SetTime(sc.EndTime1 - 1);
    else
      PeriodEndDateTime.SetTime(sc.EndTime2 - 1);

    // add a day if EndTime is on the next day
    if (PeriodEndDateTime < PeriodStartDateTime)
      PeriodEndDateTime.AddDays(1);
  }

  // Check end of period
  if (PeriodFirstIndex >= 0) {
    if (CurrentBarDateTime > PeriodEndDateTime) {
      PeriodFirstIndex = -1;
    }
  }

  // Collecting data, back propagate if changed
  if (PeriodFirstIndex >= 0) {
    bool Changed = false;

    if (sc.High[sc.Index] > PeriodHigh) {
      PeriodHigh = sc.High[sc.Index];
      Changed = true;
    }

    if (sc.Low[sc.Index] < PeriodLow) {
      PeriodLow = sc.Low[sc.Index];
      Changed = true;
    }

    if (Changed) {
      PeriodMid = (PeriodHigh + PeriodLow) / 2.0f;

      for (int Index = PeriodFirstIndex; Index < sc.Index; Index++) {
        Subgraph_IBHigh[Index] = PeriodHigh;
        Subgraph_IBLow[Index] = PeriodLow;
        Subgraph_IBMid[Index] = PeriodMid;
      }

      sc.EarliestUpdateSubgraphDataArrayIndex = PeriodFirstIndex;
    }
  }

  // Plot current values
  if (PeriodLow != FLT_MAX) {
    Subgraph_IBHigh[sc.Index] = PeriodHigh;
    Subgraph_IBLow[sc.Index] = PeriodLow;
    Subgraph_IBMid[sc.Index] = PeriodMid;
  }
}
