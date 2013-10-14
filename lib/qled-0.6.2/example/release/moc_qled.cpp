/****************************************************************************
** Meta object code from reading C++ file 'qled.h'
**
** Created: Fri 22. Jan 09:23:10 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qled.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qled.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QLed[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       4,   39, // properties
       2,   51, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       6,    5,    5,    5, 0x0a,
      21,    5,    5,    5, 0x0a,
      42,    5,    5,    5, 0x0a,
      64,    5,    5,    5, 0x0a,
      83,    5,    5,    5, 0x0a,

 // properties: name, type, flags
     102,   97, 0x01095103,
     117,  108, 0x0009510b,
     125,  108, 0x0009510b,
     143,  134, 0x0009510b,

 // enums: name, flags, count, data
     108, 0x0,    7,   59,
     134, 0x0,    4,   73,

 // enum data: key, value
     149, uint(QLed::Red),
     153, uint(QLed::Green),
     159, uint(QLed::Yellow),
     166, uint(QLed::Grey),
     171, uint(QLed::Orange),
     178, uint(QLed::Purple),
     185, uint(QLed::Blue),
     190, uint(QLed::Circle),
     197, uint(QLed::Square),
     204, uint(QLed::Triangle),
     213, uint(QLed::Rounded),

       0        // eod
};

static const char qt_meta_stringdata_QLed[] = {
    "QLed\0\0setValue(bool)\0setOnColor(ledColor)\0"
    "setOffColor(ledColor)\0setShape(ledShape)\0"
    "toggleValue()\0bool\0value\0ledColor\0"
    "onColor\0offColor\0ledShape\0shape\0Red\0"
    "Green\0Yellow\0Grey\0Orange\0Purple\0Blue\0"
    "Circle\0Square\0Triangle\0Rounded\0"
};

const QMetaObject QLed::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QLed,
      qt_meta_data_QLed, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QLed::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QLed::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QLed::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QLed))
        return static_cast<void*>(const_cast< QLed*>(this));
    return QWidget::qt_metacast(_clname);
}

int QLed::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setValue((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: setOnColor((*reinterpret_cast< ledColor(*)>(_a[1]))); break;
        case 2: setOffColor((*reinterpret_cast< ledColor(*)>(_a[1]))); break;
        case 3: setShape((*reinterpret_cast< ledShape(*)>(_a[1]))); break;
        case 4: toggleValue(); break;
        default: ;
        }
        _id -= 5;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = value(); break;
        case 1: *reinterpret_cast< ledColor*>(_v) = onColor(); break;
        case 2: *reinterpret_cast< ledColor*>(_v) = offColor(); break;
        case 3: *reinterpret_cast< ledShape*>(_v) = shape(); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setValue(*reinterpret_cast< bool*>(_v)); break;
        case 1: setOnColor(*reinterpret_cast< ledColor*>(_v)); break;
        case 2: setOffColor(*reinterpret_cast< ledColor*>(_v)); break;
        case 3: setShape(*reinterpret_cast< ledShape*>(_v)); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
