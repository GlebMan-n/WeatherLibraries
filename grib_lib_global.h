#ifndef GRIB_LIB_GLOBAL_H
#define GRIB_LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef GRIB_LIB_LIB
    #define GRIB_LIB_EXPORT Q_DECL_EXPORT
#else
    #define GRIB_LIB_EXPORT Q_DECL_IMPORT
#endif

#endif // GRIB_LIB_GLOBAL_H
