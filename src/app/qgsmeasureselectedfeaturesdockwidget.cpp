
#include "qgsmeasureselectedfeaturesdockwidget.h"
#include "qgisapp.h"
#include "qgsproject.h"
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"
#include "qgsfeatureiterator.h"


QgsMeasureSelectedFeaturesBase::QgsMeasureSelectedFeaturesBase(QgsMapCanvas *canvas, QWidget *parent)
    : QDockWidget(parent)
{
    setupUi(this);

    mCurrentLayer = QgisApp::instance()->activeLayer();

    connect( canvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMeasureSelectedFeaturesBase::onCurrentLayerChanged );

    //mTotalLabel->setWordWrap( true );

    if(mCurrentLayer->type() == QgsMapLayerType::VectorLayer && mCurrentLayer->isSpatial())
    {
        mLayer = static_cast<QgsVectorLayer *>( mCurrentLayer );
        if(mLayer->geometryType() == QgsWkbTypes::LineGeometry || mLayer->geometryType() == QgsWkbTypes::PolygonGeometry)
        {
            connect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsMeasureSelectedFeaturesBase::onSelectionChanged );
            this->setWindowTitle( tr( "Measure selected features: ")+mLayer->name() );
            refreshSelected();
        }
        else
        {
            disableLineEdits();
            this->setWindowTitle( tr( "Point layer selected" ) );
        }
    }
    else
    {
        disableLineEdits();
        this->setWindowTitle( tr( "Raster or non-spatial vector layer selected" ) );
    }    
}

void QgsMeasureSelectedFeaturesBase::onCurrentLayerChanged(QgsMapLayer *layer)
{
  mTotalLineEdit->setEnabled( true );
  mAreaHectaresLineEdit->setEnabled( true );
  mKilometersLineEdit->setEnabled( true );
  if ( mLayer )
  {
      if(mLayer->geometryType() == QgsWkbTypes::LineGeometry || mLayer->geometryType() == QgsWkbTypes::PolygonGeometry)
      {
        disconnect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsMeasureSelectedFeaturesBase::onSelectionChanged );
      }
  }

  if(layer->type() == QgsMapLayerType::VectorLayer && layer->isSpatial())
  {
      mLayer = static_cast<QgsVectorLayer *>( layer );
      if (mLayer->geometryType() == QgsWkbTypes::LineGeometry || mLayer->geometryType() == QgsWkbTypes::PolygonGeometry)
      {
          connect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsMeasureSelectedFeaturesBase::onSelectionChanged );
          this->setWindowTitle( tr( "Measure selected features: ")+mLayer->name() );
          refreshSelected();
      }
      else
      {
          disableLineEdits();
          this->setWindowTitle( tr( "Point layer selected" ) );
      }
  }
  else
  {
      disableLineEdits();
      this->setWindowTitle( tr( "Raster or non-spatial vector layer selected" ) );
  }
}

void QgsMeasureSelectedFeaturesBase::refreshSelected()
{
    mTotalLineEdit->clear();
    mAreaHectaresLineEdit->clear();
    mKilometersLineEdit->clear();
    //Don't try to measure dimensionless point features!
    if (mLayer->geometryType() == QgsWkbTypes::LineGeometry || mLayer->geometryType() == QgsWkbTypes::PolygonGeometry)
    {
        getTotalDimensions();
    }
}

void QgsMeasureSelectedFeaturesBase::onSelectionChanged()
{
    refreshSelected();
}

void QgsMeasureSelectedFeaturesBase::disableLineEdits()
{
    mTotalLabel->setText( tr("Total") );
    mTotalLineEdit->clear();
    mTotalLineEdit->setEnabled( false );
    mAreaHectaresLineEdit->clear();
    mAreaHectaresLineEdit->setEnabled( false );
    mKilometersLineEdit->clear();
    mKilometersLineEdit->setEnabled(false );
}

void QgsMeasureSelectedFeaturesBase::getTotalDimensions()
{
    QgsFeatureIterator selected = mLayer->getSelectedFeatures();

    QgsFeature feat;

    QString totalAsString;

    double convertedKm;
    QString convertedKmAsString;

    double convertedHectares;
    QString convertedHectaresAsString;

    mDa.setEllipsoid( mLayer->crs().ellipsoidAcronym() );

    if(mLayer->geometryType() == QgsWkbTypes::LineGeometry)
    {
        //We have a line vector layer

        mAreaHectaresLineEdit->setPlaceholderText("Hectares (polygons only)");
        mAreaHectaresLineEdit->setEnabled(false);

        if(mLayer->crs().isGeographic())
        {
            //use distance area ellipsoidal measurement            
            double totalLengthEllipsoidal = 0.0;            
            while ( selected.nextFeature( feat ) )
            {
                totalLengthEllipsoidal += mDa.measureLength(feat.geometry());
            }

            mTotalLabel->setText( tr("Total length (ellipsoidal)") );
            totalAsString = QString::number( totalLengthEllipsoidal, 'f', 3 );
            mTotalLineEdit->setText(totalAsString+" "+QgsUnitTypes::encodeUnit(mDa.lengthUnits()));
            convertedKm = mDa.convertLengthMeasurement(totalLengthEllipsoidal, QgsUnitTypes::DistanceKilometers);
            convertedKmAsString = QString::number( convertedKm, 'f', 3 );
            mKilometersLineEdit->setText(convertedKmAsString+" km");
        }
        else
        {
            //use simple planar length measurement
            double totalLengthPlanar = 0.0;
            while ( selected.nextFeature( feat ) )
            {
                totalLengthPlanar += feat.geometry().length();
            }
            mTotalLabel->setText( tr("Total length (planar)") );
            totalAsString = QString::number( totalLengthPlanar, 'f', 3 );
            mTotalLineEdit->setText(totalAsString+" "+QgsUnitTypes::encodeUnit(mLayer->crs().mapUnits()));
            convertedKm = mDa.convertLengthMeasurement(totalLengthPlanar, QgsUnitTypes::DistanceKilometers);
            convertedKmAsString = QString::number( convertedKm, 'f', 3 );
            mKilometersLineEdit->setText(convertedKmAsString+" km");
        }
    }
    else if(mLayer->geometryType() == QgsWkbTypes::PolygonGeometry)
    {
        //We have a polygon vector layer
        if(mLayer->crs().isGeographic())
        {
            //use distance area ellipsoidal measurement
            double totalAreaEllipsoidal = 0.0;
            while ( selected.nextFeature( feat ) )
            {
                totalAreaEllipsoidal += mDa.measureArea(feat.geometry());
            }
            mTotalLabel->setText( tr("Total area (ellipsoidal)") );
            totalAsString = QString::number( totalAreaEllipsoidal, 'f', 3 );
            mTotalLineEdit->setText(totalAsString+" "+QgsUnitTypes::encodeUnit(mDa.areaUnits()));

            convertedKm = mDa.convertAreaMeasurement(totalAreaEllipsoidal, QgsUnitTypes::AreaSquareKilometers);
            convertedKmAsString = QString::number( convertedKm, 'f', 3 );
            mKilometersLineEdit->setText( convertedKmAsString+" km2" );

            convertedHectares = mDa.convertAreaMeasurement(totalAreaEllipsoidal, QgsUnitTypes::AreaHectares);
            convertedHectaresAsString = QString::number( convertedHectares, 'f', 3 );
            mAreaHectaresLineEdit->setText(convertedHectaresAsString+" hectares");
        }
        else
        {
            //use simple planar area measurement
            double totalAreaPlanar = 0.0;
            while ( selected.nextFeature( feat ) )
            {
                totalAreaPlanar += feat.geometry().area();
            }
            mTotalLabel->setText( tr("Total area (planar)") );
            totalAsString = QString::number( totalAreaPlanar, 'f', 3 );
            mTotalLineEdit->setText(totalAsString+" square "+QgsUnitTypes::encodeUnit(mLayer->crs().mapUnits()));

            convertedKm = mDa.convertAreaMeasurement(totalAreaPlanar, QgsUnitTypes::AreaSquareKilometers);
            convertedKmAsString = QString::number( convertedKm, 'f', 3 );
            mKilometersLineEdit->setText( convertedKmAsString+" km2" );

            convertedHectares = mDa.convertAreaMeasurement(totalAreaPlanar, QgsUnitTypes::AreaHectares);
            convertedHectaresAsString = QString::number( convertedHectares, 'f', 3 );
            mAreaHectaresLineEdit->setText(convertedHectaresAsString+" hectares");
        }
    }
}
