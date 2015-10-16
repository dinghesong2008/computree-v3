/****************************************************************************

 Copyright (C) 2010-2012 the Office National des Forêts (ONF), France
                     and the Association de Recherche Technologie et Sciences (ARTS), Ecole Nationale Supérieure d'Arts et Métiers (ENSAM), Cluny, France.
                     All rights reserved.

 Contact : alexandre.piboule@onf.fr

 Developers : Michaël KREBS (ARTS/ENSAM)

 This file is part of PluginShared library 2.0.

 PluginShared is free library: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PluginShared is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with PluginShared.  If not, see <http://www.gnu.org/licenses/lgpl.html>.

*****************************************************************************/



#include "ct_abstractstepplugin.h"

#include "ct_global/ct_context.h"

#include "ct_stepseparator.h"
#include "ct_steploadfileseparator.h"
#include "ct_stepcanbeaddedfirstseparator.h"
#include "ct_actions/ct_actionsseparator.h"
#include "ct_exporter/ct_standardexporterseparator.h"
#include "ct_reader/ct_standardreaderseparator.h"

#include "ct_step/abstract/ct_abstractsteploadfile.h"
#include "ct_step/abstract/ct_abstractstepcanbeaddedfirst.h"
#include "ct_step/ct_stepinitializedata.h"

#include "ct_turn/inTurn/tools/ct_inturnmanager.h"
#include "ct_result/tools/ct_outmanager.h"

#include "ct_step/tools/menu/ct_menulevel.h"

#include <QSettings>

CT_AbstractStepPlugin::CT_AbstractStepPlugin()
{
    _pluginSettings = NULL;
    m_coreInterface = NULL;
}

CT_AbstractStepPlugin::~CT_AbstractStepPlugin()
{
    clearMemory();

    delete _pluginSettings;
}

bool CT_AbstractStepPlugin::init()
{
    if(_pluginSettings == NULL)
        _pluginSettings = initQSettings();

    clearMemory();

    bool ok = loadOpenFileStep() && loadGenericsStep() && loadCanBeAddedFirstStep() && loadActions() && loadExporters() && loadReaders() && loadFilters() && loadMetrics();

    return ok;
}

bool CT_AbstractStepPlugin::initAfterAllPluginsLoaded()
{
    return loadAfterAllPluginsLoaded();
}

void CT_AbstractStepPlugin::finishInitialization()
{
    QList<CT_MenuLevel*> subLevels = menuOfSteps()->levels();
    QListIterator<CT_MenuLevel*> it(subLevels);

    while(it.hasNext()) {
        initAllStepOfThisPluginInLevelsRecursively(it.next());
    }
}

void CT_AbstractStepPlugin::unload()
{
    aboutToBeUnloaded();
}

QList<CT_StepSeparator*> CT_AbstractStepPlugin::getGenericsStepAvailable() const
{
    return _stepAvailable;
}

QList<CT_StepLoadFileSeparator*> CT_AbstractStepPlugin::getOpenFileStepAvailable() const
{
    return _stepOpenFileAvailable;
}

QList<CT_StepCanBeAddedFirstSeparator*> CT_AbstractStepPlugin::getCanBeAddedFirstStepAvailable() const
{
    return _stepCanBeAddedFirstAvailable;
}

QList<CT_ActionsSeparator*> CT_AbstractStepPlugin::getActionsAvailable() const
{
    return m_actions;
}

QList<CT_StandardExporterSeparator*> CT_AbstractStepPlugin::getExportersAvailable() const
{
    return m_exporters;
}

QList<CT_StandardReaderSeparator*> CT_AbstractStepPlugin::getReadersAvailable() const
{
    return m_readers;
}

QList<CT_AbstractFilter *> CT_AbstractStepPlugin::getFiltersAvailable() const
{
    return m_filters;
}

QList<CT_AbstractMetric *> CT_AbstractStepPlugin::getMetricsAvailable() const
{
    return m_metrics;
}

QList<CT_AbstractStepLoadFile*> CT_AbstractStepPlugin::getOpenFileStep(const QString &filePath) const
{
    QList<CT_AbstractStepLoadFile*> l;
    QList<CT_AbstractStepLoadFile*> lTmp;

    QListIterator<CT_StepLoadFileSeparator*> it(_stepOpenFileAvailable);

    while(it.hasNext())
    {
        QList<CT_AbstractStepLoadFile *> list = it.next()->getStepList();
        QListIterator<CT_AbstractStepLoadFile*> itStep(list);

        while(itStep.hasNext())
        {
            CT_AbstractStepLoadFile *step = itStep.next();

            bool acceptAll;
            bool acceptfile = step->acceptFile(filePath, &acceptAll);
            if(acceptfile)
            {
                if (acceptAll)
                {
                    lTmp.append(step);
                } else {
                    l.append(step);
                }
            }
        }
    }

    l.append(lTmp);

    return l;
}

QString CT_AbstractStepPlugin::getKeyForStep(const CT_VirtualAbstractStep &step) const
{
    return step.getStepName();
}

CT_VirtualAbstractStep* CT_AbstractStepPlugin::getStepFromKey(QString key) const
{
    /*QListIterator<CT_StepLoadFileSeparator*> it(_stepOpenFileAvailable);

    while(it.hasNext())
    {
        QList<CT_AbstractStepLoadFile *> list = it.next()->getStepList();
        QListIterator<CT_AbstractStepLoadFile*> itStep(list);

        while(itStep.hasNext())
        {
            CT_AbstractStepLoadFile *step = itStep.next();

            if(step->getStepName() == key)
                return step;
        }
    }

    QListIterator<CT_StepSeparator*> itG(_stepAvailable);

    while(itG.hasNext())
    {
        QList<CT_VirtualAbstractStep*> list = itG.next()->getStepList();
        QListIterator<CT_VirtualAbstractStep*> itStep(list);

        while(itStep.hasNext())
        {
            CT_VirtualAbstractStep *step = itStep.next();

            if(step->getStepName() == key)
                return step;
        }
    }

    QListIterator<CT_StepCanBeAddedFirstSeparator*> itF(_stepCanBeAddedFirstAvailable);

    while(itF.hasNext())
    {
        QList<CT_AbstractStepCanBeAddedFirst*> list = itF.next()->getStepList();
        QListIterator<CT_AbstractStepCanBeAddedFirst*> itStep(list);

        while(itStep.hasNext())
        {
            CT_VirtualAbstractStep *step = itStep.next();

            if(step->getStepName() == key)
                return step;
        }
    }*/

    return searchStepFromKeyOfThisPluginInMenuRecursively(menuOfSteps(), key);
}

CT_VirtualAbstractStep* CT_AbstractStepPlugin::createNewInstanceOfStep(const CT_VirtualAbstractStep &step, CT_VirtualAbstractStep *parent) const
{
    CT_VirtualAbstractStep *myStep = (CT_VirtualAbstractStep*)dynamic_cast<const CT_VirtualAbstractStep*>(&step);

    if(myStep != NULL)
    {
        CT_StepInitializeData *data = createNewStepInitializeData(dynamic_cast<CT_VirtualAbstractStep*>(parent));

        CT_VirtualAbstractStep *newStep = myStep->createNewInstance(*data);

        newStep->init();

        return newStep;
    }

    return NULL;
}

bool CT_AbstractStepPlugin::loadFilters()
{
    return true;
}

bool CT_AbstractStepPlugin::loadMetrics()
{
    return true;
}

void CT_AbstractStepPlugin::clearGenericsStep()
{
    qDeleteAll(_stepAvailable.begin(), _stepAvailable.end());
    _stepAvailable.clear();
}

void CT_AbstractStepPlugin::clearOpenFileStep()
{
    qDeleteAll(_stepOpenFileAvailable.begin(), _stepOpenFileAvailable.end());
    _stepOpenFileAvailable.clear();
}

void CT_AbstractStepPlugin::clearCanBeAddedFirstStep()
{
    qDeleteAll(_stepCanBeAddedFirstAvailable.begin(), _stepCanBeAddedFirstAvailable.end());
    _stepCanBeAddedFirstAvailable.clear();
}

void CT_AbstractStepPlugin::clearActions()
{
    qDeleteAll(m_actions.begin(), m_actions.end());
    m_actions.clear();
}

void CT_AbstractStepPlugin::clearExporters()
{
    qDeleteAll(m_exporters.begin(), m_exporters.end());
    m_exporters.clear();
}

void CT_AbstractStepPlugin::clearReaders()
{
    qDeleteAll(m_readers.begin(), m_readers.end());
    m_readers.clear();
}

void CT_AbstractStepPlugin::clearFilters()
{
    qDeleteAll(m_filters.begin(), m_filters.end());
    m_filters.clear();
}

void CT_AbstractStepPlugin::clearMetrics()
{
    qDeleteAll(m_metrics.begin(), m_metrics.end());
    m_metrics.clear();
}


CT_StepInitializeData* CT_AbstractStepPlugin::createNewStepInitializeData(CT_VirtualAbstractStep *parent) const
{
    return new CT_StepInitializeData(parent, *this, _pluginSettings);
}

CT_VirtualAbstractStep* CT_AbstractStepPlugin::searchStepFromKeyOfThisPluginInMenuRecursively(CT_StepsMenu *menu, const QString &key) const
{
    QList<CT_MenuLevel*> subLevels = menu->levels();
    QListIterator<CT_MenuLevel*> it(subLevels);

    while(it.hasNext()) {
        CT_VirtualAbstractStep *step = searchStepFromKeyOfThisPluginInLevelsRecursively(it.next(), key);

        if(step != NULL)
            return step;
    }

    return NULL;
}

CT_VirtualAbstractStep* CT_AbstractStepPlugin::searchStepFromKeyOfThisPluginInLevelsRecursively(CT_MenuLevel *level, const QString &key) const
{
    CT_VirtualAbstractStep *step = NULL;

    QList<CT_MenuLevel*> subLevels = level->levels();
    QListIterator<CT_MenuLevel*> it(subLevels);

    while(it.hasNext()) {
        CT_MenuLevel *subLevel = it.next();

        step = searchStepFromKeyOfThisPluginInLevelsRecursively(subLevel, key);

        if(step != NULL)
            return step;
    }

    QList<CT_VirtualAbstractStep*> steps = level->steps();
    QListIterator<CT_VirtualAbstractStep*> itS(steps);

    while(itS.hasNext()) {
        step = itS.next();

        if((step->getPlugin() == this) && (step->getStepName() == key))
            return step;
    }

    return NULL;
}

CT_StepSeparator* CT_AbstractStepPlugin::addNewSeparator(CT_StepSeparator *sep)
{
    if(sep != NULL) {
        sep->m_menuOfSteps = menuOfSteps();
        _stepAvailable.append(sep);
    }

    return sep;
}

CT_StepLoadFileSeparator* CT_AbstractStepPlugin::addNewSeparator(CT_StepLoadFileSeparator *sep)
{
    if(sep != NULL) {
        sep->m_menuOfSteps = menuOfSteps();
        _stepOpenFileAvailable.append(sep);
    }

    return sep;
}

CT_StepCanBeAddedFirstSeparator* CT_AbstractStepPlugin::addNewSeparator(CT_StepCanBeAddedFirstSeparator *sep)
{
    if(sep != NULL) {
        sep->m_menuOfSteps = menuOfSteps();
        _stepCanBeAddedFirstAvailable.append(sep);
    }

    return sep;
}

CT_ActionsSeparator* CT_AbstractStepPlugin::addNewSeparator(CT_ActionsSeparator *sep)
{
    if(sep != NULL)
        m_actions.append(sep);

    return sep;
}

CT_StandardExporterSeparator* CT_AbstractStepPlugin::addNewSeparator(CT_StandardExporterSeparator *sep)
{
    if(sep != NULL)
        m_exporters.append(sep);

    return sep;
}

CT_StandardReaderSeparator *CT_AbstractStepPlugin::addNewSeparator(CT_StandardReaderSeparator *sep)
{
    if(sep != NULL)
        m_readers.append(sep);

    return sep;
}

void CT_AbstractStepPlugin::addNewFilter(CT_AbstractFilter *filter)
{
    if(filter != NULL)
        m_filters.append(filter);
}

void CT_AbstractStepPlugin::addNewMetric(CT_AbstractMetric *metric)
{
    if(metric != NULL)
        m_metrics.append(metric);
}

void CT_AbstractStepPlugin::addNewStep(CT_VirtualAbstractStep *step, CT_StepsMenu::LevelOperationType levelOperation, const QString &subLevelDisplayableName)
{
    CT_MenuLevel *level = menuOfSteps()->getOrCreateRootLevel(levelOperation);

    if(!subLevelDisplayableName.isEmpty())
        level = CT_MenuLevel::getOrCreateLevel(subLevelDisplayableName, level);

    level->addStepToCollectionOrDeleteIt(step);
}

void CT_AbstractStepPlugin::addNewStep(CT_VirtualAbstractStep *step, CT_StepsMenu::LevelOperationType levelOperation, CT_StepsMenu::LevelPredefined subLevelPredefined)
{
    addNewStep(step, levelOperation, CT_StepsMenu::staticPredefinedToString(subLevelPredefined));
}

CT_StepsMenu *CT_AbstractStepPlugin::menuOfSteps() const
{
    return m_coreInterface->stepsMenu();
}

void CT_AbstractStepPlugin::clearMemory()
{
    clearGenericsStep();
    clearOpenFileStep();
    clearCanBeAddedFirstStep();
    clearActions();
    clearExporters();
    clearReaders();
    clearFilters();
    clearMetrics();
}

void CT_AbstractStepPlugin::initStep(CT_VirtualAbstractStep *step) const
{
    // to show in model
    step->initInResultModelList();

    step->init();

    // to show a default out model :

    // active the default out model creation
    step->setCreateDefaultOutModelActive(true);

    // create a default in turn no configured
    step->getInTurnManager()->createTurn();
    // create a default out turn and don't check if in turn is valid
    step->preProcessCreateOutResultModelListProtected();
    step->getInTurnManager()->createSearchModelListForCurrentTurn();
    step->createOutResultModelListProtected();
    step->getOutTurnManager()->createTurn(false);

    step->setCreateDefaultOutModelActive(false);
}

void CT_AbstractStepPlugin::convertStepSeparatorToOperationAndLevel()
{
    CT_MenuLevel *rootLevel = menuOfSteps()->getOrCreateRootLevel(CT_StepsMenu::LO_Other);

    QList<CT_StepSeparator*> listGenerics = getGenericsStepAvailable();
    QListIterator<CT_StepSeparator*> itG(listGenerics);

    while(itG.hasNext())
    {
        CT_StepSeparator *sep = itG.next();

        CT_MenuLevel *level = rootLevel;

        if(!sep->getTitle().isEmpty())
            level = CT_MenuLevel::getOrCreateLevel(sep->getTitle(), rootLevel);

        QList<CT_VirtualAbstractStep*> stepList = sep->getStepList();
        QListIterator<CT_VirtualAbstractStep*> itS(stepList);

        while(itS.hasNext())
            level->addStepToCollectionOrDeleteIt(itS.next()->createNewInstance(*createNewStepInitializeData(NULL)));
    }

    rootLevel = menuOfSteps()->getOrCreateRootLevel(CT_StepsMenu::LO_Load);

    QList<CT_StepLoadFileSeparator*> listOpen = getOpenFileStepAvailable();
    QListIterator<CT_StepLoadFileSeparator*> itO(listOpen);

    while(itO.hasNext())
    {
        CT_StepLoadFileSeparator *sep = itO.next();

        CT_MenuLevel *level = rootLevel;

        if(!sep->typeOfFile().isEmpty())
            level = CT_MenuLevel::getOrCreateLevel(sep->typeOfFile(), rootLevel);

        QList<CT_AbstractStepLoadFile*> stepList = sep->getStepList();
        QListIterator<CT_AbstractStepLoadFile*> itS(stepList);

        while(itS.hasNext())
            level->addStepToCollectionOrDeleteIt(itS.next()->createNewInstance(*createNewStepInitializeData(NULL)));
    }

    rootLevel = menuOfSteps()->getOrCreateRootLevel(CT_StepsMenu::LO_Other);

    QList<CT_StepCanBeAddedFirstSeparator*> listFirst = getCanBeAddedFirstStepAvailable();
    QListIterator<CT_StepCanBeAddedFirstSeparator*> itF(listFirst);

    while(itF.hasNext())
    {
        CT_StepCanBeAddedFirstSeparator *sep = itF.next();

        CT_MenuLevel *level = rootLevel;

        if(!sep->getTitle().isEmpty())
            level = CT_MenuLevel::getOrCreateLevel(sep->getTitle(), rootLevel);

        QList<CT_AbstractStepCanBeAddedFirst*> stepList = sep->getStepList();
        QListIterator<CT_AbstractStepCanBeAddedFirst*> itS(stepList);

        while(itS.hasNext())
            level->addStepToCollectionOrDeleteIt(itS.next()->createNewInstance(*createNewStepInitializeData(NULL)));
    }
}

void CT_AbstractStepPlugin::initAllStepOfThisPluginInLevelsRecursively(CT_MenuLevel *level)
{
    QList<CT_MenuLevel*> subLevels = level->levels();
    QListIterator<CT_MenuLevel*> it(subLevels);

    while(it.hasNext()) {
        CT_MenuLevel *subLevel = it.next();

        initAllStepOfThisPluginInLevelsRecursively(subLevel);
    }

    QList<CT_VirtualAbstractStep*> steps = level->steps();
    QListIterator<CT_VirtualAbstractStep*> itS(steps);

    while(itS.hasNext()) {
        CT_VirtualAbstractStep *step = itS.next();

        if(step->getPlugin() == this)
            initStep(step);
    }
}

void CT_AbstractStepPlugin::setCoreInterface(const CoreInterface *core)
{
    PS_CONTEXT->setCoreInterface(core);
    m_coreInterface = (CoreInterface*)core;
}

void CT_AbstractStepPlugin::setGuiManager(const GuiManagerInterface *gManager)
{
    PS_CONTEXT->setGuiManager(gManager);
}
