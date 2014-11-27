#include "os.storage.keys.h"
#include "os.log.h"
#include <cassert>

std::vector<char> fake;

KeysPersist::KeysPersist() :
Storage(),
m_countLevelZeroKeys(0),
m_iSubElementIndex(-1),
m_pSubElt(NULL)
{
}

KeysPersist::~KeysPersist()
{
}

void KeysPersist::StartSubElement(char key)
{
    assert(m_iSubElementIndex >= -1);

    WriteKey(key);
    WriteDataType(DATA_TYPE_SUBELT_AS_CHAR_ARRAY);

    // it's important to keep the incrementation after the previous Write* calls
    m_iSubElementIndex++;
    m_subElements.resize(m_iSubElementIndex + 1);
    m_pSubElt = &(m_subElements[m_iSubElementIndex]);
}

void KeysPersist::WriteData(void * p, size_t size, size_t count)
{
    if (-1 == m_iSubElementIndex)
    {
        Storage::WriteData(p, size, count);
    }
    else
    {
        assert(m_iSubElementIndex >= 0);
        assert(m_pSubElt);

        size_t add = size*count;
        m_pSubElt->insert(m_pSubElt->end(), (unsigned char*)p, ((unsigned char*)p) + add);
    }
}


void KeysPersist::EndSubElement()
{
    assert(m_iSubElementIndex >= 0);

    std::vector<char> * pFinishedSubElt = m_pSubElt;

    assert(pFinishedSubElt);

    m_iSubElementIndex--;
    if ( m_iSubElementIndex >= 0)
        m_pSubElt = &m_subElements[m_iSubElementIndex];
    
    int32_t sizeSubElement = pFinishedSubElt->size();

    WriteArrayElementsCount(sizeSubElement);

    if ( sizeSubElement > 0 )
        WriteData((void*)pFinishedSubElt->data(), sizeSubElement, 1);

    pFinishedSubElt->clear();
}

void KeysPersist::DoUpdateFileHeader()
{
    int32_t wk = countWriteKeyOperations();
    WriteData(&wk, sizeof(int32_t), 1);
}
int32_t KeysLoad::ReadKeysCount()
{
    //LG(INFO, "KeysLoad::ReadKeysCount begin");

    int32_t n;
    ReadData(&n, sizeof(int32_t), 1);

    //LG(INFO, "KeysLoad::ReadKeysCount returns %d", n);
    return n;
}


int32_t KeysPersist::countWriteKeyOperations() const
{
    return m_countLevelZeroKeys;
}

int32_t KeysPersist::WriteKey(char key)
{
    int32_t size = sizeof(char);
    WriteData(&key, size, 1);
    
    if ( m_iSubElementIndex == -1 )
        m_countLevelZeroKeys++;
    
    return size;
}

char KeysLoad::ReadNextKey()
{
    //LG(INFO, "KeysLoad::ReadNextKey begin");

    char key;
    ReadData(&key, sizeof(char), 1);

    //LG(INFO, "KeysLoad::ReadNextKey returns %d", key);
    return key;
}

int32_t KeysPersist::WriteDataType(char keyDataType)
{
    //LG(INFO, "KeysLoad::WriteDataType begin");

    int32_t size = sizeof(keyDataType);
    WriteData(&keyDataType, size, 1);

    //LG(INFO, "KeysLoad::WriteDataType returns %d", size);
    return size;
}
char KeysLoad::ReadNextDataType()
{
    //LG(INFO, "KeysLoad::ReadNextDataType begin");

    char dataType;
    ReadData(&dataType, sizeof(dataType), 1);

    //LG(INFO, "KeysLoad::ReadNextDataType returns %d", dataType);
    return dataType;
}

int32_t KeysPersist::WriteArrayElementsCount(int32_t count)
{
    //LG(INFO, "KeysLoad::WriteArrayElementsCount(%d)", count);

    int32_t size = sizeof(count);
    WriteData(&count, size, 1);

    //LG(INFO, "KeysLoad::WriteArrayElementsCount returns %d", size);
    return size;
}
int32_t KeysLoad::ReadNextElementsCount()
{
    //LG(INFO, "KeysLoad::ReadNextElementsCount begin");

    int32_t count;
    ReadData(&count, sizeof(count), 1);

    //LG(INFO, "KeysLoad::ReadNextElementsCount returns %d", count);
    return count;
}

int32_t KeysPersist::WriteKeyData(char key, std::string & sValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (string)%s )", key, sValue.c_str());

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_STRING_AS_CHAR_ARRAY);
    int32_t nElems = sValue.size();
    WriteSize += WriteArrayElementsCount(nElems);

    WriteData((void*)sValue.c_str(), nElems * sizeof(char), 1);
    WriteSize += nElems * sizeof(char);

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, bool bValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (bool)%s )", key, (bValue ? "true" : "false"));

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_BOOL);

    int32_t size = sizeof(char);
    char val = (bValue ? 0x01 : 0x00);
    WriteData(&val, size, 1);
    WriteSize += size;

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, int32_t iValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (int)%d )", key, iValue);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_INT32);

    int32_t size = sizeof(int32_t);
    WriteData(&iValue, size, 1);
    WriteSize += size;

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, double dValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (double)%f )", key, dValue);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_DOUBLE);

    int32_t size = sizeof(double);
    WriteData(&dValue, size, 1);
    WriteSize += size;

    //LG(INFO, "KeysPersist::Write returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, char * cValueArray, int nElems)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_CHAR_ARRAY);
    WriteSize += WriteArrayElementsCount(nElems);

    int32_t size = nElems * sizeof(double);
    WriteData((void*)cValueArray, size, 1);
    WriteSize += size;

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, double * dValueArray, int nElems)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_DOUBLE_ARRAY);
    WriteSize += WriteArrayElementsCount(nElems);

    int32_t size = nElems * sizeof(double);
    WriteData((void*)dValueArray, size, 1);
    WriteSize += size;

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, float * bValueArray, int nElems)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_FLOAT_ARRAY);
    WriteSize += WriteArrayElementsCount(nElems);

    // write data
    int32_t size = nElems * sizeof(float);
    WriteData((void*)bValueArray, size, 1);
    WriteSize += size;

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}


KeysLoad::KeysLoad() :
Storage(),
m_iCurReadSubElementLevel(-1),
m_firstLevelSubElementDataIt(NULL),
m_controlSizeAfterIt(0)
{

}

KeysLoad::~KeysLoad()
{
}


void KeysLoad::ReadData(void * p, size_t size, size_t count)
{
    if (-1 == m_iCurReadSubElementLevel)
    {
        Storage::ReadData(p, size, count);
    }
    else
    {
        assert(m_iCurReadSubElementLevel >= 0);
        assert(m_firstLevelSubElementDataIt);
        
        size_t add = size*count;

        m_controlSizeAfterIt -= add;
        assert(m_controlSizeAfterIt >= 0);

        memcpy(p, m_firstLevelSubElementDataIt, add);
        m_firstLevelSubElementDataIt += add;
    }
}

void KeysLoad::StartSubElement(int32_t nElems)
{
    assert(m_iCurReadSubElementLevel >= -1);

    if (m_iCurReadSubElementLevel == -1)
    {
        ReadNextCharArray(nElems);

        m_firstLevelSubElement.swap(m_tmpChars);
        m_firstLevelSubElementDataIt = m_firstLevelSubElement.data();
     
        m_controlSizeAfterIt = m_firstLevelSubElement.size();
    }

    // it's important to keep incrementation AFTER call to ReadCharArray
    m_iCurReadSubElementLevel++;
}
void KeysLoad::EndSubElement()
{
    assert(m_iCurReadSubElementLevel >= 0);
    
    assert(m_firstLevelSubElementDataIt);

    m_iCurReadSubElementLevel--;

    if (m_iCurReadSubElementLevel == -1)
    {
        assert(m_controlSizeAfterIt == 0);
        m_firstLevelSubElement.clear();
        m_firstLevelSubElementDataIt = NULL;
    }    
}

void KeysLoad::DoUpdateFileHeader()
{
    assert(0);
}

void KeysLoad::ReadAllKeys()
{
    //LG(INFO, "KeysLoad::ReadAllKeys begin");

    int32_t nKeys;
    if (m_iCurReadSubElementLevel == -1)
    {
        nKeys = ReadKeysCount();
    }
    else
    {
        nKeys = -1;// the value is not important, it's never read
    }

    for (int32_t i = 0; ((m_iCurReadSubElementLevel == -1) && (i < nKeys)) || (m_controlSizeAfterIt!=0); i++)
    {
        char key = ReadNextKey();
        char dataType = ReadNextDataType();

        switch (dataType)
        {
        case DATA_TYPE_STRING_AS_CHAR_ARRAY:
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
                
            case DATA_TYPE_CHAR_ARRAY:
            {
                int32_t nElems = ReadNextElementsCount();
                if (nElems >= 0)
                {
                    ReadNextCharArray(nElems);
                    LoadCharArrayForKey(key, m_tmpChars.data(), nElems);
                }
                else
                {
                    LG(ERR, "KeysLoad::ReadAllKeys abort because found negative count of elements : %x", nElems);
                    break;
                }
                break;
            }
                
            case DATA_TYPE_SUBELT_AS_CHAR_ARRAY:
            {
                int32_t nElems = ReadNextElementsCount();
                if (nElems >= 0)
                {
                    StartSubElement(nElems);

                    LoadCharForKey(KEY_SUBELT_KEY_START, key);
                    
                    ReadAllKeys();
                    
                    LoadCharForKey(KEY_SUBELT_KEY_END, key);

                    EndSubElement();
                }
                else
                {
                    LG(ERR, "KeysLoad::ReadAllKeys abort because found negative count of elements : %x", nElems);
                    break;
                }
                break;
            }
                
            case DATA_TYPE_INT32_ARRAY:
            {
                int32_t nElems = ReadNextElementsCount();
                if (nElems >= 0)
                {
                    ReadNextInt32Array(nElems);
                    LoadInt32ArrayForKey(key, m_tmpInts32.data(), nElems);
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

        case DATA_TYPE_BOOL:
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
            LG(ERR, "KeysLoad::ReadAllKeys : unhandled dataType %d", dataType);
            break;
        }
    }

    //LG(INFO, "KeysLoad::ReadAllKeys end");
}

void KeysLoad::ReadNextCharArrayAsString(int32_t nChars)
{
    //LG(INFO, "KeysLoad::ReadNextCharArrayAsString(%d)", nChars);

    m_tmpString.resize(nChars);

    ReadData((void*)m_tmpString.data(), sizeof(char), nChars);

    //LG(INFO, "KeysLoad::ReadNextCharArrayAsString returns");
}


void KeysLoad::ReadNextCharArray(int32_t nChars)
{
    //LG(INFO, "KeysLoad::ReadNextCharArray(%d)", nChars);
    
    m_tmpChars.resize(nChars);
    
    ReadData((void*)m_tmpChars.data(), sizeof(char), nChars);
    
    //LG(INFO, "KeysLoad::ReadNextCharArray returns");
}

void KeysLoad::ReadNextInt32Array(int32_t nInts)
{
    //LG(INFO, "KeysLoad::ReadNextInt32Array(%d)", nInts);
    
    m_tmpInts32.resize(nInts);
    
    ReadData((void*)m_tmpInts32.data(), sizeof(int32_t), nInts);
    
    //LG(INFO, "KeysLoad::ReadNextInt32Array returns");
}

void KeysLoad::ReadNextDoubleArray(int32_t nElems)
{
    //LG(INFO, "KeysLoad::ReadNextDoubleArray(%d)", nElems);

    m_tmpDoubles.resize(nElems);
    //LG(INFO, "KeysLoad::ReadNextDoubleArray after resize");
    ReadData((void*)m_tmpDoubles.data(), sizeof(double), nElems);

    //LG(INFO, "KeysLoad::ReadNextDoubleArray end");
}

void KeysLoad::ReadNextFloatArray(int32_t nElems)
{
    //LG(INFO, "KeysLoad::ReadNextFloatArray(%d)", nElems);

    m_tmpFloats.resize(nElems);
    ReadData((void*)m_tmpFloats.data(), sizeof(float), nElems);

    //LG(INFO, "KeysLoad::ReadNextFloatArray end");
}
