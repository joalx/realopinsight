/****************************************************************************
** Meta object code from reading C++ file 'ServiceEditor.hpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../client/src/ServiceEditor.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ServiceEditor.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ServiceEditor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      29,   14,   14,   14, 0x05,
      44,   14,   14,   14, 0x05,
      60,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      86,   14,   14,   14, 0x0a,
     104,   14,   14,   14, 0x0a,
     123,   14,   14,   14, 0x0a,
     145,   14,   14,   14, 0x0a,
     182,  176,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ServiceEditor[] = {
    "ServiceEditor\0\0saveClicked()\0"
    "closeClicked()\0returnPressed()\0"
    "nodeTypeActivated(qint32)\0handleSaveClick()\0"
    "handleCloseClick()\0handleReturnPressed()\0"
    "handleNodeTypeChanged(QString)\0_text\0"
    "handleNodeTypeActivated(QString)\0"
};

void ServiceEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ServiceEditor *_t = static_cast<ServiceEditor *>(_o);
        switch (_id) {
        case 0: _t->saveClicked(); break;
        case 1: _t->closeClicked(); break;
        case 2: _t->returnPressed(); break;
        case 3: _t->nodeTypeActivated((*reinterpret_cast< qint32(*)>(_a[1]))); break;
        case 4: _t->handleSaveClick(); break;
        case 5: _t->handleCloseClick(); break;
        case 6: _t->handleReturnPressed(); break;
        case 7: _t->handleNodeTypeChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->handleNodeTypeActivated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ServiceEditor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ServiceEditor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ServiceEditor,
      qt_meta_data_ServiceEditor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ServiceEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ServiceEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ServiceEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ServiceEditor))
        return static_cast<void*>(const_cast< ServiceEditor*>(this));
    return QWidget::qt_metacast(_clname);
}

int ServiceEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void ServiceEditor::saveClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ServiceEditor::closeClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void ServiceEditor::returnPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void ServiceEditor::nodeTypeActivated(qint32 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE