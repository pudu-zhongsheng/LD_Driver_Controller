/****************************************************************************
** Meta object code from reading C++ file 'it8512plus_widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../devices/load/it8512plus/it8512plus_widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'it8512plus_widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IT8512Plus_Widget_t {
    QByteArrayData data[20];
    char stringdata0[295];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IT8512Plus_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IT8512Plus_Widget_t qt_meta_stringdata_IT8512Plus_Widget = {
    {
QT_MOC_LITERAL(0, 0, 17), // "IT8512Plus_Widget"
QT_MOC_LITERAL(1, 18, 15), // "serialConnected"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 8), // "portName"
QT_MOC_LITERAL(4, 44, 18), // "serialDisconnected"
QT_MOC_LITERAL(5, 63, 13), // "statusUpdated"
QT_MOC_LITERAL(6, 77, 7), // "voltage"
QT_MOC_LITERAL(7, 85, 7), // "current"
QT_MOC_LITERAL(8, 93, 5), // "power"
QT_MOC_LITERAL(9, 99, 16), // "updateDeviceInfo"
QT_MOC_LITERAL(10, 116, 12), // "updateStatus"
QT_MOC_LITERAL(11, 129, 20), // "onControlModeChanged"
QT_MOC_LITERAL(12, 150, 20), // "onOutputStateChanged"
QT_MOC_LITERAL(13, 171, 17), // "onWorkModeChanged"
QT_MOC_LITERAL(14, 189, 21), // "onValueSettingChanged"
QT_MOC_LITERAL(15, 211, 23), // "onDynamicSettingChanged"
QT_MOC_LITERAL(16, 235, 15), // "onTriggerSignal"
QT_MOC_LITERAL(17, 251, 17), // "checkTargetValues"
QT_MOC_LITERAL(18, 269, 19), // "onMaxVoltageChanged"
QT_MOC_LITERAL(19, 289, 5) // "value"

    },
    "IT8512Plus_Widget\0serialConnected\0\0"
    "portName\0serialDisconnected\0statusUpdated\0"
    "voltage\0current\0power\0updateDeviceInfo\0"
    "updateStatus\0onControlModeChanged\0"
    "onOutputStateChanged\0onWorkModeChanged\0"
    "onValueSettingChanged\0onDynamicSettingChanged\0"
    "onTriggerSignal\0checkTargetValues\0"
    "onMaxVoltageChanged\0value"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IT8512Plus_Widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   79,    2, 0x06 /* Public */,
       4,    0,   82,    2, 0x06 /* Public */,
       5,    3,   83,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   90,    2, 0x08 /* Private */,
      10,    0,   91,    2, 0x08 /* Private */,
      11,    0,   92,    2, 0x08 /* Private */,
      12,    0,   93,    2, 0x08 /* Private */,
      13,    0,   94,    2, 0x08 /* Private */,
      14,    0,   95,    2, 0x08 /* Private */,
      15,    0,   96,    2, 0x08 /* Private */,
      16,    0,   97,    2, 0x08 /* Private */,
      17,    0,   98,    2, 0x08 /* Private */,
      18,    1,   99,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float, QMetaType::Float, QMetaType::Float,    6,    7,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   19,

       0        // eod
};

void IT8512Plus_Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IT8512Plus_Widget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->serialConnected((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->serialDisconnected(); break;
        case 2: _t->statusUpdated((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2])),(*reinterpret_cast< float(*)>(_a[3]))); break;
        case 3: _t->updateDeviceInfo(); break;
        case 4: _t->updateStatus(); break;
        case 5: _t->onControlModeChanged(); break;
        case 6: _t->onOutputStateChanged(); break;
        case 7: _t->onWorkModeChanged(); break;
        case 8: _t->onValueSettingChanged(); break;
        case 9: _t->onDynamicSettingChanged(); break;
        case 10: _t->onTriggerSignal(); break;
        case 11: _t->checkTargetValues(); break;
        case 12: _t->onMaxVoltageChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (IT8512Plus_Widget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IT8512Plus_Widget::serialConnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (IT8512Plus_Widget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IT8512Plus_Widget::serialDisconnected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (IT8512Plus_Widget::*)(float , float , float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IT8512Plus_Widget::statusUpdated)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IT8512Plus_Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<LoadBase::staticMetaObject>(),
    qt_meta_stringdata_IT8512Plus_Widget.data,
    qt_meta_data_IT8512Plus_Widget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IT8512Plus_Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IT8512Plus_Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IT8512Plus_Widget.stringdata0))
        return static_cast<void*>(this);
    return LoadBase::qt_metacast(_clname);
}

int IT8512Plus_Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LoadBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void IT8512Plus_Widget::serialConnected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IT8512Plus_Widget::serialDisconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void IT8512Plus_Widget::statusUpdated(float _t1, float _t2, float _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
