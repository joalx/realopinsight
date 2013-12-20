#ifndef LIBREALOPINSIGHT_GLOBAL_H
#define LIBREALOPINSIGHT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBREALOPINSIGHT_LIBRARY)
#  define LIBREALOPINSIGHTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBREALOPINSIGHTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBREALOPINSIGHT_GLOBAL_H