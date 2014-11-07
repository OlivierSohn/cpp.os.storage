#pragma once
#include "os.storage.h"
#include "os.log.h"


// data types

#define DATA_TYPE_INT32        'a'
#define DATA_TYPE_BOOL         'b'
#define DATA_TYPE_DOUBLE       'd'
#define DATA_TYPE_DOUBLE_ARRAY 'D'
#define DATA_TYPE_FLOAT        'f'
#define DATA_TYPE_FLOAT_ARRAY  'F'
#define DATA_TYPE_CHAR         's'
#define DATA_TYPE_CHAR_ARRAY   'S'

// keys

#define KEY_IC_DQROT            'a' // 4 double
#define KEY_IC_DPOS             'b' // 3 double
#define KEY_IC_SET              'c' // bool
#define KEY_DATE_CREA           'd' // n * char
#define KEY_GUID                'e' // n * char
#define KEY_RAWPATH_GUID        'f' // n * char
#define KEY_INTPATH_GUID        'g' // n * char
#define KEY_REGPATH_GUID        'h' // n * char
#define KEY_NAME                'i' // n * char
#define KEY_ITGR_MODE           'j' // int32
#define KEY_FILTER_RATE         'k' // double
#define KEY_FILTER_CUTOFF       'l' // double
#define KEY_FILTER_ADAPT        'm' // bool
#define KEY_FPS                 'n' // int32
#define KEY_VEC_ACC_POS         'P' // n * 4 double
#define KEY_VEC_INT_POS         'p' // n * 7 double
#define KEY_VEC_ACC_ROT         'R' // n * 4 double
#define KEY_VEC_INT_ROT         'r' // n * 5 double
#define KEY_ACC_SEN_BIAS        's' // 3 double
#define KEY_ACC_SEN_MAX_GRAV    't' // 3 double
#define KEY_ACC_SEN_MIN_GRAV    'u' // 3 double
#define KEY_ACC_SEN_NOISE_AMP   'v' // 3 double

class KeysPersist : public Storage
{
public:
    KeysPersist();
    virtual ~KeysPersist();

protected:
    virtual void DoUpdateFileHeader();

    int32_t WriteKeyData(char key, bool bValue);
    int32_t WriteKeyData(char key, int32_t iValue);
    int32_t WriteKeyData(char key, double dValue);
    int32_t WriteKeyData(char key, std::string & sValue);
    int32_t WriteKeyData(char key, float * bValueArray, int nElems);
    int32_t WriteKeyData(char key, double * bValueArray, int nElems);

    int32_t countWriteKeyOperations() const;

private:
    int32_t m_countWriteKeyOperations;

    int32_t WriteKey(char key);
    int32_t WriteDataType(char keyDataType);
    int32_t WriteArrayElementsCount(int32_t count);
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
    virtual void LoadFloatArrayForKey(char key, float * pfVal, int32_t nElems) = 0;
    virtual void LoadDoubleArrayForKey(char key, double * pdVal, int32_t nElems) = 0;

private:
    std::string m_tmpString;
    std::vector<double> m_tmpDoubles;
    std::vector<float> m_tmpFloats;

    int32_t ReadKeysCount();
    char ReadNextKey();
    char ReadNextDataType();
    int32_t ReadNextElementsCount();
    void ReadNextCharArrayAsString(int32_t nChars);
    void ReadNextDoubleArray(int32_t nChars);
    void ReadNextFloatArray(int32_t nChars);
};
