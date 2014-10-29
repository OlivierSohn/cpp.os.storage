
#pragma once
#include "os.storage.h"
#include "os.log.h"

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

    virtual void LoadCharForKey(char key, char cVal) { LG(ERR, "KeysLoad : LoadCharForKey was not implemented by subClass"); }
    virtual void LoadStringForKey(char key, std::string & str) { LG(ERR, "KeysLoad : LoadStringForKey was not implemented by subClass"); }
    virtual void LoadInt32ForKey(char key, int32_t iVal) { LG(ERR, "KeysLoad : LoadInt32ForKey was not implemented by subClass"); }
    virtual void LoadBoolForKey(char key, bool bVal) { LG(ERR, "KeysLoad : LoadBoolForKey was not implemented by subClass"); }
    virtual void LoadFloatForKey(char key, float fVal) { LG(ERR, "KeysLoad : LoadFloatForKey was not implemented by subClass"); }
    virtual void LoadDoubleForKey(char key, double fVal) { LG(ERR, "KeysLoad : LoadDoubleForKey was not implemented by subClass"); }
    virtual void LoadFloatArrayForKey(char key, float * pfVal, int32_t nElems) { LG(ERR, "KeysLoad : LoadFloatArrayForKey was not implemented by subClass"); }
    virtual void LoadDoubleArrayForKey(char key, double * pdVal, int32_t nElems) { LG(ERR, "KeysLoad : LoadDoubleArrayForKey was not implemented by subClass"); }

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
