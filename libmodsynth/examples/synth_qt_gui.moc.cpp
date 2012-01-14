/****************************************************************************
** Meta object code from reading C++ file 'less_trivial_synth_qt_gui.h'
**
** Created: Mon Jan 9 22:07:33 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "synth_qt_gui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'less_trivial_synth_qt_gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SynthGUI[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   10,    9,    9, 0x0a,
      34,   30,    9,    9, 0x0a,
      51,   30,    9,    9, 0x0a,
      75,   67,    9,    9, 0x0a,
      93,   30,    9,    9, 0x0a,
     115,  111,    9,    9, 0x0a,
     132,  111,    9,    9, 0x0a,
     146,  111,    9,    9, 0x0a,
     161,    9,    9,    9, 0x0a,
     175,    9,    9,    9, 0x09,
     194,    9,    9,    9, 0x09,
     213,    9,    9,    9, 0x09,
     231,    9,    9,    9, 0x09,
     251,    9,    9,    9, 0x09,
     271,    9,    9,    9, 0x09,
     290,    9,    9,    9, 0x09,
     306,    9,    9,    9, 0x09,
     323,    9,    9,    9, 0x09,
     336,    9,    9,    9, 0x09,
     351,    9,    9,    9, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_SynthGUI[] = {
    "SynthGUI\0\0hz\0setTuning(float)\0sec\0"
    "setAttack(float)\0setDecay(float)\0"
    "percent\0setSustain(float)\0setRelease(float)\0"
    "val\0setTimbre(float)\0setRes(float)\0"
    "setDist(float)\0aboutToQuit()\0"
    "tuningChanged(int)\0attackChanged(int)\0"
    "decayChanged(int)\0sustainChanged(int)\0"
    "releaseChanged(int)\0timbreChanged(int)\0"
    "resChanged(int)\0distChanged(int)\0"
    "test_press()\0test_release()\0oscRecv()\0"
};

const QMetaObject SynthGUI::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_SynthGUI,
      qt_meta_data_SynthGUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SynthGUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SynthGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SynthGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SynthGUI))
        return static_cast<void*>(const_cast< SynthGUI*>(this));
    return QFrame::qt_metacast(_clname);
}

int SynthGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        //case 0: setTuning((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: setAttack((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: setDecay((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: setSustain((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 4: setRelease((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 5: setTimbre((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 6: setRes((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 7: setDist((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 8: aboutToQuit(); break;
        //case 9: tuningChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: attackChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: decayChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: sustainChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: releaseChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: timbreChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: resChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: distChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: test_press(); break;
        case 18: test_release(); break;
        case 19: oscRecv(); break;
        default: ;
        }
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
