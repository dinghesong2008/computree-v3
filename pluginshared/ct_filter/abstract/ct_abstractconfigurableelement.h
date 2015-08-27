#ifndef CT_ABSTRACTCONFIGURABLEELEMENT_H
#define CT_ABSTRACTCONFIGURABLEELEMENT_H

#include "pluginShared_global.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include <QDebug>

class PLUGINSHAREDSHARED_EXPORT CT_AbstractConfigurableElement : public QObject
{
    Q_OBJECT
public:
    CT_AbstractConfigurableElement();
    ~CT_AbstractConfigurableElement();

    virtual QString getName() {return "";}

    virtual QString getCompleteName() {return getName();}

    virtual QString getShortDescription() const = 0;
    virtual QString getDetailledDescription() const = 0;

    virtual QString getParametersAsString() const = 0;
    virtual bool setParametersFromString(QString parameters) = 0;

    virtual CT_AbstractConfigurableElement* copy() const = 0;

    virtual bool configure();

    virtual void postConfigure() = 0;

protected:
    CT_StepConfigurableDialog*  _configDialog;

    CT_StepConfigurableDialog* addConfigurationDialog();

    virtual void createConfigurationDialog() {}
    virtual void updateParamtersAfterConfiguration() {}

};

#endif // CT_ABSTRACTCONFIGURABLEELEMENT_H