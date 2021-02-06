#ifndef QGSMEASURESELECTEDFEATURESDOCKWIDGET_H
#define QGSMEASURESELECTEDFEATURESDOCKWIDGET_H

#include <QDockWidget>
#include "ui_qgsmeasureselectedfeaturesbase.h"
#include "qgsdistancearea.h"

class QgsMapCanvas;
class QWidget;
class QgsMapLayer;
class QgsVectorLayer;

class QgsMeasureSelectedFeaturesBase : public QDockWidget, private Ui::QgsMeasureSelectedFeaturesBase
{
    Q_OBJECT

public:
    explicit QgsMeasureSelectedFeaturesBase(QgsMapCanvas *canvas, QWidget *parent = nullptr);
    QgsMapLayer *mCurrentLayer = nullptr;
    QgsVectorLayer *mLayer = nullptr;

private slots:
    void onCurrentLayerChanged(QgsMapLayer *layer);
    void onSelectionChanged();

private:
    void refreshSelected();
    void disableLineEdits();
    void getTotalDimensions();
    QgsDistanceArea mDa;

};

#endif // QGSMEASURESELECTEDFEATURESDOCKWIDGET_H
