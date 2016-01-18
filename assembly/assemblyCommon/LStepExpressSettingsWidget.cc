#include <nqlogger.h>

#include "ApplicationConfig.h"

#include "LStepExpressSettingsWidget.h"

LStepExpressSettingsCheckBox::LStepExpressSettingsCheckBox(LStepExpressSettings* settings,
                                                           const QString& key,
                                                           QWidget * parent)
    : QCheckBox(parent),
      settings_(settings),
      key_(key)
{
    connect(this, SIGNAL(stateChanged(int)),
            this, SLOT(statusChanged(int)));

    connect(this, SIGNAL(valueChanged(QString,bool)),
            settings_, SLOT(valueChanged(QString,bool)));
}

void LStepExpressSettingsCheckBox::statusChanged(int /* state */)
{
    emit valueChanged(key_, isChecked());
}

LStepExpressSettingsTripleCheckBox::LStepExpressSettingsTripleCheckBox(LStepExpressSettings* settings,
                                                                       const QString& key,
                                                                       QWidget * parent)
    : QWidget(parent),
      settings_(settings),
      key_(key)
{
    QHBoxLayout * layout = new QHBoxLayout(this);
    setLayout(layout);

    box_[0] = new QCheckBox(this);
    layout->addWidget(box_[0]);
    box_[1] = new QCheckBox(this);
    layout->addWidget(box_[1]);
    box_[2] = new QCheckBox(this);
    layout->addWidget(box_[2]);

    connect(box_[0], SIGNAL(stateChanged(int)),
            this, SLOT(statusChanged(int)));
    connect(box_[1], SIGNAL(stateChanged(int)),
            this, SLOT(statusChanged(int)));
    connect(box_[2], SIGNAL(stateChanged(int)),
            this, SLOT(statusChanged(int)));

    connect(this, SIGNAL(valueChanged(QString,bool,bool,bool)),
            settings_, SLOT(valueChanged(QString,bool,bool,bool)));
}

void LStepExpressSettingsTripleCheckBox::statusChanged(int /* state */)
{
    emit valueChanged(key_, box_[0]->isChecked(), box_[1]->isChecked(), box_[2]->isChecked());
}

LStepExpressSettingsIntSpinBox::LStepExpressSettingsIntSpinBox(LStepExpressSettings* settings,
                                                               const QString& key,
                                                               int min, int max,
                                                               QWidget * parent)
    : QSpinBox(parent),
      settings_(settings),
      key_(key)
{
    setMinimum(min);
    setMaximum(max);
    setSingleStep(min);

    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(handleValueChanged(int)));

    connect(this, SIGNAL(valueChanged(QString,int)),
            settings_, SLOT(valueChanged(QString,int)));
}

void LStepExpressSettingsIntSpinBox::wheelEvent(QWheelEvent * event)
{
    event->accept();
}

void LStepExpressSettingsIntSpinBox::handleValueChanged(int value)
{
    emit valueChanged(key_, value);
}

LStepExpressSettingsDoubleSpinBox::LStepExpressSettingsDoubleSpinBox(LStepExpressSettings* settings,
                                                                     const QString& key,
                                                                     double min, double max,
                                                                     QWidget * parent)
    : QDoubleSpinBox(parent),
      settings_(settings),
      key_(key)
{
    setMinimum(min);
    setMaximum(max);
    setSingleStep(min);
    setDecimals(4);

    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(handleValueChanged(double)));

    connect(this, SIGNAL(valueChanged(QString,double)),
            settings_, SLOT(valueChanged(QString,double)));
}

void LStepExpressSettingsDoubleSpinBox::wheelEvent(QWheelEvent * event)
{
    event->accept();
}

void LStepExpressSettingsDoubleSpinBox::handleValueChanged(double value)
{
    emit valueChanged(key_, value);
}

LStepExpressSettingsWidget::LStepExpressSettingsWidget(LStepExpressSettings* settings,
                                                       QWidget *parent)
    : QWidget(parent),
      settings_(settings)
{
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    setLayout(vlayout);

    mainToolBox_ = new QToolBox(this);
    vlayout->addWidget(mainToolBox_);

    axisToolBox_ = new QWidget(mainToolBox_);
    fillAxisToolBox();
    mainToolBox_->addItem(axisToolBox_, "Axis");

    motorToolBox_ = new QToolBox(mainToolBox_);
    mainToolBox_->addItem(motorToolBox_, "Motor");

    encoderToolBox_ = new QToolBox(mainToolBox_);
    mainToolBox_->addItem(encoderToolBox_, "Encoder");

    QWidget * buttons = new QWidget(this);
    QHBoxLayout* hlayout = new QHBoxLayout(this);
    buttons->setLayout(hlayout);

    readFromDeviceButton_ = new QPushButton("Read From Device", buttons);
    connect(readFromDeviceButton_, SIGNAL(clicked(bool)),
            this, SLOT(readFromDeviceClicked()));
    hlayout->addWidget(readFromDeviceButton_);

    readFromFileButton_ = new QPushButton("Read From File", buttons);
    connect(readFromFileButton_, SIGNAL(clicked(bool)),
            this, SLOT(readFromFileClicked()));
    hlayout->addWidget(readFromFileButton_);

    writeToDeviceButton_ = new QPushButton("Write To Device", buttons);
    connect(writeToDeviceButton_, SIGNAL(clicked(bool)),
            this, SLOT(writeToDeviceClicked()));
    hlayout->addWidget(writeToDeviceButton_);

    writeToFileButton_ = new QPushButton("Write To File", buttons);
    connect(writeToFileButton_, SIGNAL(clicked(bool)),
            this, SLOT(writeToFileClicked()));
    hlayout->addWidget(writeToFileButton_);

    vlayout->addWidget(buttons);
 }

void LStepExpressSettingsWidget::fillAxisToolBox()
{
    QGridLayout *layout = new QGridLayout(axisToolBox_);
    axisToolBox_->setLayout(layout);

    layout->addWidget(new QLabel("X", axisToolBox_), 0, 1);
    layout->addWidget(new QLabel("Y", axisToolBox_), 0, 2);
    layout->addWidget(new QLabel("Z", axisToolBox_), 0, 3);
    layout->addWidget(new QLabel("A", axisToolBox_), 0, 4);

    layout->addWidget(new QLabel("Direction", axisToolBox_), 1, 0);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "X-AxisDirection", axisToolBox_), 1, 1);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "Y-AxisDirection", axisToolBox_), 1, 2);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "Z-AxisDirection", axisToolBox_), 1, 3);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "A-AxisDirection", axisToolBox_), 1, 4);

    layout->addWidget(new QLabel("Gear Denominator", axisToolBox_), 2, 0);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "X-GearDenominator", 1, 1000000, axisToolBox_), 2, 1);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Y-GearDenominator", 1, 1000000, axisToolBox_), 2, 2);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Z-GearDenominator", 1, 1000000, axisToolBox_), 2, 3);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "A-GearDenominator", 1, 1000000, axisToolBox_), 2, 4);

    layout->addWidget(new QLabel("Gear Numerator", axisToolBox_), 3, 0);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "X-GearNumerator", 1, 1000000, axisToolBox_), 3, 1);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Y-GearNumerator", 1, 1000000, axisToolBox_), 3, 2);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Z-GearNumerator", 1, 1000000, axisToolBox_), 3, 3);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "A-GearNumerator", 1, 1000000, axisToolBox_), 3, 4);

    layout->addWidget(new QLabel("Spindle Pitch", axisToolBox_), 4, 0);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "X-SpindlePitch", 0.0001, 68.0, axisToolBox_), 4, 1);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Y-SpindlePitch", 0.0001, 68.0, axisToolBox_), 4, 2);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Z-SpindlePitch", 0.0001, 68.0, axisToolBox_), 4, 3);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "A-SpindlePitch", 0.0001, 68.0, axisToolBox_), 4, 4);

    layout->addWidget(new QLabel("Swap Limit Switch", axisToolBox_), 5, 0);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "X-SwapLimitSwitch", axisToolBox_), 5, 1);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "Y-SwapLimitSwitch", axisToolBox_), 5, 2);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "Z-SwapLimitSwitch", axisToolBox_), 5, 3);
    layout->addWidget(new LStepExpressSettingsCheckBox(settings_, "A-SwapLimitSwitch", axisToolBox_), 5, 4);

    layout->addWidget(new QLabel("Limit Switch Polarity", axisToolBox_), 6, 0);
    layout->addWidget(new LStepExpressSettingsTripleCheckBox(settings_, "X-LimitSwitchPolarity", axisToolBox_), 6, 1);
    layout->addWidget(new LStepExpressSettingsTripleCheckBox(settings_, "Y-LimitSwitchPolarity", axisToolBox_), 6, 2);
    layout->addWidget(new LStepExpressSettingsTripleCheckBox(settings_, "Z-LimitSwitchPolarity", axisToolBox_), 6, 3);
    layout->addWidget(new LStepExpressSettingsTripleCheckBox(settings_, "A-LimitSwitchPolarity", axisToolBox_), 6, 4);

    layout->addWidget(new QLabel("Acceleration", axisToolBox_), 7, 0);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "X-Acceleration", 0.01, 20.0, axisToolBox_), 7, 1);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Y-Acceleration", 0.01, 20.0, axisToolBox_), 7, 2);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Z-Acceleration", 0.01, 20.0, axisToolBox_), 7, 3);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "A-Acceleration", 0.01, 20.0, axisToolBox_), 7, 4);

    layout->addWidget(new QLabel("Acceleration Jerk", axisToolBox_), 8, 0);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "X-AccelerationJerk", 1, 1000000, axisToolBox_), 8, 1);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Y-AccelerationJerk", 1, 1000000, axisToolBox_), 8, 2);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Z-AccelerationJerk", 1, 1000000, axisToolBox_), 8, 3);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "A-AccelerationJerk", 1, 1000000, axisToolBox_), 8, 4);

    layout->addWidget(new QLabel("Deceleration", axisToolBox_), 9, 0);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "X-Deceleration", 0.01, 20.0, axisToolBox_), 9, 1);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Y-Deceleration", 0.01, 20.0, axisToolBox_), 9, 2);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Z-Deceleration", 0.01, 20.0, axisToolBox_), 9, 3);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "A-Deceleration", 0.01, 20.0, axisToolBox_), 9, 4);

    layout->addWidget(new QLabel("Deceleration Jerk", axisToolBox_), 10, 0);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "X-DecelerationJerk", 1, 1000000, axisToolBox_), 10, 1);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Y-DecelerationJerk", 1, 1000000, axisToolBox_), 10, 2);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "Z-DecelerationJerk", 1, 1000000, axisToolBox_), 10, 3);
    layout->addWidget(new LStepExpressSettingsIntSpinBox(settings_, "A-DecelerationJerk", 1, 1000000, axisToolBox_), 10, 4);

    layout->addWidget(new QLabel("Velocity", axisToolBox_), 11, 0);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "X-Velocity", 0.0, 100.0, axisToolBox_), 11, 1);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Y-Velocity", 0.0, 100.0, axisToolBox_), 11, 2);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "Z-Velocity", 0.0, 100.0, axisToolBox_), 11, 3);
    layout->addWidget(new LStepExpressSettingsDoubleSpinBox(settings_, "A-Velocity", 0.0, 100.0, axisToolBox_), 11, 4);
}

void LStepExpressSettingsWidget::readFromDeviceClicked()
{
    NQLog("LStepExpressSettingsWidget") << "readFromDeviceClicked()";

    settings_->readSettingsFromDevice();
}

void LStepExpressSettingsWidget::readFromFileClicked()
{
    NQLog("LStepExpressSettingsWidget") << "readFromFileClicked()";

}

void LStepExpressSettingsWidget::writeToDeviceClicked()
{
    NQLog("LStepExpressSettingsWidget") << "writeToDeviceClicked()";

    settings_->writeSettingsToDevice();
}

void LStepExpressSettingsWidget::writeToFileClicked()
{
    NQLog("LStepExpressSettingsWidget") << "writeToFileClicked()";

}