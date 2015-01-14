#include "ct_standardmeshmodelopfdrawmanager.h"

#include "ct_itemdrawable/abstract/ct_abstractmeshmodel.h"

#include <QtOpenGL>

QT_GL_SHADERPROGRAM* CT_StandardMeshModelOPFDrawManager::SHADER_PROG = NULL;
QT_GL_SHADER* CT_StandardMeshModelOPFDrawManager::SHADER = NULL;
bool CT_StandardMeshModelOPFDrawManager::SHADER_ERROR = false;
int CT_StandardMeshModelOPFDrawManager::MATRIX_LOCATION = -1;
int CT_StandardMeshModelOPFDrawManager::DELTAX_LOCATION = -1;
int CT_StandardMeshModelOPFDrawManager::DELTAD_LOCATION = -1;
int CT_StandardMeshModelOPFDrawManager::MINX_LOCATION = -1;
int CT_StandardMeshModelOPFDrawManager::DDOWN_LOCATION = -1;
int CT_StandardMeshModelOPFDrawManager::N_SHARED = 0;

CT_StandardMeshModelOPFDrawManager::CT_StandardMeshModelOPFDrawManager(QString drawConfigurationName) : CT_StandardMeshModelDrawManager(drawConfigurationName)
{
    m_dUp = 1.0;
    m_dDown = 1.0;
    ++N_SHARED;

    if(SHADER_PROG == NULL)
        SHADER_PROG = new QT_GL_SHADERPROGRAM();
}

CT_StandardMeshModelOPFDrawManager::~CT_StandardMeshModelOPFDrawManager()
{
    --N_SHARED;

    if(N_SHARED == 0)
    {
        delete SHADER;
        SHADER = NULL;

        delete SHADER_PROG;
        SHADER_PROG = NULL;
    }
}

bool CT_StandardMeshModelOPFDrawManager::useItemTransformMatrix() const
{
    return true;
}

void CT_StandardMeshModelOPFDrawManager::draw(GraphicsViewInterface &view, PainterInterface &painter, const CT_AbstractItemDrawable &itemDrawable) const
{
    Q_UNUSED(view)

    const CT_AbstractMeshModel &item = dynamic_cast<const CT_AbstractMeshModel&>(itemDrawable);

    bool ok = false;

    if((dUp() != 1.0) || (dDown() != 1.0))
    {
        initShaders();

        if(!SHADER_ERROR && (QT_GL_CONTEXT::currentContext() != NULL))
        {
            if(SHADER_PROG->isLinked())
            {
                if(!SHADER_PROG->bind())
                {
                    QString log = SHADER_PROG->log();

                    if(!log.isEmpty())
                        PS_LOG->addErrorMessage(LogInterface::unknow, QObject::tr("CT_StandardMeshModelOPFDrawManager => Bind error : %1").arg(SHADER_PROG->log()));
                }
                else
                    ok = true;
            }
        }
    }

    if(ok)
    {
        if(MATRIX_LOCATION == -1)
        {
            MATRIX_LOCATION = SHADER_PROG->uniformLocation("tMatrix");
            DELTAX_LOCATION = SHADER_PROG->uniformLocation("deltaX");
            DELTAD_LOCATION = SHADER_PROG->uniformLocation("deltaD");
            MINX_LOCATION = SHADER_PROG->uniformLocation("minX");
            DDOWN_LOCATION = SHADER_PROG->uniformLocation("dDown");
        }

        Eigen::Vector3d min, max;
        item.getBoundingBox(min, max);

        double deltaX = max(0) - min(0);

        SHADER_PROG->setUniformValue(MATRIX_LOCATION, item.transformMatrix());
        SHADER_PROG->setUniformValue(DELTAX_LOCATION, (float)deltaX);
        SHADER_PROG->setUniformValue(DELTAD_LOCATION, (float)m_deltaD);
        SHADER_PROG->setUniformValue(MINX_LOCATION, (float)min(0));
        SHADER_PROG->setUniformValue(DDOWN_LOCATION, (float)m_dDown);
    }
    else
    {
        painter.pushMatrix();
        QMatrix4x4 m = item.transformMatrix();
        Eigen::Matrix4d em;
        em << m(0, 0), m(0, 1), m(0, 2), m(0, 3),
              m(1, 0), m(1, 1), m(1, 2), m(1, 3),
              m(2, 0), m(2, 1), m(2, 2), m(2, 3),
              m(3, 0), m(3, 1), m(3, 2), m(3, 3);

        painter.multMatrix(em);
    }

    bool showFaces = getDrawConfiguration()->getVariableValue(INDEX_CONFIG_SHOW_FACES).toBool();
    bool showEdges = getDrawConfiguration()->getVariableValue(INDEX_CONFIG_SHOW_EDGES).toBool();
    bool showPoints = getDrawConfiguration()->getVariableValue(INDEX_CONFIG_SHOW_POINTS).toBool();

    if(showFaces)
        painter.drawFaces(&item);

    if(showEdges)
        painter.drawEdges(&item);

    if(showPoints)
        painter.drawPoints(&item);

    if(!ok)
        painter.popMatrix();
    else
        SHADER_PROG->release();
}

void CT_StandardMeshModelOPFDrawManager::setDUp(double dUp)
{
    m_dUp = dUp;
    m_deltaD = m_dUp - m_dDown;
}

void CT_StandardMeshModelOPFDrawManager::setDDown(double dDown)
{
    m_dDown = dDown;
    m_deltaD = m_dUp - m_dDown;
}

double CT_StandardMeshModelOPFDrawManager::dUp() const
{
    return m_dUp;
}

double CT_StandardMeshModelOPFDrawManager::dDown() const
{
    return m_dDown;
}

void CT_StandardMeshModelOPFDrawManager::setDrawConfiguration(CT_ItemDrawableConfiguration *dc)
{
    internalSetDrawConfiguration(dc);
}

void CT_StandardMeshModelOPFDrawManager::setAutoDeleteDrawConfiguration(bool e)
{
    internalSetAutoDeleteDrawConfiguration(e);
}

CT_ItemDrawableConfiguration CT_StandardMeshModelOPFDrawManager::createDrawConfiguration(QString drawConfigurationName) const
{
    return CT_StandardMeshModelDrawManager::createDrawConfiguration(drawConfigurationName);
}

void CT_StandardMeshModelOPFDrawManager::initShaders() const
{
    if(!SHADER_ERROR && (QT_GL_CONTEXT::currentContext() != NULL))
    {
        if(SHADER == NULL)
        {
            SHADER = new QT_GL_SHADER(QT_GL_SHADER::Vertex);

            if(!SHADER->compileSourceFile("./shaders/opfMesh.vert"))
            {
                PS_LOG->addErrorMessage(LogInterface::unknow, QObject::tr("CT_StandardMeshModelOPFDrawManager => Vertex shader compilation error : %1").arg(SHADER->log()));

                delete SHADER;
                SHADER = NULL;

                SHADER_ERROR = true;
            }
            else
            {
                SHADER_PROG->addShader(SHADER);
            }

            if(!SHADER_PROG->shaders().isEmpty() && !SHADER_PROG->isLinked())
            {
                if(!SHADER_PROG->link())
                {
                    PS_LOG->addErrorMessage(LogInterface::unknow, QObject::tr("CT_StandardMeshModelOPFDrawManager => Link error : %1").arg(SHADER_PROG->log()));
                    SHADER_ERROR = true;
                }
            }
        }
    }
}
