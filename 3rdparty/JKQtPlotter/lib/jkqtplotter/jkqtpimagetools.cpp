/*
    Copyright (c) 2008-2022 Jan W. Krieger (<jan@jkrieger.de>)

    

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License (LGPL) for more details.

    You should have received a copy of the GNU Lesser General Public License (LGPL)
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "jkqtplotter/jkqtpimagetools.h"
#include <QPainter>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include "jkqtplotter/jkqtpbaseelements.h"
#include "jkqtplotter/jkqtpbaseplotter.h"
#include "jkqtcommon/jkqtpdrawingtools.h"
#include "jkqtplotter/graphs/jkqtpimage.h"
#ifdef QT_XML_LIB
#  include <QtXml/QtXml>
#endif





JKQTPColorPaletteStyleAndToolsMixin::JKQTPColorPaletteStyleAndToolsMixin(JKQTBasePlotter *parent)
{
    cbParent=parent;
    colorBarRightAxis=new JKQTPVerticalIndependentAxis(0, 100, 0, 100, parent);
    colorBarRightAxis->setAxisLabel("");
    colorBarTopAxis=new JKQTPHorizontalIndependentAxis(0, 100, 0, 100, parent);
    colorBarTopAxis->setAxisLabel("");


    palette=JKQTPMathImageMATLAB;
    imageNameFontName=parent->getDefaultTextFontName();
    imageNameFontSize=parent->getDefaultTextSize();
    imageName="";
    showColorBar=true;
    colorBarWidth=14;
    colorBarRelativeHeight=0.75;
    autoImageRange=true;
    imageMin=0;
    imageMax=1;
    colorBarOffset=4;
    rangeMinFailAction=JKQTPMathImageLastPaletteColor;
    rangeMaxFailAction=JKQTPMathImageLastPaletteColor;
    rangeMinFailColor=QColor("black");
    rangeMaxFailColor=QColor("black");
    nanColor=QColor("black");
    infColor=QColor("black");
    colorBarTopVisible=false;
    colorBarRightVisible=true;

    if (parent) palette=parent->getCurrentPlotterStyle().graphsStyle.defaultPalette;

}

JKQTPColorPaletteStyleAndToolsMixin::~JKQTPColorPaletteStyleAndToolsMixin()
= default;

void JKQTPColorPaletteStyleAndToolsMixin::cbGetOutsideSize(JKQTPEnhancedPainter& painter, int& leftSpace, int& rightSpace, int& topSpace, int& bottomSpace) {
    Q_UNUSED(leftSpace);
    Q_UNUSED(bottomSpace);
    if (showColorBar) {
        double internalDataMin=0;
        double internalDataMax=0;
        cbGetDataMinMax(internalDataMin, internalDataMax);
        if (colorBarRightVisible) {
            rightSpace+=cbParent->pt2px(painter, colorBarWidth+colorBarOffset);
            colorBarRightAxis->setRange(internalDataMin, internalDataMax);
            colorBarRightAxis->setAxisWidth(colorBarRelativeHeight*cbParent->getPlotHeight());
            const auto s1=colorBarRightAxis->getSize2(painter);
            const auto s2=colorBarRightAxis->getSize1(painter);
            const QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);
            rightSpace+=qMax(static_cast<double>(s1.requiredSize+s2.requiredSize), static_cast<double>(names.width()));
        }
        if (colorBarTopVisible) {
            //topSpace+=cbParent->pt2px(painter, colorBarWidth+colorBarOffset);
            colorBarTopAxis->setRange(internalDataMin, internalDataMax);
            colorBarTopAxis->setAxisWidth(colorBarRelativeHeight*cbParent->getPlotWidth());
            const auto s1=colorBarTopAxis->getSize2(painter);
            const auto s2=colorBarTopAxis->getSize1(painter);
            const QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);
            topSpace+=cbParent->pt2px(painter, colorBarWidth+colorBarOffset)+qMax(static_cast<double>(s1.requiredSize+s2.requiredSize), static_cast<double>(names.height()));
        }
    }
}

void JKQTPColorPaletteStyleAndToolsMixin::cbDrawOutside(JKQTPEnhancedPainter& painter, QRect leftSpace, QRect rightSpace, QRect topSpace, QRect bottomSpace) {
    Q_UNUSED(leftSpace);
    Q_UNUSED(bottomSpace);
    if (showColorBar) {
     double internalDataMin=0;
     double internalDataMax=0;
     cbGetDataMinMax(internalDataMin, internalDataMax);
     if (colorBarRightVisible) {
         painter.save(); auto __finalpaint=JKQTPFinally([&painter]() {painter.restore();});

         static const uint8_t h=1;
         static const uint8_t dSize = 200*h;
         uint8_t d[dSize];//, dd[200*h];
         for (uint8_t i=0; i<200; i++) {
          for (int j=0; j<h; j++) {
              d[static_cast<int>(i)*h+j]=i;
              //dd[i*h+j]=j;
          }
         }
         QImage b(h, 200, QImage::Format_ARGB32);
         JKQTPImageTools::array2image<uint8_t>(d, h, 200, b, palette, 0, 199);


         QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);

         double icolorBarRelativeHeight=colorBarRelativeHeight;
         int barHeight=static_cast<int>(rightSpace.height()*icolorBarRelativeHeight);
         if (barHeight<1) barHeight=1;
         // find a height for the bar that allows to show the image name
         while ((barHeight>1) && ((rightSpace.height()-barHeight)/2<names.height())) {
          barHeight--;
         }

         //qDebug()<<"r: "<<imageName<<rightSpace;

         QRectF cb(static_cast<int>(rightSpace.x()+cbParent->pt2px(painter, colorBarOffset)), rightSpace.top()+(rightSpace.height()-barHeight)/2, static_cast<int>(cbParent->pt2px(painter, colorBarWidth)), barHeight);
         painter.drawImage(cb, b.mirrored(true, false));
         QPen p=painter.pen();
         p.setColor(colorBarRightAxis->getAxisColor());
         p.setWidthF(qMax(JKQTPlotterDrawingTools::ABS_MIN_LINEWIDTH, cbParent->pt2px(painter, colorBarRightAxis->getLineWidth()*cbParent->getLineWidthMultiplier())));

         colorBarRightAxis->setRange(internalDataMin, internalDataMax);
         colorBarRightAxis->setAxisWidth(cb.height());
         colorBarRightAxis->setAxisOffset(cb.top());
         colorBarRightAxis->setOtherAxisOffset(cb.left());
         colorBarRightAxis->setOtherAxisWidth(cb.width());
         colorBarRightAxis->drawAxes(painter);

         if ( colorBarRightAxis->getDrawMode1().testFlag(JKQTPCADMLine) && colorBarRightAxis->getDrawMode2().testFlag(JKQTPCADMLine)) {
             painter.setPen(p);
             painter.drawLine(QLineF(cb.topLeft(), cb.topRight()));
             painter.drawLine(QLineF(cb.bottomLeft(), cb.bottomRight()));
         }


         cbParent->getMathText()->setFontSize(imageNameFontSize*cbParent->getFontSizeMultiplier());
         cbParent->getMathText()->setFontSpecial(imageNameFontName);
         cbParent->getMathText()->parse(imageName);
         cbParent->getMathText()->draw(painter, Qt::AlignHCenter|Qt::AlignVCenter, QRectF(rightSpace.x(), rightSpace.y(), rightSpace.width(), (rightSpace.height()-barHeight)/2),  cbParent->getCurrentPlotterStyle().debugShowTextBoxes);


     }
     if (colorBarTopVisible) {
         painter.save(); auto __finalpaint=JKQTPFinally([&painter]() {painter.restore();});

         static const uint8_t h=1;
         static const uint8_t dSize = 200*h;
         uint8_t d[dSize];//, dd[200*h];
         for (uint8_t i=0; i<200; i++) {
          for (int j=0; j<h; j++) {
              d[static_cast<int>(i)*h+j]=i;
              //dd[i*h+j]=j;
          }
         }
         QImage b(h,200, QImage::Format_ARGB32);
         JKQTPImageTools::array2image<uint8_t>(d,h,200, b, palette, 0, 199);

         QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);

         double icolorBarRelativeHeight=colorBarRelativeHeight;
         int barWidth=static_cast<int>(topSpace.width()*icolorBarRelativeHeight);
         if (barWidth<1) barWidth=1;
         // find a height for the bar that allows to show the image name
         while ((barWidth>1) && ((topSpace.width()-barWidth)/2<names.width())) {
          barWidth--;
         }


         QRectF cb(topSpace.x()+(topSpace.width()-barWidth)/2, static_cast<int>(topSpace.bottom()-cbParent->pt2px(painter, colorBarOffset+(colorBarWidth))), barWidth, static_cast<int>(cbParent->pt2px(painter, colorBarWidth)));
         //qDebug()<<"t: "<<imageName<<topSpace<<topSpace.bottom()<<colorBarOffset<<"\n     "<<cb;
         QTransform rm;
         rm.rotate(90);
         painter.drawImage(cb, b.transformed(rm));
         QPen p=painter.pen();
         p.setColor(colorBarTopAxis->getAxisColor());
         p.setWidthF(qMax(JKQTPlotterDrawingTools::ABS_MIN_LINEWIDTH, cbParent->pt2px(painter, colorBarTopAxis->getLineWidth()*cbParent->getLineWidthMultiplier())));
         painter.setPen(p);
         //painter.drawRect(cb);


         colorBarTopAxis->setRange(internalDataMin, internalDataMax);
         colorBarTopAxis->setAxisWidth(cb.width());
         colorBarTopAxis->setAxisOffset(cb.left());
         colorBarTopAxis->setOtherAxisOffset(cb.top());
         colorBarTopAxis->setOtherAxisWidth(cb.height());
         colorBarTopAxis->drawAxes(painter);

         if ( colorBarTopAxis->getDrawMode1().testFlag(JKQTPCADMLine) && colorBarTopAxis->getDrawMode2().testFlag(JKQTPCADMLine)) {
             painter.setPen(p);
             painter.drawLine(QLineF(cb.topLeft(), cb.bottomLeft()));
             painter.drawLine(QLineF(cb.topRight(), cb.bottomRight()));
         }


         cbParent->getMathText()->setFontSize(imageNameFontSize*cbParent->getFontSizeMultiplier());
         cbParent->getMathText()->setFontSpecial(imageNameFontName);
         cbParent->getMathText()->parse(imageName);
         cbParent->getMathText()->draw(painter, Qt::AlignHCenter|Qt::AlignVCenter, QRectF(topSpace.right()-(topSpace.width()-barWidth)/2, topSpace.y(), (topSpace.width()-barWidth)/2, topSpace.height()),  cbParent->getCurrentPlotterStyle().debugShowTextBoxes);


     }
    }
}


QStringList JKQTPColorPaletteStyleAndToolsMixin::getPalettes()  {
    return JKQTPImageTools::getPredefinedPalettes();
}

int JKQTPColorPaletteStyleAndToolsMixin::getPalettesCount()
{
    return getPalettes().size();
}

QIcon JKQTPColorPaletteStyleAndToolsMixin::getPaletteIcon(int i)  {
    QImage img=getPaletteImage(i, static_cast<size_t>(JKQTPImageTools::PALETTE_ICON_WIDTH));
    QPixmap pix(JKQTPImageTools::PALETTE_ICON_WIDTH,8);
    QRectF r(0,0,JKQTPImageTools::PALETTE_ICON_WIDTH-1,7);
    QPainter p(&pix);
    p.drawImage(r, img);
    p.setPen(QPen(QColor("black")));
    p.drawRect(r);
    p.end();
    return QIcon(pix);

}

QIcon JKQTPColorPaletteStyleAndToolsMixin::getPaletteIcon(JKQTPMathImageColorPalette palette)  {
    return getPaletteIcon(static_cast<int>(palette));
}

QImage JKQTPColorPaletteStyleAndToolsMixin::getPaletteImage(int i, size_t width)
{
    QImage img;
    std::vector<double> pic(width, 0.0);
    for (size_t j=0; j<width; j++) {
     pic[j]=j;
    }
    JKQTPImageTools::array2image<double>(pic.data(), jkqtp_bounded<int>(width), 1, img, static_cast<JKQTPMathImageColorPalette>(i), 0, width-1);
    return img;
}

QImage JKQTPColorPaletteStyleAndToolsMixin::getPaletteImage(JKQTPMathImageColorPalette palette, size_t width)
{
    return getPaletteImage(static_cast<int>(palette), width);
}

QIcon JKQTPColorPaletteStyleAndToolsMixin::getPaletteKeyIcon(int i)
{
    QImage img=getPaletteKeyImage(i, static_cast<size_t>(JKQTPImageTools::PALETTE_ICON_WIDTH), static_cast<size_t>(JKQTPImageTools::PALETTE_IMAGEICON_HEIGHT));
    QPixmap pix(JKQTPImageTools::PALETTE_ICON_WIDTH,JKQTPImageTools::PALETTE_IMAGEICON_HEIGHT);
    QRectF r(0,0,JKQTPImageTools::PALETTE_ICON_WIDTH-1,JKQTPImageTools::PALETTE_IMAGEICON_HEIGHT-1);
    QPainter p(&pix);
    p.drawImage(r, img);
    p.setPen(QPen(QColor("black")));
    p.drawRect(r);
    p.end();
    return QIcon(pix);
}

QIcon JKQTPColorPaletteStyleAndToolsMixin::getPaletteKeyIcon(JKQTPMathImageColorPalette palette)
{
    return getPaletteIcon(static_cast<int>(palette));
}

QImage JKQTPColorPaletteStyleAndToolsMixin::getPaletteKeyImage(int i, size_t width, size_t height)
{
    QImage img;

    const double x01=double(width)/3.0;
    const double y01=double(height)/3.0*2.0;
    const double w1x=double(width*width)/(5.0*5.0);
    const double w1y=double(height*height)/(2.0*2.0);
    const double x02=double(width)/3.0*2.0;
    const double y02=double(height)/4.0;
    const double w2x=double(width*width)/(8.0*8.0);
    const double w2y=double(height*height)/(8.0*8.0);

    const size_t NPixels=jkqtp_bounded<size_t>(width*height);
    std::vector<double> pic(NPixels, 0.0);
    double mmax=0;
    for (size_t j=0; j<NPixels; j++) {
     const size_t x=j%width;
     const size_t y=j/width;
     pic[j]=exp(-0.5*(double((x-x01)*double(x-x01))/w1x+double((y-y01)*double(y-y01))/w1y))+0.7*exp(-0.5*(double((x-x02)*double(x-x02))/w2x+double((y-y02)*double(y-y02))/w2y));
     if (pic[j]>mmax) mmax=pic[j];
    }
    JKQTPImageTools::array2image<double>(pic.data(), static_cast<int>(width), static_cast<int>(height), img, static_cast<JKQTPMathImageColorPalette>(i), 0, mmax);
    return img;
}

QImage JKQTPColorPaletteStyleAndToolsMixin::getPaletteKeyImage(JKQTPMathImageColorPalette palette, size_t width, size_t height)
{
    return getPaletteKeyImage(static_cast<int>(palette), width, height);
}


void JKQTPColorPaletteStyleAndToolsMixin::setColorPalette(int pal) {
    palette=static_cast<JKQTPMathImageColorPalette>(pal);
}

void JKQTPColorPaletteStyleAndToolsMixin::cbSetParent(JKQTBasePlotter* parent) {
    cbParent=parent;
    colorBarRightAxis->setParent(parent);
    colorBarTopAxis->setParent(parent);

}

void JKQTPColorPaletteStyleAndToolsMixin::setColorPalette(const JKQTPMathImageColorPalette &__value)
{
    palette = __value;
}

JKQTPMathImageColorPalette JKQTPColorPaletteStyleAndToolsMixin::getColorPalette() const
{
    return palette;
}

void JKQTPColorPaletteStyleAndToolsMixin::setRangeMinFailAction(const JKQTPMathImageColorRangeFailAction &__value)
{
    rangeMinFailAction = __value;
}

JKQTPMathImageColorRangeFailAction JKQTPColorPaletteStyleAndToolsMixin::getActionRangeMinFail() const
{
    return rangeMinFailAction;
}

void JKQTPColorPaletteStyleAndToolsMixin::setRangeMaxFailAction(const JKQTPMathImageColorRangeFailAction &__value)
{
    rangeMaxFailAction = __value;
}

JKQTPMathImageColorRangeFailAction JKQTPColorPaletteStyleAndToolsMixin::getActionRangeMaxFail() const
{
    return rangeMaxFailAction;
}

void JKQTPColorPaletteStyleAndToolsMixin::setRangeMinFailColor(const QColor &__value)
{
    rangeMinFailColor = __value;
}

QColor JKQTPColorPaletteStyleAndToolsMixin::getRangeMinFailColor() const
{
    return rangeMinFailColor;
}

void JKQTPColorPaletteStyleAndToolsMixin::setRangeMaxFailColor(const QColor &__value)
{
    rangeMaxFailColor = __value;
}

QColor JKQTPColorPaletteStyleAndToolsMixin::getRangeMaxFailColor() const
{
    return rangeMaxFailColor;
}

void JKQTPColorPaletteStyleAndToolsMixin::setNanColor(const QColor &__value)
{
    nanColor = __value;
}

QColor JKQTPColorPaletteStyleAndToolsMixin::getNanColor() const
{
    return nanColor;
}

void JKQTPColorPaletteStyleAndToolsMixin::setInfColor(const QColor &__value)
{
    infColor = __value;
}

QColor JKQTPColorPaletteStyleAndToolsMixin::getInfColor() const
{
    return infColor;
}

void JKQTPColorPaletteStyleAndToolsMixin::setShowColorBar(bool __value)
{
    showColorBar = __value;
}

bool JKQTPColorPaletteStyleAndToolsMixin::getShowColorBar() const
{
    return showColorBar;
}

void JKQTPColorPaletteStyleAndToolsMixin::setColorBarWidth(int __value)
{
    colorBarWidth = __value;
}

int JKQTPColorPaletteStyleAndToolsMixin::getColorBarWidth() const
{
    return colorBarWidth;
}

void JKQTPColorPaletteStyleAndToolsMixin::setColorBarOffset(int __value)
{
    colorBarOffset = __value;
}

int JKQTPColorPaletteStyleAndToolsMixin::getColorBarOffset() const
{
    return colorBarOffset;
}

void JKQTPColorPaletteStyleAndToolsMixin::setColorBarRelativeHeight(double __value)
{
    colorBarRelativeHeight = __value;
}

double JKQTPColorPaletteStyleAndToolsMixin::getColorBarRelativeHeight() const
{
    return colorBarRelativeHeight;
}

void JKQTPColorPaletteStyleAndToolsMixin::setImageMin(double __value)
{
    imageMin = __value;
}

double JKQTPColorPaletteStyleAndToolsMixin::getImageMin() const
{
    return imageMin;
}

void JKQTPColorPaletteStyleAndToolsMixin::setImageMax(double __value)
{
    imageMax = __value;
}

double JKQTPColorPaletteStyleAndToolsMixin::getImageMax() const
{
    return imageMax;
}

void JKQTPColorPaletteStyleAndToolsMixin::setAutoImageRange(bool __value)
{
    autoImageRange = __value;
}

bool JKQTPColorPaletteStyleAndToolsMixin::getAutoImageRange() const
{
    return autoImageRange;
}

void JKQTPColorPaletteStyleAndToolsMixin::setImageName(const QString &__value)
{
    imageName = __value;
}

QString JKQTPColorPaletteStyleAndToolsMixin::getImageName() const
{
    return imageName;
}

void JKQTPColorPaletteStyleAndToolsMixin::setImageNameFontName(const QString &__value)
{
    imageNameFontName = __value;
}

QString JKQTPColorPaletteStyleAndToolsMixin::getImageNameFontName() const
{
    return imageNameFontName;
}

void JKQTPColorPaletteStyleAndToolsMixin::setImageNameFontSize(double __value)
{
    imageNameFontSize = __value;
}

double JKQTPColorPaletteStyleAndToolsMixin::getImageNameFontSize() const
{
    return imageNameFontSize;
}

JKQTPVerticalIndependentAxis *JKQTPColorPaletteStyleAndToolsMixin::getColorBarRightAxis() {
    return colorBarRightAxis;
}

JKQTPHorizontalIndependentAxis *JKQTPColorPaletteStyleAndToolsMixin::getColorBarTopAxis() {
    return colorBarTopAxis;
}

const JKQTPVerticalIndependentAxis *JKQTPColorPaletteStyleAndToolsMixin::getColorBarRightAxis() const  {
    return colorBarRightAxis;
}

const JKQTPHorizontalIndependentAxis *JKQTPColorPaletteStyleAndToolsMixin::getColorBarTopAxis() const  {
    return colorBarTopAxis;
}

void JKQTPColorPaletteStyleAndToolsMixin::setColorBarTopVisible(bool __value)
{
    colorBarTopVisible = __value;
}

bool JKQTPColorPaletteStyleAndToolsMixin::getColorBarTopVisible() const
{
    return colorBarTopVisible;
}

void JKQTPColorPaletteStyleAndToolsMixin::setColorBarRightVisible(bool __value)
{
    colorBarRightVisible = __value;
}

bool JKQTPColorPaletteStyleAndToolsMixin::getColorBarRightVisible() const
{
    return colorBarRightVisible;
}


JKQTPColorPaletteWithModifierStyleAndToolsMixin::JKQTPColorPaletteWithModifierStyleAndToolsMixin(JKQTBasePlotter *parent):
    JKQTPColorPaletteStyleAndToolsMixin(parent)
{
    modifierColorBarTopAxis=new JKQTPVerticalIndependentAxis(0, 100, 0, 100, parent);
    modifierColorBarTopAxis->setAxisLabel("");
    modifierColorBarRightAxis=new JKQTPHorizontalIndependentAxis (0, 100, 0, 100, parent);
    modifierColorBarRightAxis->setAxisLabel("");
    colorBarModifiedWidth=80;
    modifierMode=JKQTPMathImageModifierMode::ModifyNone;
    modifierMin=0;
    modifierMax=0;
    autoModifierRange=true;
}



void JKQTPColorPaletteWithModifierStyleAndToolsMixin::cbGetOutsideSize(JKQTPEnhancedPainter &painter, int &leftSpace, int &rightSpace, int &topSpace, int &bottomSpace)
{
    Q_UNUSED(leftSpace);
    Q_UNUSED(bottomSpace);
    if (showColorBar) {
        double internalDataMin=0;
        double internalDataMax=0;
        double internalModifierMin=0;
        double internalModifierMax=0;
        const double axisSepPT=cbParent->getCurrentPlotterStyle().secondaryAxisSeparation;
        cbGetDataMinMax(internalDataMin, internalDataMax);
        cbGetModifierDataMinMax(internalDataMin, internalDataMax);
        if (colorBarRightVisible) {
            rightSpace+=cbParent->pt2px(painter, colorBarWidth+colorBarOffset+axisSepPT);
            colorBarRightAxis->setRange(internalDataMin, internalDataMax);
            colorBarRightAxis->setAxisWidth(colorBarRelativeHeight*cbParent->getPlotHeight());
            modifierColorBarRightAxis->setRange(internalModifierMin, internalModifierMax);
            modifierColorBarRightAxis->setAxisWidth(cbParent->pt2px(painter, colorBarModifiedWidth));
            const auto s1=colorBarRightAxis->getSize2(painter);
            const auto s2=colorBarRightAxis->getSize1(painter);
            const QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);
            rightSpace+=qMax(static_cast<double>(s1.requiredSize+s2.requiredSize), static_cast<double>(names.width()));
            if (modifierMode!=JKQTPMathImageModifierMode::ModifyNone) {
                rightSpace=rightSpace+static_cast<int>(2.0*cbParent->pt2px(painter, colorBarModifiedWidth-colorBarWidth));
            }
        }
        if (colorBarTopVisible) {
            //topSpace+=cbParent->pt2px(painter, colorBarWidth+colorBarOffset);
            colorBarTopAxis->setRange(internalDataMin, internalDataMax);
            colorBarTopAxis->setAxisWidth(colorBarRelativeHeight*cbParent->getPlotWidth());
            modifierColorBarTopAxis->setRange(internalModifierMin, internalModifierMax);
            modifierColorBarTopAxis->setAxisWidth(cbParent->pt2px(painter, colorBarModifiedWidth));
            const auto s1=colorBarTopAxis->getSize2(painter);
            const auto s2=colorBarTopAxis->getSize1(painter);
            const QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);
            topSpace+= cbParent->pt2px(painter, colorBarWidth+colorBarOffset+axisSepPT)+qMax(static_cast<double>(s1.requiredSize+s2.requiredSize), static_cast<double>(names.height()));
            if (modifierMode!=JKQTPMathImageModifierMode::ModifyNone) {
                topSpace=topSpace+static_cast<int>(2.0*cbParent->pt2px(painter, colorBarModifiedWidth-colorBarWidth));
            }
        }
    }
}

void JKQTPColorPaletteWithModifierStyleAndToolsMixin::cbDrawOutside(JKQTPEnhancedPainter &painter, QRect /*leftSpace*/, QRect rightSpace, QRect topSpace, QRect /*bottomSpace*/)
{
    if (showColorBar) {
        double internalDataMin=0;
        double internalDataMax=0;
        double internalModifierMin=0;
        double internalModifierMax=0;
        cbGetDataMinMax(internalDataMin, internalDataMax);
        cbGetModifierDataMinMax(internalModifierMin, internalModifierMax);
        uint8_t h=1;
        if (modifierMode!=JKQTPMathImageModifierMode::ModifyNone) {
            h=50;
        }
        const uint8_t steps=200;
        const int dSize = steps*h;
        std::vector<double> d(dSize, 0.0);
        std::vector<double> dd(dSize, 0.0);
        for (uint8_t i=0; i<steps; i++) {
            for (uint8_t j=0; j<h; j++) {
                d[i*h+j]=i;
                dd[i*h+j]=j;
            }
        }

        QImage b(h, steps, QImage::Format_ARGB32);
        JKQTPImageTools::array2image<double>(d.data(),h, steps, b, palette, 0, steps-1);
        if (modifierMode!=JKQTPMathImageModifierMode::ModifyNone) {
            modifyImage(b, dd.data(), JKQTPMathImageDataType::DoubleArray, h, steps, 0, h-1);
        }

        if (colorBarRightVisible) {
            painter.save(); auto __finalpaint=JKQTPFinally([&painter]() {painter.restore();});

            QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);

            double icolorBarRelativeHeight=colorBarRelativeHeight;
            int barHeight=static_cast<int>(rightSpace.height()*icolorBarRelativeHeight);
            if (barHeight<1) barHeight=1;
            // find a height for the bar that allows to show the image name
            while ((barHeight>1) && ((rightSpace.height()-barHeight)/2<names.height())) {
                barHeight--;
            }

            //qDebug()<<"r: "<<imageName<<rightSpace;

            QRectF cb(static_cast<int>(rightSpace.x()+cbParent->pt2px(painter, colorBarOffset)), rightSpace.top()+(rightSpace.height()-barHeight)/2, static_cast<int>(cbParent->pt2px(painter, (modifierMode==JKQTPMathImageModifierMode::ModifyNone)?colorBarWidth:colorBarModifiedWidth)), barHeight);
            painter.drawImage(cb, b.mirrored(true, false));
            QPen p=painter.pen();
            p.setColor(colorBarRightAxis->getAxisColor());
            p.setWidthF(qMax(JKQTPlotterDrawingTools::ABS_MIN_LINEWIDTH, cbParent->pt2px(painter, colorBarRightAxis->getLineWidth()*cbParent->getLineWidthMultiplier())));

            colorBarRightAxis->setRange(internalDataMin, internalDataMax);
            colorBarRightAxis->setAxisWidth(cb.height());
            colorBarRightAxis->setAxisOffset(cb.top());
            colorBarRightAxis->setOtherAxisOffset(cb.left());
            colorBarRightAxis->setOtherAxisWidth(cb.width());
            //qDebug()<<"=================================================================\ncolorBarRightAxis:";
            colorBarRightAxis->drawAxes(painter);

            if (modifierMode!=JKQTPMathImageModifierMode::ModifyNone) {
                modifierColorBarRightAxis->setRange(internalModifierMin, internalModifierMax);
                modifierColorBarRightAxis->setAxisWidth(cbParent->pt2px(painter, colorBarModifiedWidth));
                modifierColorBarRightAxis->setAxisOffset(cb.left());
                modifierColorBarRightAxis->setOtherAxisOffset(cb.top());
                modifierColorBarRightAxis->setOtherAxisWidth(cb.height());
                modifierColorBarRightAxis->drawAxes(painter);
            } else {
                if ( colorBarRightAxis->getDrawMode1().testFlag(JKQTPCADMLine) && colorBarRightAxis->getDrawMode2().testFlag(JKQTPCADMLine)) {
                    painter.setPen(p);
                    painter.drawLine(QLineF(cb.topLeft(), cb.topRight()));
                    painter.drawLine(QLineF(cb.bottomLeft(), cb.bottomRight()));
                }
            }

            cbParent->getMathText()->setFontSize(imageNameFontSize*cbParent->getFontSizeMultiplier());
            cbParent->getMathText()->setFontSpecial(cbParent->getCurrentPlotterStyle().defaultFontName);
            cbParent->getMathText()->parse(imageName);
            cbParent->getMathText()->draw(painter, Qt::AlignHCenter|Qt::AlignVCenter, QRectF(rightSpace.x(), rightSpace.y(), rightSpace.width(), (rightSpace.height()-barHeight)/2),  cbParent->getCurrentPlotterStyle().debugShowTextBoxes);


        }
        if (colorBarTopVisible) {
            painter.save(); auto __finalpaint=JKQTPFinally([&painter]() {painter.restore();});

            QSizeF names=cbParent->getTextSizeSize(imageNameFontName, imageNameFontSize*cbParent->getFontSizeMultiplier(), imageName, painter);

            double icolorBarRelativeHeight=colorBarRelativeHeight;
            int barWidth=topSpace.width()*icolorBarRelativeHeight;
            if (barWidth<1) barWidth=1;
            // find a height for the bar that allows to show the image name
            while ((barWidth>1) && ((topSpace.width()-barWidth)/2<names.width())) {
                barWidth--;
            }


            QRectF cb(topSpace.x()+(topSpace.width()-barWidth)/2, topSpace.bottom()-cbParent->pt2px(painter, colorBarOffset+((modifierMode==JKQTPMathImageModifierMode::ModifyNone)?colorBarWidth:colorBarModifiedWidth)), barWidth, cbParent->pt2px(painter, (modifierMode==JKQTPMathImageModifierMode::ModifyNone)?colorBarWidth:colorBarModifiedWidth));
            //qDebug()<<"t: "<<imageName<<topSpace<<topSpace.bottom()<<colorBarOffset<<"\n     "<<cb;
            QTransform rm;
            rm.rotate(90);
            painter.drawImage(cb, b.transformed(rm));
            QPen p=painter.pen();
            p.setColor(colorBarTopAxis->getAxisColor());
            p.setWidthF(qMax(JKQTPlotterDrawingTools::ABS_MIN_LINEWIDTH, cbParent->pt2px(painter, colorBarTopAxis->getLineWidth()*cbParent->getLineWidthMultiplier())));


            colorBarTopAxis->setRange(internalDataMin, internalDataMax);
            colorBarTopAxis->setAxisWidth(cb.width());
            colorBarTopAxis->setAxisOffset(cb.left());
            colorBarTopAxis->setOtherAxisOffset(cb.top());
            colorBarTopAxis->setOtherAxisWidth(cb.height());
            colorBarTopAxis->drawAxes(painter);

            if (modifierMode!=JKQTPMathImageModifierMode::ModifyNone) {
                modifierColorBarTopAxis->setRange(internalModifierMin, internalModifierMax);
                modifierColorBarTopAxis->setAxisWidth(cbParent->pt2px(painter, colorBarModifiedWidth));
                modifierColorBarTopAxis->setAxisOffset(cb.top());
                modifierColorBarTopAxis->setOtherAxisOffset(cb.left());
                modifierColorBarTopAxis->setOtherAxisWidth(cb.width());
                modifierColorBarTopAxis->drawAxes(painter);
            } else {
                if ( colorBarTopAxis->getDrawMode1().testFlag(JKQTPCADMLine) && colorBarTopAxis->getDrawMode2().testFlag(JKQTPCADMLine)) {
                    painter.setPen(p);
                    painter.drawLine(QLineF(cb.topLeft(), cb.bottomLeft()));
                    painter.drawLine(QLineF(cb.topRight(), cb.bottomRight()));
                }
            }

            cbParent->getMathText()->setFontSize(imageNameFontSize*cbParent->getFontSizeMultiplier());
            cbParent->getMathText()->setFontSpecial(cbParent->getCurrentPlotterStyle().defaultFontName);
            cbParent->getMathText()->parse(imageName);
            cbParent->getMathText()->draw(painter, Qt::AlignHCenter|Qt::AlignVCenter, QRectF(topSpace.right()-(topSpace.width()-barWidth)/2, topSpace.y(), (topSpace.width()-barWidth)/2, topSpace.height()),  cbParent->getCurrentPlotterStyle().debugShowTextBoxes);


        }
    }
}

void JKQTPColorPaletteWithModifierStyleAndToolsMixin::cbSetParent(JKQTBasePlotter *parent)
{
    JKQTPColorPaletteStyleAndToolsMixin::cbSetParent(parent);
    modifierColorBarTopAxis->setParent(parent);
    modifierColorBarRightAxis->setParent(parent);
}


void JKQTPColorPaletteWithModifierStyleAndToolsMixin::setModifierMode(const JKQTPMathImageModifierMode &__value)
{
    modifierMode = __value;
}

JKQTPMathImageModifierMode JKQTPColorPaletteWithModifierStyleAndToolsMixin::getModifierMode() const
{
    return modifierMode;
}


void JKQTPColorPaletteWithModifierStyleAndToolsMixin::modifyImage(QImage &img, const void *dataModifier, JKQTPMathImageDataType datatypeModifier, int Nx, int Ny, double internalModifierMin, double internalModifierMax)
{
    JKQTPModifyImage(img, modifierMode, dataModifier, datatypeModifier, Nx, Ny, internalModifierMin, internalModifierMax);
}


void JKQTPColorPaletteWithModifierStyleAndToolsMixin::setColorBarModifiedWidth(double __value)
{
    colorBarModifiedWidth = __value;
}

double JKQTPColorPaletteWithModifierStyleAndToolsMixin::getColorBarModifiedWidth() const
{
    return colorBarModifiedWidth;
}


JKQTPVerticalIndependentAxis *JKQTPColorPaletteWithModifierStyleAndToolsMixin::getModifierColorBarTopAxis()  {
    return modifierColorBarTopAxis;
}

JKQTPHorizontalIndependentAxis *JKQTPColorPaletteWithModifierStyleAndToolsMixin::getModifierColorBarRightAxis()   {
    return modifierColorBarRightAxis ;
}


const JKQTPVerticalIndependentAxis *JKQTPColorPaletteWithModifierStyleAndToolsMixin::getModifierColorBarTopAxis() const {
    return modifierColorBarTopAxis;
}

const JKQTPHorizontalIndependentAxis *JKQTPColorPaletteWithModifierStyleAndToolsMixin::getModifierColorBarRightAxis() const {
    return modifierColorBarRightAxis ;
}


void JKQTPColorPaletteWithModifierStyleAndToolsMixin::setAutoModifierRange(bool __value)
{
    autoModifierRange = __value;
}

bool JKQTPColorPaletteWithModifierStyleAndToolsMixin::getAutoModifierRange() const
{
    return autoModifierRange;
}

void JKQTPColorPaletteWithModifierStyleAndToolsMixin::setModifierMin(double __value)
{
    modifierMin = __value;
}

double JKQTPColorPaletteWithModifierStyleAndToolsMixin::getModifierMin() const
{
    return modifierMin;
}

void JKQTPColorPaletteWithModifierStyleAndToolsMixin::setModifierMax(double __value)
{
    modifierMax = __value;
}

double JKQTPColorPaletteWithModifierStyleAndToolsMixin::getModifierMax() const
{
    return modifierMax;
}
