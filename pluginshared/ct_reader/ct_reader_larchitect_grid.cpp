#include "ct_reader_larchitect_grid.h"
#include <QFile>
#include <QIODevice>
#include <QTextStream>

#include "ct_pointcloud/ct_pointcloudstdvector.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_colorcloud/ct_colorcloudstdvector.h"
#include "ct_global/ct_context.h"
#include <QDebug>

#include <limits>

CT_Reader_LArchitect_Grid::CT_Reader_LArchitect_Grid() : CT_AbstractReader()
{
    _nbLinesToSkip = 4;
}

bool CT_Reader_LArchitect_Grid::setFilePath(const QString &filepath)
{
    bool valid = false;

    // Test File validity
    if(QFile::exists(filepath))
    {
        QFile f(filepath);

        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            valid = true;

            QTextStream stream(&f);

            // Read (x,y,z) min and max limits of the grid
            QString line = stream.readLine();
            if (!line.isNull() && valid)
            {
                QStringList values = line.split(" ");
                if (values.size() >= 6)
                {
                    bool okxmin, okymin, okzmin, okxmax, okymax, okzmax;
                    _xmin = values.at(0).toFloat(&okxmin);
                    _ymin = values.at(1).toFloat(&okymin);
                    _zmin = values.at(2).toFloat(&okzmin);
                    _xmax = values.at(3).toFloat(&okxmax);
                    _ymax = values.at(4).toFloat(&okymax);
                    _zmax = values.at(5).toFloat(&okzmax);

                    if (!okxmin || !okymin || !okzmin || !okxmax || !okymax || !okzmax) {valid = false;}
                }
            } else {valid = false;}

            // read resolution of the grid
            line = stream.readLine();
            if (!line.isNull() && valid)
            {
                QStringList values = line.split(" ");
                if (values.size() >= 3)
                {
                    bool okresx, okresy, okresz;
                    _resx = values.at(0).toFloat(&okresx);
                    _resy = values.at(1).toFloat(&okresy);
                    _resz = values.at(2).toFloat(&okresz);

                    if (!okresx || !okresy || !okresz) {valid = false;}
                    if (_resx != _resy || _resx != _resz) {valid = false;}
                }
            } else {valid = false;}

            // read dimensions of the grid
            line = stream.readLine();
            if (!line.isNull() && valid)
            {
                QStringList values = line.split(" ");
                if (values.size() >= 3)
                {
                    bool okdimx, okdimy, okdimz;
                    _dimx = values.at(0).toUInt(&okdimx);
                    _dimy = values.at(1).toUInt(&okdimy);
                    _dimz = values.at(2).toUInt(&okdimz);

                    if (!okdimx || !okdimy || !okdimz) {valid = false;}
                }
            } else {valid = false;}

            // read number of materials
            line = stream.readLine();
            if (!line.isNull() && valid)
            {
                QStringList values = line.split(" ");
                if (values.size() >= 1)
                {
                    bool oknmat;
                    _nmat = values.at(0).toInt(&oknmat);

                    if (!oknmat || _nmat <= 0) {valid = false;}

                    // read materials ids and names
                    for (int i = 0 ; i < _nmat && valid==true ; i++)
                    {
                        if (values.size() >= (2*i+3))
                        {
                            bool okmatid;
                            int matid;
                            QString matName;

                            matid = values.at(2*i+1).toInt(&okmatid);
                            matName = values.at(2*i+2);

                            if (!okmatid || matName.size() <= 0) {valid = false;}

                            _matNames.insert(matName.toUpper(), matid);
                        } else {valid = false;}
                    }
                }
            } else {valid = false;}

            f.close();
        } else {valid = false;}
    }

    if(valid)
        valid = CT_AbstractReader::setFilePath(filepath);

    return valid;
}

CT_AbstractReader* CT_Reader_LArchitect_Grid::copy() const
{
    return new CT_Reader_LArchitect_Grid();
}

void CT_Reader_LArchitect_Grid::protectedInit()
{
    addNewReadableFormat(FileFormat("grid", tr("Fichiers de grilles 3D LArchitect")));
}

void CT_Reader_LArchitect_Grid::protectedCreateOutItemDrawableModelList()
{
    addOutItemDrawableModel(new CT_OutStdSingularItemModel(DEF_CT_Reader_LArchitect_Grid_wood_surface, new CT_Grid3D<float>()));
    addOutItemDrawableModel(new CT_OutStdSingularItemModel(DEF_CT_Reader_LArchitect_Grid_leaf_surface, new CT_Grid3D<float>()));
    addOutItemDrawableModel(new CT_OutStdSingularItemModel(DEF_CT_Reader_LArchitect_Grid_wood_volume, new CT_Grid3D<float>()));
    addOutItemDrawableModel(new CT_OutStdSingularItemModel(DEF_CT_Reader_LArchitect_Grid_leaf_volume, new CT_Grid3D<float>()));
    addOutItemDrawableModel(new CT_OutStdSingularItemModel(DEF_CT_Reader_LArchitect_Grid_all_surface, new CT_Grid3D<float>()));
    addOutItemDrawableModel(new CT_OutStdSingularItemModel(DEF_CT_Reader_LArchitect_Grid_all_volume, new CT_Grid3D<float>()));
}

bool CT_Reader_LArchitect_Grid::protectedReadFile()
{
    bool valid = false;

    CT_Grid3D<float> *wood_surface = NULL;
    CT_Grid3D<float> *leaf_surface = NULL;
    CT_Grid3D<float> *wood_volume = NULL;
    CT_Grid3D<float> *leaf_volume = NULL;
    CT_Grid3D<float> *all_surface = NULL;
    CT_Grid3D<float> *all_volume = NULL;
    CT_Grid3D<float> *refGrid = NULL;

    if (1)
    {
        wood_surface = new CT_Grid3D<float>(NULL, NULL, _xmin, _ymin, _zmin, _dimx, _dimy, _dimz, _resx, -1, 0);
        refGrid = wood_surface;
    }
    if (1)
    {
        leaf_surface = new CT_Grid3D<float>(NULL, NULL, _xmin, _ymin, _zmin, _dimx, _dimy, _dimz, _resx, -1, 0);
        refGrid = leaf_surface;
    }
    if (1)
    {
        wood_volume = new CT_Grid3D<float>(NULL, NULL, _xmin, _ymin, _zmin, _dimx, _dimy, _dimz, _resx, -1, 0);
        refGrid = wood_volume;
    }
    if (1)
    {
        leaf_volume = new CT_Grid3D<float>(NULL, NULL, _xmin, _ymin, _zmin, _dimx, _dimy, _dimz, _resx, -1, 0);
        refGrid = leaf_volume;
    }
    if (1)
    {
        all_surface = new CT_Grid3D<float>(NULL, NULL, _xmin, _ymin, _zmin, _dimx, _dimy, _dimz, _resx, -1, 0);
        refGrid = all_surface;
    }
    if (1)
    {
        all_volume = new CT_Grid3D<float>(NULL, NULL, _xmin, _ymin, _zmin, _dimx, _dimy, _dimz, _resx, -1, 0);
        refGrid = all_volume;
    }

    int wood_index = _matNames.value("WOOD", -1);
    int leaf_index = _matNames.value("FOLIAGE", -1);

    if ((wood_surface!=NULL || wood_volume!=NULL) && wood_index==-1) {return false; qDebug() << "CT_Reader_LArchitect_Grid: pas d'index bois dans le fichier";}
    if ((leaf_surface!=NULL || leaf_volume!=NULL) && leaf_index==-1) {return false; qDebug() << "CT_Reader_LArchitect_Grid: pas d'index feuille dans le fichier";}

    // Test File validity
    if(QFile::exists(filepath()))
    {
        QFile f(filepath());

        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            valid = true;

            QTextStream stream(&f);
            qint64 fileSize = f.size();

            for (int i = 0 ; i < _nbLinesToSkip ; i++)
            {
                stream.readLine();
            }

            for (size_t xx = 0 ; xx < _dimx ; xx++)
            {
                for (size_t yy = 0 ; yy < _dimy ; yy++)
                {
                    for (size_t zz = 0 ; zz < _dimz && !stream.atEnd(); zz++)
                    {
                        size_t index;

                        QString line = stream.readLine();
                        QStringList list = line.split(" ");

                        if (refGrid->index(xx, yy, zz, index))
                        {
                            for (int i = 0 ; i < _nmat ; i++)
                            {
                                int pos = 3*i;
                                if (list.size() >= (pos + 1))
                                {
                                    bool ok;
                                    int code = list.at(pos).toInt(&ok);
                                    if (ok)
                                    {
                                        if (code == wood_index && list.size() >= pos + 3)
                                        {
                                            bool okSurf, okVol;
                                            float surface = list.at(pos+1).toFloat(&okSurf);
                                            float volume = list.at(pos+2).toFloat(&okVol);

                                            if (wood_surface!=NULL && okSurf) {wood_surface->setValueAtIndex(index, surface);}
                                            if (wood_volume!=NULL && okVol)  {wood_volume->setValueAtIndex(index, volume);}
                                            if (all_surface!=NULL && okSurf)  {all_surface->addValueAtIndex(index, surface);}
                                            if (all_volume!=NULL && okVol)   {all_volume->addValueAtIndex(index, volume);}


                                        } else if (code == leaf_index && list.size() >= pos + 3)
                                        {
                                            bool okSurf, okVol;
                                            float surface = list.at(pos+1).toFloat(&okSurf);
                                            float volume = list.at(pos+2).toFloat(&okVol);

                                            if (leaf_surface!=NULL && okSurf) {leaf_surface->setValueAtIndex(index, surface);}
                                            if (leaf_volume!=NULL && okVol)  {leaf_volume->setValueAtIndex(index, volume);}
                                            if (all_surface!=NULL && okSurf)  {all_surface->addValueAtIndex(index, surface);}
                                            if (all_volume!=NULL && okVol)   {all_volume->addValueAtIndex(index, volume);}

                                        }
                                    }
                                }
                            }
                        } else {qDebug() << "Indice hors grille";}

                        setProgress(stream.pos()*100/fileSize);
                    }
                }
            }

            f.close();
        }
    }

    if(!valid)
    {
        delete wood_surface;
        delete leaf_surface;
        delete wood_volume;
        delete leaf_volume;
        delete all_surface;
        delete all_volume;

        return false;
    }

    if (wood_surface != NULL) {wood_surface->computeMinMax();}
    if (wood_volume != NULL) {wood_volume->computeMinMax();}
    if (leaf_surface != NULL) {leaf_surface->computeMinMax();}
    if (leaf_volume != NULL) {leaf_volume->computeMinMax();}
    if (all_surface != NULL) {all_surface->computeMinMax();}
    if (all_volume != NULL) {all_volume->computeMinMax();}

    addOutItemDrawable(DEF_CT_Reader_LArchitect_Grid_all_surface, all_surface);
    addOutItemDrawable(DEF_CT_Reader_LArchitect_Grid_all_volume, all_volume);
    addOutItemDrawable(DEF_CT_Reader_LArchitect_Grid_leaf_surface, leaf_surface);
    addOutItemDrawable(DEF_CT_Reader_LArchitect_Grid_leaf_volume, leaf_volume);
    addOutItemDrawable(DEF_CT_Reader_LArchitect_Grid_wood_surface, wood_surface);
    addOutItemDrawable(DEF_CT_Reader_LArchitect_Grid_wood_volume, wood_volume);

    return true;
}