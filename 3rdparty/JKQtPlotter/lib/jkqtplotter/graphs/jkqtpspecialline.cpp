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



#include "jkqtplotter/graphs/jkqtpspecialline.h"
#include "jkqtplotter/jkqtpbaseplotter.h"
#include <stdlib.h>
#include <QDebug>
#include <iostream>
#include "jkqtplotter/jkqtptools.h"
#include "jkqtplotter/jkqtpimagetools.h"
#include "jkqtplotter/graphs/jkqtpimage.h"
#include "jkqtplotter/jkqtpbaseelements.h"
#include "jkqtplotter/jkqtplotter.h"
#define SmallestGreaterZeroCompare_xvsgz() if ((xvsgz>10.0*DBL_MIN)&&((smallestGreaterZero<10.0*DBL_MIN) || (xvsgz<smallestGreaterZero))) smallestGreaterZero=xvsgz;




JKQTPSpecialLineGraphBase::JKQTPSpecialLineGraphBase(JKQTBasePlotter* parent):
    JKQTPXYBaselineGraph(parent), m_drawSymbols(false), m_specialLineType(JKQTPStepLeft)
{
    parentPlotStyle=-1;
    initLineStyle(parent, parentPlotStyle, JKQTPPlotStyleType::Default);
    initFillStyle(parent, parentPlotStyle, JKQTPPlotStyleType::Default);
    initSymbolStyle(parent, parentPlotStyle, JKQTPPlotStyleType::Default);
    setFillCurve(false);
}

void JKQTPSpecialLineGraphBase::drawKeyMarker(JKQTPEnhancedPainter& painter, const QRectF& r) {
    QRectF rect=r;
    painter.save(); auto __finalpaint=JKQTPFinally([&painter]() {painter.restore();});
    QPen p=getKeyLinePen(painter, rect, parent);
    QPen np(Qt::NoPen);
    QBrush b=getFillBrush(painter, parent);
    const double y=rect.top()+rect.height()/2.0;
    rect.setWidth(rect.width()-p.widthF());
    rect.setHeight(rect.height()-p.widthF());
    rect.setX(rect.x()+p.widthF()/2.0);
    rect.setY(rect.y()+p.widthF()/2.0);
    painter.setPen(np);
    if (getDrawLine()) painter.setPen(p);
    painter.setBrush(b);
    if (getFillCurve()) painter.drawRect(rect);
    if (!getFillCurve() && getDrawLine()) painter.drawLine(QLineF(rect.left(), y, rect.right(), y));
    if (m_drawSymbols) {
        JKQTPPlotSymbol(painter, rect.center().x(), rect.center().y(), getSymbolType(), getKeySymbolSizePx(painter, rect, parent), getKeySymbolLineWidthPx(painter, rect, parent), getKeyLabelColor(), getSymbolFillColor(),getSymbolFont());
    }

}


QColor JKQTPSpecialLineGraphBase::getKeyLabelColor() const {
    return getLineColor();
}

void JKQTPSpecialLineGraphBase::setDrawSymbols(bool __value)
{
    m_drawSymbols=__value;
}

bool JKQTPSpecialLineGraphBase::getDrawSymbols() const
{
    return m_drawSymbols;
}


void JKQTPSpecialLineGraphBase::setSpecialLineType(const JKQTPSpecialLineType &__value)
{
    this->m_specialLineType = __value;
}

JKQTPSpecialLineType JKQTPSpecialLineGraphBase::getSpecialLineType() const
{
    return this->m_specialLineType;
}


void JKQTPSpecialLineGraphBase::setColor(QColor c)
{
    setLineColor(c);
    setSymbolColor(c);
    setSymbolFillColor(JKQTPGetDerivedColor(parent->getCurrentPlotterStyle().graphsStyle.defaultGraphStyle.symbolFillColorDerivationMode, c));
    setFillColor(JKQTPGetDerivedColor(parent->getCurrentPlotterStyle().graphsStyle.defaultGraphStyle.fillColorDerivationMode, c));
    c.setAlphaF(0.5);
    setHighlightingLineColor(c);
}



JKQTPSpecialLineHorizontalGraph::JKQTPSpecialLineHorizontalGraph(JKQTBasePlotter* parent):
    JKQTPSpecialLineGraphBase(parent)
{
}


JKQTPSpecialLineHorizontalGraph::JKQTPSpecialLineHorizontalGraph(JKQTPlotter* parent):
    JKQTPSpecialLineHorizontalGraph(parent->getPlotter())
{
}




void JKQTPSpecialLineHorizontalGraph::draw(JKQTPEnhancedPainter& painter) {
#ifdef JKQTBP_AUTOTIMER
    JKQTPAutoOutputTimer jkaaot("JKQTPSpecialLineHorizontalGraph::draw");
#endif
    if (parent==nullptr) return;
    JKQTPDatastore* datastore=parent->getDatastore();
    if (datastore==nullptr) return;

    drawErrorsBefore(painter);

    QPen p=getLinePen(painter, parent);
    QPen ph=getHighlightingLinePen(painter, parent);
    QPen np(Qt::NoPen);
    QBrush b=getFillBrush(painter, parent);
    const double xmin=transformX(getXAxis()->getMin());
    const double xmax=transformX(getXAxis()->getMax());
    const double ymin=transformY(getYAxis()->getMin());
    const double ymax=transformY(getYAxis()->getMax());
    const auto symType=getSymbolType();
    const double symbolSize=parent->pt2px(painter, getSymbolSize());
    const QMarginsF clipMargins=(symType==JKQTPNoSymbol)?QMarginsF(0,0,0,0):QMarginsF(symbolSize,symbolSize,symbolSize,symbolSize);
    const QRectF cliprect=QRectF(qMin(xmin,xmax),qMin(ymin,ymax),fabs(xmax-xmin),fabs(ymax-ymin))+clipMargins;

    int imax=0;
    int imin=0;

    if (getIndexRange(imin, imax)) {


        QPolygonF pl, pf;
        QPolygonF ps;

        double xold=-1;
        double yold=-1;
        double y0=transformY(getBaseline());
        if (getYAxis()->isLogAxis()) {
            if (getBaseline()>0 && getBaseline()>getYAxis()->getMin()) y0=transformY(getBaseline());
            else y0=transformY(getYAxis()->getMin());
        }
        bool firstPoint=true;
        intSortData();
        for (int iii=imin; iii<imax; iii++) {
            const int i=qBound(imin, getDataIndex(iii), imax);
            const double xv=datastore->get(static_cast<size_t>(xColumn),static_cast<size_t>(i));
            const double yv=datastore->get(static_cast<size_t>(yColumn),static_cast<size_t>(i));
            //std::cout<<"(xv, yv) =    ( "<<xv<<", "<<yv<<" )\n";
            if (JKQTPIsOKFloat(xv) && JKQTPIsOKFloat(yv)) {
                const double x=transformX(xv);
                const double y=transformY(yv);
                if (JKQTPIsOKFloat(x) && JKQTPIsOKFloat(y)) {
                    ps.append(QPointF(x,y));
                    if (!firstPoint) {
                        //double xl1=xold;
                        //double yl1=yold;
                        //double xl2=x;
                        //double yl2=y;

                        if (m_specialLineType==JKQTPStepCenter) {
                            //                      x/y
                            //              |--------*
                            //              |
                            //     *--------|
                            // xold/yold
                            const double d=(x-xold);
                            pf<<QPointF(xold+d/2.0, yold);
                            pf<<QPointF(xold+d/2.0, y);
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(xold+d/2.0, yold);
                                pl<<QPointF(xold+d/2.0, y);
                                pl<<QPointF(x, y);
                            }
                        } else if (m_specialLineType==JKQTPStepLeft) {
                            //                     x/y
                            //     |----------------*
                            //     |
                            //     *
                            // xold/yold
                            pf<<QPointF(xold, y);
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(xold, y);
                                pl<<QPointF(x, y);
                            }
                        } else if (m_specialLineType==JKQTPStepRight) {
                            //                     x/y
                            //                      *
                            //                      |
                            //     *----------------|
                            // xold/yold
                            pf<<QPointF(x, yold);
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(x, yold);
                                pl<<QPointF(x, y);
                            }
                        } else if (m_specialLineType==JKQTPStepAverage) {
                            //                     x/y
                            //                      *
                            //                      |
                            //     |----------------|
                            //     |
                            //     *
                            // xold/yold
                            //const double d=(x-xold);
                            const double h=(y-yold);
                            pf<<QPointF(xold, yold+h/2.0);
                            pf<<QPointF(x, yold+h/2.0);
                            pf<<QPointF(x,y);
                            if (getDrawLine()) {
                                pl<<QPointF(xold, yold+h/2.0);
                                pl<<QPointF(x, yold+h/2.0);
                                pl<<QPointF(x,y);
                            }
                        } else if (m_specialLineType==JKQTPDirectLine) {
                            //                     x/y
                            //                 /----*
                            //           /----/
                            //     *----/
                            // xold/yold
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(x, y);
                            }
                        }

                        //std::cout<<"line ("<<xl1<<", "<<yl1<<") -- ("<<xl2<<", "<<yl2<<")"<<std::endl;
                    } else {
                        if (getDrawLine()) pl<<QPointF(x,y);
                        pf<<QPointF(x, y0);
                        pf<<QPointF(x, y);
                        //xstart=x;
                        //ystart=y0;
                    }
                    xold=x;
                    yold=y;
                    firstPoint=false;
                }
            }
        }
        if (getFillCurve()) {
            pf<<QPointF(xold, y0);
        }
        painter.save();
        auto __finalpaint=JKQTPFinally([&painter]() {painter.restore();});

        if (getFillCurve()) {
            painter.setBrush(b);
            painter.setPen(Qt::NoPen);
            painter.drawPolygon(pf.intersected(cliprect));
        }

        QList<QPolygonF> pl_fordrawing;
        if (isHighlighted() || getDrawLine()) {
            if (getUseNonvisibleLineCompression()) pl_fordrawing=JKQTPClipPolyLine(JKQTPSimplifyPolyLines(pl, getNonvisibleLineCompressionAgressiveness()), cliprect);
            else pl_fordrawing=JKQTPClipPolyLine(pl, cliprect);
        }

        if (isHighlighted()) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(ph);
            for (const QPolygonF& lines : pl_fordrawing) {
                painter.drawPolylineFast(lines);
            }
        }

        if (getDrawLine()) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(p);
            for (const QPolygonF& lines : pl_fordrawing) {
                painter.drawPolylineFast(lines);
            }
        }


        if (m_drawSymbols) {
            painter.save();
            auto __finalpaintsym=JKQTPFinally([&painter]() {painter.restore();});
            for (auto& ppoint: ps) {
                if (cliprect.contains(ppoint)) plotStyledSymbol(parent, painter, ppoint.x(), ppoint.y());
            }
        }
    }

    drawErrorsAfter(painter);
}











JKQTPSpecialLineVerticalGraph::JKQTPSpecialLineVerticalGraph(JKQTBasePlotter *parent):
    JKQTPSpecialLineGraphBase(parent)
{
}

JKQTPSpecialLineVerticalGraph::JKQTPSpecialLineVerticalGraph(JKQTPlotter *parent):
    JKQTPSpecialLineVerticalGraph(parent->getPlotter())
{

}

void JKQTPSpecialLineVerticalGraph::draw(JKQTPEnhancedPainter& painter) {
#ifdef JKQTBP_AUTOTIMER
    JKQTPAutoOutputTimer jkaaot("JKQTPSpecialLineVerticalGraph::draw");
#endif
    if (parent==nullptr) return;
    JKQTPDatastore* datastore=parent->getDatastore();
    if (datastore==nullptr) return;

    drawErrorsBefore(painter);

    QPen p=getLinePen(painter, parent);
    QPen ph=getHighlightingLinePen(painter, parent);
    QPen np(Qt::NoPen);
    QBrush b=getFillBrush(painter, parent);
    const double xmin=transformX(getXAxis()->getMin());
    const double xmax=transformX(getXAxis()->getMax());
    const double ymin=transformY(getYAxis()->getMin());
    const double ymax=transformY(getYAxis()->getMax());
    const auto symType=getSymbolType();
    const double symbolSize=parent->pt2px(painter, getSymbolSize());
    const QMarginsF clipMargins=(symType==JKQTPNoSymbol)?QMarginsF(0,0,0,0):QMarginsF(symbolSize,symbolSize,symbolSize,symbolSize);
    const QRectF cliprect=QRectF(qMin(xmin,xmax),qMin(ymin,ymax),fabs(xmax-xmin),fabs(ymax-ymin))+clipMargins;

    int imax=0;
    int imin=0;

    if (getIndexRange(imin, imax)) {


        QPolygonF pl, pf;
        QPolygonF ps;

        double xold=-1;
        double yold=-1;
        double x0=transformX(getBaseline());
        if (getXAxis()->isLogAxis()) {
            if (getBaseline()>0 && getBaseline()>getXAxis()->getMin()) x0=transformX(getBaseline());
            else x0=transformX(getXAxis()->getMin());
        }
        bool first=false;
        intSortData();
        for (int iii=imin; iii<imax; iii++) {
            const int i=qBound(imin, getDataIndex(iii), imax);
            const double xv=datastore->get(static_cast<size_t>(xColumn),static_cast<size_t>(i));
            const double yv=datastore->get(static_cast<size_t>(yColumn),static_cast<size_t>(i));
            //std::cout<<"(xv, yv) =    ( "<<xv<<", "<<yv<<" )\n";
            if (JKQTPIsOKFloat(xv) && JKQTPIsOKFloat(yv)) {
                const double x=transformX(xv);
                const double y=transformY(yv);
                if (JKQTPIsOKFloat(x) && JKQTPIsOKFloat(y)) {
                    ps.append(QPointF(x,y));
                    if (first) {
                        //double xl1=xold;
                        //double yl1=yold;
                        //double xl2=x;
                        //double yl2=y;

                        if (m_specialLineType==JKQTPStepCenter) {
                            double d=(y-yold);
                            pf<<QPointF(xold, yold+d/2.0);
                            pf<<QPointF(x, yold+d/2.0);
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(xold, yold+d/2.0);
                                pl<<QPointF(x, yold+d/2.0);
                                pl<<QPointF(x, y);
                            }
                        } else if (m_specialLineType==JKQTPStepLeft) {
                            pf<<QPointF(x, yold);
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(x, yold);
                                pl<<QPointF(x, y);
                            }
                        } else if (m_specialLineType==JKQTPStepRight) {
                            pf<<QPointF(xold, y);
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(xold, y);
                                pl<<QPointF(x, y);
                            }
                        } else if (m_specialLineType==JKQTPStepAverage) {
                            //                     x/y
                            //              |-------*
                            //              |
                            //              |
                            //              |
                            //     *--------|
                            // xold/yold
                            const double d=(x-xold);
                            //const double h=(y-yold);
                            pf<<QPointF(xold+d/2.0, yold);
                            pf<<QPointF(xold+d/2.0, y);
                            pf<<QPointF(x,y);
                            if (getDrawLine()) {
                                pl<<QPointF(xold+d/2.0, yold);
                                pl<<QPointF(xold+d/2.0, y);
                                pl<<QPointF(x,y);
                            }
                        } else if (m_specialLineType==JKQTPDirectLine) {
                            //                     x/y
                            //                 /----*
                            //           /----/
                            //     *----/
                            // xold/yold
                            pf<<QPointF(x, y);
                            if (getDrawLine()) {
                                pl<<QPointF(x, y);
                            }
                        }

                        //std::cout<<"line ("<<xl1<<", "<<yl1<<") -- ("<<xl2<<", "<<yl2<<")"<<std::endl;
                    } else {
                        if (getDrawLine()) pl<<QPointF(x,y);
                        pf<<QPointF(x0, y);
                        pf<<QPointF(x, y);
                    }
                    xold=x;
                    yold=y;
                    first=true;
                }
            }
        }
        pf<<QPointF(x0, yold);
        painter.save(); auto __finalpaint=JKQTPFinally([&painter]() {painter.restore();});

        if (getFillCurve()) {
            painter.setBrush(b);
            painter.setPen(Qt::NoPen);
            painter.drawPolygon(pf.intersected(cliprect));
        }

        QList<QPolygonF> pl_fordrawing;
        if (isHighlighted() || getDrawLine()) {
            if (getUseNonvisibleLineCompression()) pl_fordrawing=JKQTPClipPolyLine(JKQTPSimplifyPolyLines(pl, getNonvisibleLineCompressionAgressiveness()), cliprect);
            else pl_fordrawing=JKQTPClipPolyLine(pl, cliprect);
        }

        if (isHighlighted()) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(ph);
            for (const QPolygonF& lines : pl_fordrawing) {
                painter.drawPolylineFast(lines);
            }
        }

        if (getDrawLine()) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(p);
            for (const QPolygonF& lines : pl_fordrawing) {
                painter.drawPolylineFast(lines);
            }
        }

        if (m_drawSymbols) {
            painter.save();
            auto __finalpaintsym=JKQTPFinally([&painter]() {painter.restore();});
            for (auto& ppoint: ps) {
                if (cliprect.contains(ppoint)) plotStyledSymbol(parent, painter, ppoint.x(), ppoint.y());
            }
        }
    }

    drawErrorsAfter(painter);
}

int JKQTPSpecialLineVerticalGraph::getKeyColumn() const
{
    return getYColumn();
}

int JKQTPSpecialLineVerticalGraph::getValueColumn() const
{
    return getXColumn();
}

void JKQTPSpecialLineVerticalGraph::setKeyColumn(int __value)
{
    setYColumn(__value);
}

void JKQTPSpecialLineVerticalGraph::setValueColumn(int __value)
{
    setXColumn(__value);
}











