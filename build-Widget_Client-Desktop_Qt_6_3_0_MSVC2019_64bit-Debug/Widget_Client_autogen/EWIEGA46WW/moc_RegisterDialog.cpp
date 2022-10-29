/****************************************************************************
** Meta object code from reading C++ file 'RegisterDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Widget_Client/RegisterDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RegisterDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_RegisterDialog_t {
    const uint offsetsAndSize[14];
    char stringdata0[100];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_RegisterDialog_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_RegisterDialog_t qt_meta_stringdata_RegisterDialog = {
    {
QT_MOC_LITERAL(0, 14), // "RegisterDialog"
QT_MOC_LITERAL(15, 17), // "onSuccessfulLogin"
QT_MOC_LITERAL(33, 0), // ""
QT_MOC_LITERAL(34, 11), // "std::string"
QT_MOC_LITERAL(46, 4), // "hash"
QT_MOC_LITERAL(51, 22), // "on_LoginButton_clicked"
QT_MOC_LITERAL(74, 25) // "on_RegisterButton_clicked"

    },
    "RegisterDialog\0onSuccessfulLogin\0\0"
    "std::string\0hash\0on_LoginButton_clicked\0"
    "on_RegisterButton_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RegisterDialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   32,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    0,   35,    2, 0x08,    3 /* Private */,
       6,    0,   36,    2, 0x08,    4 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void RegisterDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RegisterDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onSuccessfulLogin((*reinterpret_cast< std::add_pointer_t<std::string>>(_a[1]))); break;
        case 1: _t->on_LoginButton_clicked(); break;
        case 2: _t->on_RegisterButton_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (RegisterDialog::*)(const std::string & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RegisterDialog::onSuccessfulLogin)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject RegisterDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_RegisterDialog.offsetsAndSize,
    qt_meta_data_RegisterDialog,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_RegisterDialog_t
, QtPrivate::TypeAndForceComplete<RegisterDialog, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const std::string &, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *RegisterDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RegisterDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RegisterDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "std::enable_shared_from_this<RegisterDialog>"))
        return static_cast< std::enable_shared_from_this<RegisterDialog>*>(this);
    return QDialog::qt_metacast(_clname);
}

int RegisterDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void RegisterDialog::onSuccessfulLogin(const std::string & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
