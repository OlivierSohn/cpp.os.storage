#include "os.storage.keys.h"
#include "os.log.h"
#include <cassert>

KeysPersist::KeysPersist() :
Storage(),
m_countWriteKeyOperations(0)
{

}

KeysPersist::~KeysPersist()
{
}

void KeysPersist::DoUpdateFileHeader()
{
    int32_t wk = countWriteKeyOperations();
    WriteData(&wk, sizeof(int32_t), 1);
}
int32_t KeysLoad::ReadKeysCount()
{
    int32_t n;
    ReadData(&n, sizeof(int32_t), 1);
    return n;
}


int32_t KeysPersist::countWriteKeyOperations() const
{
    return m_countWriteKeyOperations;
}

int32_t KeysPersist::WriteKey(char key)
{
    int32_t size = sizeof(char);
    WriteData(&key, size, 1);
    m_countWriteKeyOperations++;
    return size;
}

char KeysLoad::ReadNextKey()
{
    char key;
    ReadData(&key, sizeof(char), 1);
    return key;
}

int32_t KeysPersist::WriteDataType(char keyDataType)
{
    int32_t size = sizeof(keyDataType);
    WriteData(&keyDataType, size, 1);
    return size;
}
char KeysLoad::ReadNextDataType()
{
    char dataType;
    ReadData(&dataType, sizeof(dataType), 1);
    return dataType;
}

int32_t KeysPersist::WriteArrayElementsCount(int32_t count)
{
    int32_t size = sizeof(count);
    WriteData(&count, size, 1);
    return size;
}
int32_t KeysLoad::ReadNextElementsCount()
{
    int32_t count;
    ReadData(&count, sizeof(count), 1);
    return count;
}

int32_t KeysPersist::WriteKeyData(char key, std::string & sValue)
{
    LG(INFO, "KeysPersist::Write key:%c val:%s", key, sValue.c_str());

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_CHAR_ARRAY);
    int32_t nElems = sValue.size();
    WriteSize += WriteArrayElementsCount(nElems);

    WriteData((void*)sValue.c_str(), nElems * sizeof(char), 1);
    WriteSize += nElems * sizeof(char);

    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, bool bValue)
{
    LG(INFO, "KeysPersist::Write key:%c val:%s", key, (bValue ? "true" : "false"));

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_BOOL);

    int32_t size = sizeof(char);
    char val = (bValue ? 0x01 : 0x00);
    WriteData(&val, size, 1);
    WriteSize += size;

    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, int32_t iValue)
{
    LG(INFO, "KeysPersist::Write key:%c val:%d", key, iValue);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_INT32);

    int32_t size = sizeof(int32_t);
    WriteData(&iValue, size, 1);
    WriteSize += size;

    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, double dValue)
{
    LG(INFO, "KeysPersist::Write key:%c val:%f", key, dValue);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_DOUBLE);

    int32_t size = sizeof(double);
    WriteData(&dValue, size, 1);
    WriteSize += size;

    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, double * bValueArray, int nElems)
{
    LG(INFO, "KeysPersist::Write key:%c, %x elements", key, nElems);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_DOUBLE_ARRAY);
    WriteSize += WriteArrayElementsCount(nElems);

    int32_t size = nElems * sizeof(double);
    WriteData((void*)bValueArray, size, 1);
    WriteSize += size;

    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, float * bValueArray, int nElems)
{
    LG(INFO, "KeysPersist::Write key:%c, %x elements", key, nElems);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_FLOAT_ARRAY);
    WriteSize += WriteArrayElementsCount(nElems);

    // write data
    int32_t size = nElems * sizeof(float);
    WriteData((void*)bValueArray, size, 1);
    WriteSize += size;

    return WriteSize;
}


KeysLoad::KeysLoad() :
Storage()
{

}

KeysLoad::~KeysLoad()
{
}

void KeysLoad::DoUpdateFileHeader()
{
    assert(0);
}

void KeysLoad::ReadAllKeys()
{
    LG(INFO, "KeysLoad::ReadAllKeys begin");

    int32_t nKeys = ReadKeysCount();

    for (int32_t i = 0; i < nKeys; i++)
    {
        char key = ReadNextKey();
        char dataType = ReadNextDataType();

        switch (dataType)
        {
        case DATA_TYPE_CHAR_ARRAY:
        {
            int32_t nElems = ReadNextElementsCount();
            if (nElems >= 0)
            {
                ReadNextCharArrayAsString(nElems);
                LoadStringForKey(key, m_tmpString);
            }
            else
            {
                LG(ERR, "KeysLoad::ReadAllKeys abort because found negative count of elements : %x", nElems);
                break;
            }
            break;
        }

        case DATA_TYPE_DOUBLE_ARRAY:
        {
            int32_t nElems = ReadNextElementsCount();
            if (nElems >= 0)
            {
                ReadNextDoubleArray(nElems);
                LoadDoubleArrayForKey(key, m_tmpDoubles.data(), nElems);
            }
            else
            {
                LG(ERR, "KeysLoad::ReadAllKeys abort because found negative count of elements : %x", nElems);
                break;
            }
            break;
        }

        case DATA_TYPE_FLOAT_ARRAY:
        {
            int32_t nElems = ReadNextElementsCount();
            if (nElems >= 0)
            {
                ReadNextFloatArray(nElems);
                LoadFloatArrayForKey(key, m_tmpFloats.data(), nElems);
            }
            else
            {
                LG(ERR, "KeysLoad::ReadAllKeys abort because found negative count of elements : %x", nElems);
                break;
            }
            break;
        }

        case DATA_TYPE_INT32:
        {
            int32_t iVal;
            ReadData(&iVal, sizeof(iVal), 1);
            LoadInt32ForKey(key, iVal);
            break;
        }

        case DATA_TYPE_BOOL :
        {
            char cVal;
            ReadData(&cVal, sizeof(cVal), 1);
            bool bVal = false;
            if (cVal)
                bVal = true;
            LoadBoolForKey(key, bVal);
            break;
        }

        case DATA_TYPE_DOUBLE:
        {
            double dVal;
            ReadData(&dVal, sizeof(dVal), 1);
            LoadDoubleForKey(key, dVal);
            break;
        }

        case DATA_TYPE_FLOAT:
        {
            float fVal;
            ReadData(&fVal, sizeof(fVal), 1);
            LoadFloatForKey(key, fVal);
            break;
        }

        case DATA_TYPE_CHAR:
        {
            char cVal;
            ReadData(&cVal, sizeof(cVal), 1);
            LoadCharForKey(key, cVal);
            break;
        }

        default:
            LG(ERR, "KeysLoad::ReadAllKeys : unknown dataType %d, assuming it is not an array", dataType);
            break;
        }
    }

    LG(INFO, "KeysLoad::ReadAllKeys end");
}

void KeysLoad::ReadNextCharArrayAsString(int32_t nChars)
{
    LG(INFO, "KeysLoad::ReadNextString(%x) begin", nChars);

    m_tmpString.resize(nChars);

    ReadData((void*)m_tmpString.data(), sizeof(char), nChars);

    LG(INFO, "KeysLoad::ReadNextString(%x) read %s", nChars, m_tmpString.c_str());
}

void KeysLoad::ReadNextDoubleArray(int32_t nElems)
{
    m_tmpDoubles.resize(nElems);
    ReadData((void*)m_tmpDoubles.data(), sizeof(double), nElems);
}

void KeysLoad::ReadNextFloatArray(int32_t nElems)
{
    m_tmpFloats.resize(nElems);
    ReadData((void*)m_tmpFloats.data(), sizeof(float), nElems);
}
