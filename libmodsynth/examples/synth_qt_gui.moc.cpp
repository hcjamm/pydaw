/****************************************************************************
** Meta object code from reading C++ file 'synth_qt_gui.h'
**
** Created: Tue Jan 17 22:03:01 2012
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
      50,   14, // methods
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
     245,   47,    9,    9, 0x0a,
     268,   47,    9,    9, 0x0a,
     286,   47,    9,    9, 0x0a,
     305,   47,    9,    9, 0x0a,
     325,   47,    9,    9, 0x0a,
     344,   47,    9,    9, 0x0a,
     365,   47,    9,    9, 0x0a,
     384,   47,    9,    9, 0x0a,
     404,   47,    9,    9, 0x0a,
     423,   47,    9,    9, 0x0a,
     444,   47,    9,    9, 0x0a,
     467,    9,    9,    9, 0x0a,
     481,    9,    9,    9, 0x09,
     500,    9,    9,    9, 0x09,
     518,    9,    9,    9, 0x09,
     538,    9,    9,    9, 0x09,
     558,    9,    9,    9, 0x09,
     577,    9,    9,    9, 0x09,
     593,    9,    9,    9, 0x09,
     610,    9,    9,    9, 0x09,
     635,    9,    9,    9, 0x09,
     659,    9,    9,    9, 0x09,
     685,    9,    9,    9, 0x09,
     711,    9,    9,    9, 0x09,
     732,    9,    9,    9, 0x09,
     757,    9,    9,    9, 0x09,
     777,    9,    9,    9, 0x09,
     798,    9,    9,    9, 0x09,
     820,    9,    9,    9, 0x09,
     841,    9,    9,    9, 0x09,
     864,    9,    9,    9, 0x09,
     885,    9,    9,    9, 0x09,
     907,    9,    9,    9, 0x09,
     928,    9,    9,    9, 0x09,
     951,    9,    9,    9, 0x09,
     976,    9,    9,    9, 0x09,
     989,    9,    9,    9, 0x09,
    1004,    9,    9,    9, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_SynthGUI[] = {
    "SynthGUI\0\0sec\0setAttack(float)\0"
    "setDecay(float)\0val\0setSustain(float)\0"
    "setRelease(float)\0setTimbre(float)\0"
    "setRes(float)\0setDist(float)\0"
    "setFilterAttack(float)\0setFilterDecay(float)\0"
    "setFilterSustain(float)\0setFilterRelease(float)\0"
    "setNoiseAmp(float)\0setFilterEnvAmt(float)\0"
    "setDistWet(float)\0setOsc1Type(float)\0"
    "setOsc1Pitch(float)\0setOsc1Tune(float)\0"
    "setOsc1Volume(float)\0setOsc2Type(float)\0"
    "setOsc2Pitch(float)\0setOsc2Tune(float)\0"
    "setOsc2Volume(float)\0setMasterVolume(float)\0"
    "aboutToQuit()\0attackChanged(int)\0"
    "decayChanged(int)\0sustainChanged(int)\0"
    "releaseChanged(int)\0timbreChanged(int)\0"
    "resChanged(int)\0distChanged(int)\0"
    "filterAttackChanged(int)\0"
    "filterDecayChanged(int)\0"
    "filterSustainChanged(int)\0"
    "filterReleaseChanged(int)\0"
    "noiseAmpChanged(int)\0filterEnvAmtChanged(int)\0"
    "distWetChanged(int)\0osc1TypeChanged(int)\0"
    "osc1PitchChanged(int)\0osc1TuneChanged(int)\0"
    "osc1VolumeChanged(int)\0osc2TypeChanged(int)\0"
    "osc2PitchChanged(int)\0osc2TuneChanged(int)\0"
    "osc2VolumeChanged(int)\0masterVolumeChanged(int)\0"
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
        case 12: setFilterEnvAmt((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 13: setDistWet((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 14: setOsc1Type((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 15: setOsc1Pitch((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 16: setOsc1Tune((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 17: setOsc1Volume((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 18: setOsc2Type((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 19: setOsc2Pitch((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 20: setOsc2Tune((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 21: setOsc2Volume((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 22: setMasterVolume((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 23: aboutToQuit(); break;
        case 24: attackChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: decayChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: sustainChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: releaseChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 28: timbreChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: resChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: distChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: filterAttackChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: filterDecayChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 33: filterSustainChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 34: filterReleaseChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 35: noiseAmpChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 36: filterEnvAmtChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 37: distWetChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 38: osc1TypeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 39: osc1PitchChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 40: osc1TuneChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 41: osc1VolumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 42: osc2TypeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 43: osc2PitchChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 44: osc2TuneChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 45: osc2VolumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 46: masterVolumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 47: test_press(); break;
        case 48: test_release(); break;
        case 49: oscRecv(); break;
        default: ;
        }
        _id -= 50;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
