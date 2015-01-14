#pragma once
#include "os.storage.h"
#include "os.log.h"
#include <deque>

// data types

#define DATA_TYPE_INT32        'a'
#define DATA_TYPE_INT32_ARRAY  'A'
#define DATA_TYPE_BOOL         'b'
#define DATA_TYPE_CHAR_ARRAY   'c'
#define DATA_TYPE_SUBELT_AS_CHAR_ARRAY   'C'
#define DATA_TYPE_DOUBLE       'd'
#define DATA_TYPE_DOUBLE_ARRAY 'D'
#define DATA_TYPE_FLOAT        'f'
#define DATA_TYPE_FLOAT_ARRAY  'F'
#define DATA_TYPE_CHAR         's'
#define DATA_TYPE_STRING_AS_CHAR_ARRAY   'S'

// keys

#define KEY_IC_DQROT            'a' // 4 double
#define KEY_IC_DPOS             'b' // 3 double
#define KEY_IC_SET              'c' // bool
#define KEY_DATE_CREA           'd' // string
#define KEY_GUID                'e' // string
#define KEY_RAWPATH_GUID        'f' // string
#define KEY_INTPATH_GUID        'g' // string
#define KEY_REGPATH_GUID        'h' // string
#define KEY_NAME                'i' // string
#define KEY_ITGR_MODE           'j' // int32
#define KEY_FILTER_RATE         'k' // double
#define KEY_FILTER_CUTOFF       'l' // double
#define KEY_FILTER_ADAPT        'm' // bool
#define KEY_FPS                 'n' // int32
#define KEY_VEC_ACC_POS         'P' // n * 4 double
#define KEY_VEC_INT_POS         'p' // n * 7 double
#define KEY_VEC_ACC_ROT         'R' // n * 4 double
#define KEY_VEC_INT_ROT         'r' // n * 5 double
/////////////////////// keys for accelerometer
#define KEY_ACC_SEN_BIAS        's' // 3 double
#define KEY_ACC_SEN_MAX_GRAV    't' // 3 double
#define KEY_ACC_SEN_MIN_GRAV    'u' // 3 double
#define KEY_ACC_SEN_NOISE_AMP   'v' // 3 double
#define KEY_ACC_SEN_NOISE_MIN   'w' // 3 double
#define KEY_ACC_SEN_NOISE_MAX   'x' // 3 double
#define KEY_ACC_CALI_FROM_NOISE 'z' // bool
/////////////////////// keys for extremity types
#define KEY_ETY_T_FIXEDPOINTINSPACE       -1 // subelement
#define KEY_ETY_T_POINTINSPACE            -2 // subelement
#define KEY_ETY_T_POINTONWIREFROMSTART    -3 // subelement
#define KEY_ETY_T_POINTONWIREFROMEND      -4 // subelement
/////////////////////// keys for wire types
#define KEY_WRE_T_LINE                   -30 // subelement
#define KEY_WRE_T_ARCOFCIRCLE            -31 // subelement
#define KEY_WRE_T_BEZIER                 -32 // subelement
#define KEY_WRE_T_FORMULA                -33 // subelement
/////////////////////// keys for extremity and wire attributes
#define KEY_REF                     -50 // int32
/////////////////////// keys for extremity attributes
#define KEY_ETY_VEC_XYZ             -60 // 3 wmfloats
#define KEY_ETY_VARNAME_X           -61 // string
#define KEY_ETY_VARNAME_Y           -62 // string
#define KEY_ETY_VARNAME_Z           -63 // string
#define KEY_ETY_VARNAME_RATIO       -64 // string
#define KEY_ETY_WRE_REF             -65 // int32
/////////////////////// keys for wire attributes
#define KEY_WRE_ETY_START_REF   -90 // int32
#define KEY_WRE_ETY_END_REF     -91 // int32
#define KEY_WRE_ETY_START_RNDBLE -92 // bool
#define KEY_WRE_ETY_END_RNDBLE  -93 // bool
#define KEY_WRE_FIXEDPROJLENGTH -94 // bool
#define KEY_WRE_PROJLENGTH      -95 // wmfloat
#define KEY_WRE_NORMAL          -96 // 3 wmfloats
#define KEY_WRE_CENTER          -97 // 3 wmfloats
#define KEY_WRE_BZR_POINTSBYPAIR -98 // n * wmfloat
#define KEY_WRE_BZR_OFFSET      -99 // 3 wmfloat
#define KEY_WRE_SEGMENTS         -100 // subelement
#define KEY_WRE_SEGMENTSCOUNT  -101 // int32
#define KEY_WRE_SEGMENT         -102 // subelement
#define KEY_WRE_SEGMENT_RAWPARAM1 -103 // wmfloat
#define KEY_WRE_SEGMENT_RAWPARAM2 -104 // wmfloat
#define KEY_WRE_SEGMENT_COMPLEMENT -105 // int32
/////////////////////// read only keys
#define KEY_SUBELT_KEY_START              -126 // char
#define KEY_SUBELT_KEY_END              -127 // char

class KeysPersist : public Storage
{
public:
    KeysPersist();
    virtual ~KeysPersist();

    int32_t WriteKeyData(char key, bool bValue);
    int32_t WriteKeyData(char key, int32_t iValue);
    int32_t WriteKeyData(char key, double dValue);
    int32_t WriteKeyData(char key, std::string & sValue);
    int32_t WriteKeyData(char key, int32_t * iValueArray, int nElems);
    int32_t WriteKeyData(char key, char * cValueArray, int nElems);
    int32_t WriteKeyData(char key, float * bValueArray, int nElems);
    int32_t WriteKeyData(char key, double * bValueArray, int nElems);

    void StartSubElement(char key);
    void EndSubElement();

protected:
    virtual void DoUpdateFileHeader();

    int32_t countWriteKeyOperations() const;

    virtual void WriteData(void * p, size_t size, size_t count);

private:
    int32_t m_countLevelZeroKeys;

    int32_t WriteKey(char key);
    int32_t WriteDataType(char keyDataType);
    int32_t WriteArrayElementsCount(int32_t count);

    int m_iSubElementIndex;
    std::vector<std::vector<char> > m_subElements;
    std::vector<char> * m_pSubElt;
};

class KeysLoad : public Storage
{
public:
    KeysLoad();
    virtual ~KeysLoad();

protected:
    virtual void DoUpdateFileHeader();

    void ReadAllKeys();

    virtual void LoadCharForKey(char key, char cVal) = 0;
    virtual void LoadStringForKey(char key, std::string & str) = 0;
    virtual void LoadInt32ForKey(char key, int32_t iVal) = 0;
    virtual void LoadBoolForKey(char key, bool bVal) = 0;
    virtual void LoadFloatForKey(char key, float fVal) = 0;
    virtual void LoadDoubleForKey(char key, double fVal) = 0;
    virtual void LoadCharArrayForKey(char key, char * pcVal, int32_t nElems) = 0;
    virtual void LoadInt32ArrayForKey(char key, int32_t * piVal, int32_t nElems) = 0;
    virtual void LoadFloatArrayForKey(char key, float * pfVal, int32_t nElems) = 0;
    virtual void LoadDoubleArrayForKey(char key, double * pdVal, int32_t nElems) = 0;

    virtual void StartSubElement(int32_t nElems);
    virtual void EndSubElement();

    virtual void ReadData(void * p, size_t size, size_t count);
    
private:
    std::string m_tmpString;
    std::vector<double> m_tmpDoubles;
    std::vector<float> m_tmpFloats;
    std::vector<char> m_tmpChars;
    std::vector<int32_t> m_tmpInts32;
    
    int m_iCurReadSubElementLevel;// -1 : normal (file) / n : nth level subelement 
    std::vector<char> m_firstLevelSubElement;
    char * m_firstLevelSubElementDataIt;
    int m_controlSizeAfterIt;

    int32_t ReadKeysCount();
    char ReadNextKey();
    char ReadNextDataType();
    int32_t ReadNextElementsCount();
    void ReadNextCharArrayAsString(int32_t nChars);
    void ReadNextCharArray(int32_t nChars);
    void ReadNextInt32Array(int32_t nChars);
    void ReadNextDoubleArray(int32_t nChars);
    void ReadNextFloatArray(int32_t nChars);
};
