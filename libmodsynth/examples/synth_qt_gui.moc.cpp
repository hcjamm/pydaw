/****************************************************************************
** Meta object code from reading C++ file 'synth_qt_gui.h'
**
** Created: Sun Jan 15 23:04:44 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "synth_qt_gui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'synth_qt_gui.h' doesn't include <QObject>."
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
      28,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   10,    9,    9, 0x0a,
      31,   10,    9,    9, 0x0a,
      51,   47,    9,    9, 0x0a,
      69,   10,    9,    9, 0x0a,
      87,   47,    9,    9, 0x0a,
     104,   47,    9,    9, 0x0a,
     118,   47,    9,    9, 0x0a,
     133,   10,    9,    9, 0x0a,
     156,   10,    9,    9, 0x0a,
     178,   47,    9,    9, 0x0a,
     202,   10,    9,    9, 0x0a,
     226,   47,    9,    9, 0x0a,
     245,    9,    9,    9, 0x0a,
     259,    9,    9,    9, 0x09,
     278,    9,    9,    9, 0x09,
     296,    9,    9,    9, 0x09,
     316,    9,    9,    9, 0x09,
     336,    9,    9,    9, 0x09,
     355,    9,    9,    9, 0x09,
     371,    9,    9,    9, 0x09,
     388,    9,    9,    9, 0x09,
     413,    9,    9,    9, 0x09,
     437,    9,    9,    9, 0x09,
     463,    9,    9,    9, 0x09,
     489,    9,    9,    9, 0x09,
     510,    9,    9,    9, 0x09,
     523,    9,    9,    9, 0x09,
     538,    9,    9,    9, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_SynthGUI[] = {
    "SynthGUI\0\0sec\0setAttack(float)\0"
    "setDecay(float)\0val\0setSustain(float)\0"
    "setRelease(float)\0setTimbre(float)\0"
    "setRes(float)\0setDist(float)\0"
    "setFilterAttack(float)\0setFilterDecay(float)\0"
    "setFilterSustain(float)\0setFilterRelease(float)\0"
    "setNoiseAmp(float)\0aboutToQuit()\0"
    "attackChanged(int)\0decayChanged(int)\0"
    "sustainChanged(int)\0releaseChanged(int)\0"
    "timbreChanged(int)\0resChanged(int)\0"
    "distChanged(int)\0filterAttackChanged(int)\0"
    "filterDecayChanged(int)\0"
    "filterSustainChanged(int)\0"
    "filterReleaseChanged(int)\0"
    "noiseAmpChanged(int)\0test_press()\0"
    "test_release()\0oscRecv()\0"
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
        case 0: setAttack((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: setDecay((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: setSustain((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: setRelease((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 4: setTimbre((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 5: setRes((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 6: setDist((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 7: setFilterAttack((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 8: setFilterDecay((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 9: setFilterSustain((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 10: setFilterRelease((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 11: setNoiseAmp((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 12: aboutToQuit(); break;
        case 13: attackChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: decayChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: sustainChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: releaseChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: timbreChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: resChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: distChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: filterAttackChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: filterDecayChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: filterSustainChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: filterReleaseChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: noiseAmpChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: test_press(); break;
        case 26: test_release(); break;
        case 27: oscRecv(); break;
        default: ;
        }
        _id -= 28;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
