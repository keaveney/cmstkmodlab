#include <fstream>
#include <cmath>

#include "ApplicationConfig.h"
#include "nqlogger.h"

#include "DefoReconstructionModel.h"
#include "DefoPointSaver.h"

DefoReconstructionModel::DefoReconstructionModel(DefoMeasurementListModel * listModel,
                                                 DefoMeasurementSelectionModel* refSelectionModel,
                                                 DefoMeasurementSelectionModel* defoSelectionModel,
                                                 DefoAlignmentModel* alignmentModel,
                                                 DefoPointIndexerModel* pointIndexerModel,
                                                 DefoColorSelectionModel* refColorModel,
                                                 DefoColorSelectionModel* defoColorModel,
                                                 DefoMeasurementPairListModel* pairListModel,
                                                 DefoMeasurementPairSelectionModel* pairSelectionModel,
						 DefoLensModel* lensModel,
                                                 DefoGeometryModel* geometryModel,
                                                 DefoCalibrationModel* calibrationModel,
                                                 Defo2DSplineInterpolationModel* interpolationModel,
                                                 QObject *parent)
: QObject(parent),
  listModel_(listModel),
  pairListModel_(pairListModel),
  pairSelectionModel_(pairSelectionModel),
  lensModel_(lensModel),
  geometryModel_(geometryModel),
  calibrationModel_(calibrationModel),
  interpolationModel_(interpolationModel)
{
  angle_ = 0.0;
  refMeasurement_ = 0;
  defoMeasurement_ = 0;
  pointIndexer_ = 0;

  connect(refSelectionModel, SIGNAL(selectionChanged(DefoMeasurement*)),
          this, SLOT(refSelectionChanged(DefoMeasurement*)));

  connect(defoSelectionModel, SIGNAL(selectionChanged(DefoMeasurement*)),
          this, SLOT(defoSelectionChanged(DefoMeasurement*)));

  connect(listModel_, SIGNAL(pointsUpdated(const DefoMeasurement*)),
          this, SLOT(pointsUpdated(const DefoMeasurement*)));

  connect(alignmentModel, SIGNAL(alignmentChanged(double)),
          this, SLOT(alignmentChanged(double)));

  connect(pointIndexerModel, SIGNAL(pointIndexerChanged(DefoVPointIndexer*)),
          this, SLOT(pointIndexerChanged(DefoVPointIndexer*)));

  connect(refColorModel, SIGNAL(colorChanged(float,float)),
          this, SLOT(refColorChanged(float,float)));

  connect(defoColorModel, SIGNAL(colorChanged(float,float)),
          this, SLOT(defoColorChanged(float,float)));

  connect(lensModel_, SIGNAL(lensChanged()),
          this, SLOT(lensChanged()));

  connect(geometryModel_, SIGNAL(geometryChanged()),
          this, SLOT(geometryChanged()));

  connect(calibrationModel_, SIGNAL(calibrationChanged()),
          this, SLOT(calibrationChanged()));

  connect(interpolationModel_, SIGNAL(interpolationParametersChanged()),
          this, SLOT(interpolationParametersChanged()));

  reco_ = new DefoRecoSurface(this);
  
  reco_->setPitchX(ApplicationConfig::instance()->getValue<double>("PIXEL_PITCH_X"));
  reco_->setPitchY(ApplicationConfig::instance()->getValue<double>("PIXEL_PITCH_Y"));

  connect(reco_, SIGNAL(incrementRecoProgress()),
          this, SLOT(incrementRecoProgress()));
}

void DefoReconstructionModel::setCurrentDir(QDir& dir)
{
    currentDir_ = dir;
}

void DefoReconstructionModel::refSelectionChanged(DefoMeasurement* measurement)
{
  NQLogMessage("DefoReconstructionModel")
    << "reference selection changed";
  refMeasurement_ = measurement;
  emit setupChanged();
}

void DefoReconstructionModel::defoSelectionChanged(DefoMeasurement* measurement)
{
  NQLogMessage("DefoReconstructionModel")
    << "defo selection changed";
  defoMeasurement_ = measurement;
  emit setupChanged();
}

void DefoReconstructionModel::pointsUpdated(const DefoMeasurement* measurement)
{
  NQLogMessage("DefoReconstructionModel")
    << "points updated";
  if (measurement==refMeasurement_) emit setupChanged();
  if (measurement==defoMeasurement_) emit setupChanged();
}

void DefoReconstructionModel::alignmentChanged(double angle) {
  NQLogMessage("DefoReconstructionModel")
    << "alignment changed";
  angle_ = angle;
  emit setupChanged();
}

void DefoReconstructionModel::pointIndexerChanged(DefoVPointIndexer * indexer) {
  NQLogMessage("DefoReconstructionModel")
    << "point indexer changed";
  pointIndexer_ = indexer;
  emit setupChanged();
}

void DefoReconstructionModel::refColorChanged(float hue, float saturation) {
  NQLogMessage("DefoReconstructionModel")
    << "reference color changed";
  refColor_.setHsvF(hue, saturation, 1.0, 1.0);
  emit setupChanged();
}

void DefoReconstructionModel::defoColorChanged(float hue, float saturation) {
  NQLogMessage("DefoReconstructionModel")
    << "defo color changed";
  defoColor_.setHsvF(hue, saturation, 1.0, 1.0);
  emit setupChanged();
}

void DefoReconstructionModel::lensChanged()
{
  NQLogMessage("DefoReconstructionModel::lensChanged()") << lensModel_->getCurrentName();

  double p0, p1, p2, p3;
  lensModel_->getCurrentParameters(p0, p1, p2, p3);
  reco_->setLensParameters(p0, p1, p2 ,p3);

  NQLogMessage("DefoReconstructionModel::lensChanged()") << "Parameters set to:";
  NQLogMessage("DefoReconstructionModel::lensChanged()") << "p0 = " << p0;
  NQLogMessage("DefoReconstructionModel::lensChanged()") << "p1 = " << p1;
  NQLogMessage("DefoReconstructionModel::lensChanged()") << "p2 = " << p2;
  NQLogMessage("DefoReconstructionModel::lensChanged()") << "p3 = " << p3;
}

void DefoReconstructionModel::geometryChanged()
{
  NQLogMessage("DefoReconstructionModel::geometryChanged()") << "start";

  double angle1 = geometryModel_->getAngle1(); // angle of the dot grid wrt. the horizontal plane [degree]
  double angle1Rad = angle1 * M_PI / 180.;

  double angle2 = geometryModel_->getAngle2(); // angle of the camera frame wrt. the horizontal plane [degree]
  double angle2Rad = angle2 * M_PI / 180.;

  double angle3 = geometryModel_->getAngle3(); // angle of the camera wrt. to the camera arm [degree]
  double angle3Rad = angle3 * M_PI / 180.;

  double distance = geometryModel_->getDistance();
  double height1 = geometryModel_->getHeight1();
  double height2 = geometryModel_->getHeight2();

  NQLog("DefoReconstructionModel", NQLog::Message) << "angle1 [deg]  = " << angle1;
  NQLog("DefoReconstructionModel", NQLog::Message) << "angle1 [rad]  = " << angle1Rad;
  NQLog("DefoReconstructionModel", NQLog::Message) << "angle2 [deg]  = " << angle2;
  NQLog("DefoReconstructionModel", NQLog::Message) << "angle2 [rad]  = " << angle2Rad;
  NQLog("DefoReconstructionModel", NQLog::Message) << "angle3 [deg]  = " << angle3;
  NQLog("DefoReconstructionModel", NQLog::Message) << "angle3 [rad]  = " << angle3Rad;
  NQLog("DefoReconstructionModel", NQLog::Message) << "distance [mm] = " << distance;
  NQLog("DefoReconstructionModel", NQLog::Message) << "height1 [mm]  = " << height1;
  NQLog("DefoReconstructionModel", NQLog::Message) << "height2 [mm]  = " << height2;

  // height of camera rotation point over surface
  double heightCameraToSurface = height1 - height2 - distance * std::sin(angle2Rad);
  NQLog("DefoReconstructionModel", NQLog::Message) << "heightCameraToSurface [mm] = " << heightCameraToSurface;

  // viewing distance from camera to surface
  // (assumption: camera is mounted perpendicular to frame)
  double distanceCamera = heightCameraToSurface / std::cos(angle2Rad + angle3Rad);
  NQLog("DefoReconstructionModel", NQLog::Message) << "distanceCamera [mm] = " << distanceCamera;

  reco_->setNominalCameraDistance(distanceCamera);

  // distance from grid to surface calculated as the shortest distance
  // between grid and the point on the surface under the grid rotaion axis
  double distanceGrid = (height1 - height2) * std::cos(angle1Rad);
  NQLog("DefoReconstructionModel", NQLog::Message) << "distanceGrid [mm] = " << distanceGrid;

  reco_->setNominalGridDistance(distanceGrid);

  // not really sure about this one...
  reco_->setNominalViewingAngle(angle2Rad + angle3Rad);
  NQLog("DefoReconstructionModel", NQLog::Message) << "viewing angle [rad] = " << angle2Rad + angle3Rad;

  reco_->calculateHelpers();

  reco_->setAngle1(angle1);
  reco_->setAngle2(angle2);
  reco_->setAngle3(angle3);
  reco_->setDistance(distance);
  reco_->setHeight1(height1);
  reco_->setHeight2(height2);

  NQLog("DefoReconstructionModel::geometryChanged()", NQLog::Message) << "end";

  emit setupChanged();
}

void DefoReconstructionModel::calibrationChanged()
{
  NQLog("DefoReconstructionModel::calibrationChanged()", NQLog::Message) << "start";

  reco_->setCalibX(calibrationModel_->getCalibX());
  reco_->setCalibY(calibrationModel_->getCalibY());
  reco_->setCalibZx(calibrationModel_->getCalibZx());
  reco_->setCalibZy(calibrationModel_->getCalibZy());
  NQLog("DefoReconstructionModel", NQLog::Message) << "calibX  = " << calibrationModel_->getCalibX();
  NQLog("DefoReconstructionModel", NQLog::Message) << "calibY  = " << calibrationModel_->getCalibY();
  NQLog("DefoReconstructionModel", NQLog::Message) << "calibZx = " << calibrationModel_->getCalibZx();
  NQLog("DefoReconstructionModel", NQLog::Message) << "calibZy = " << calibrationModel_->getCalibZy();

  NQLog("DefoReconstructionModel::calibrationChanged()", NQLog::Message) << "end";
}

void DefoReconstructionModel::interpolationParametersChanged()
{
  NQLog("DefoReconstructionModel::interpolationParametersChanged()", NQLog::Message) << "start";

  NQLog("DefoReconstructionModel", NQLog::Message) << "kX = " << interpolationModel_->getKX();
  NQLog("DefoReconstructionModel", NQLog::Message) << "kY = " << interpolationModel_->getKY();
  NQLog("DefoReconstructionModel", NQLog::Message) << "s  = " << interpolationModel_->getSmoothing();

  NQLog("DefoReconstructionModel::interpolationParametersChanged()", NQLog::Message) << "end";
}

void DefoReconstructionModel::incrementRecoProgress() {
  emit incrementProgress();
}

void DefoReconstructionModel::reconstruct()
{
  NQLogMessage("DefoReconstructionModel") << "reconstruct";

  emit recoProgressChanged(0);

  if (refMeasurement_==0 || defoMeasurement_==0) {
    NQLogWarning("DefoOfflinePreparationModel") 
      << "reco: reference and deformed measurements not selected";
    return;
  }

  if (pointIndexer_==0) {
    NQLogWarning("DefoOfflinePreparationModel") 
      << "reco: point indexer not available";
    return;
  }

  const DefoPointCollection* refPoints = listModel_->getMeasurementPoints(refMeasurement_);
  if (!refPoints || refPoints->size()==0) {
     NQLogWarning("DefoOfflinePreparationModel") 
       << "reco: reference measurement does not contain points";
    return;
  }

  const DefoPointCollection* defoPoints = listModel_->getMeasurementPoints(defoMeasurement_);
  if (!defoPoints || defoPoints->size()==0) {
    NQLogWarning("DefoOfflinePreparationModel") 
      << "reco: deformed measurement does not contain points";
    return;
  }

  reco_->setImageSize(std::pair<double,double>(refMeasurement_->getWidth(),
                                               refMeasurement_->getHeight()));

  if (!alignPoints(refPoints, refCollection_)) {
    std::cout << "reco: reference points could not be aligned" << std::endl;
    return;
  }
  emit incrementProgress();

  if (!alignPoints(defoPoints, defoCollection_)) {
    NQLogWarning("DefoOfflinePreparationModel") 
      << "reco: reference points could not be aligned";
    return;
  }
  emit incrementProgress();

  QString basename = "offlinePoints_%1.xml";
  QString fileLocation;

  pointIndexer_->indexPoints(&refCollection_, refColor_);
  emit incrementProgress();

  fileLocation = currentDir_.absoluteFilePath(basename.arg(refMeasurement_->getTimeStamp().toString("yyyyMMddhhmmss")));
  DefoPointSaver refSaver(fileLocation);
  refSaver.writeXMLPoints(refCollection_);
  emit incrementProgress();

  pointIndexer_->indexPoints(&defoCollection_, defoColor_);
  emit incrementProgress();

  fileLocation = currentDir_.absoluteFilePath(basename.arg(defoMeasurement_->getTimeStamp().toString("yyyyMMddhhmmss")));
  DefoPointSaver defoSaver(fileLocation);
  defoSaver.writeXMLPoints(defoCollection_);

  reco_->setFocalLength(refMeasurement_->getFocalLength());
  NQLog("DefoReconstructionModel::reconstruct()", NQLog::Message) << "focalLength [mm]   = " << refMeasurement_->getFocalLength();

  emit incrementProgress();

  // reco_->dump();
  DefoSurface surface = reco_->reconstruct(defoCollection_, refCollection_);

  emit incrementProgress();

  surface.fitSpline2D(interpolationModel_->getKX(),
                      interpolationModel_->getKY(),
                      interpolationModel_->getSmoothing(),
                      interpolationModel_->getNXY());

  emit incrementProgress();

  QString filename = "defoReco_";
  filename += refMeasurement_->getTimeStamp().toString("yyyyMMddhhmmss");
  filename += "_";
  filename += defoMeasurement_->getTimeStamp().toString("yyyyMMddhhmmss");
  filename += ".txt";
  surface.dumpSplineField(currentDir_.absoluteFilePath(filename).toStdString());

  emit incrementProgress();

  filename = "defoRecoSpline2D_";
  filename += refMeasurement_->getTimeStamp().toString("yyyyMMddhhmmss");
  filename += "_";
  filename += defoMeasurement_->getTimeStamp().toString("yyyyMMddhhmmss");
  filename += ".txt";
  surface.dumpSpline2DField(currentDir_.absoluteFilePath(filename).toStdString(),
                            interpolationModel_->getDX(),
                            interpolationModel_->getDY());

  emit incrementProgress();

  bool newPair = false;
  DefoMeasurementPair * measurementPair = pairListModel_->findMeasurementPair(refMeasurement_,
                                                                              defoMeasurement_);
  if (measurementPair==0) {
    newPair = true;
    measurementPair = new DefoMeasurementPair(refMeasurement_, defoMeasurement_);
  }

  measurementPair->setSurface(surface);

  if (newPair) {
    pairListModel_->addMeasurementPair(measurementPair);
  }
}

bool DefoReconstructionModel::alignPoints(const DefoPointCollection* original,
                                          DefoPointCollection& aligned)
{
  aligned.clear();

  if (angle_==0.0) {
    for (DefoPointCollection::const_iterator it = original->begin();
         it!=original->end();
         ++it) {
      aligned.push_back(*it);
    }
    return true;
  } else {

    double c = std::cos(angle_);
    double s = std::sin(angle_);

    for (DefoPointCollection::const_iterator it = original->begin();
         it!=original->end();
         ++it) {
      const DefoPoint& p = *it;
      DefoPoint np(p);
      np.setX(p.getX()*c - p.getY()*s);
      np.setY(p.getX()*s + p.getY()*c);

      aligned.push_back(np);
    }
  }

  return true;
}
