
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
#define DATA_TYPE_STRING_ARRAY   'r'

using namespace imajuscule;

static bool keyReadOnly(char key)
{
    switch(key)
    {
        case KEY_SUBELT_KEY_START:
        case KEY_SUBELT_KEY_END:
            return true;
        default:
            return false;
    }
}

KeysPersist::KeysPersist(DirectoryPath const &d, FileName const &f) :
WritableStorage(d, f),
m_countLevelZeroKeys(0)
{
    m_curSubElt = m_subElements.begin();
}

void KeysPersist::StartSubElement(char key)
{
    WriteKey(key, false);
    WriteDataType(DATA_TYPE_SUBELT_AS_CHAR_ARRAY);

    // it's important to keep the incrementation after the previous Write* calls
    m_subElements.push_front(SubElement());
    m_curSubElt = m_subElements.begin();
}

int KeysPersist::WriteData(void const * p, size_t size, size_t count)
{
    if (m_curSubElt == m_subElements.end())
    {
        return WritableStorage::WriteData(p, size, count);
    }
    else
    {
        size_t add = size*count;
        m_curSubElt->content.insert(m_curSubElt->content.end(), (unsigned char*)p, ((unsigned char*)p) + add);
        return add;
    }
}


void KeysPersist::EndSubElement()
{
    Assert(!m_subElements.empty());

    std::vector<char> * pFinishedSubElt = &m_curSubElt->content;

    m_curSubElt++;
        
    int32_t sizeSubElement = safe_cast<int32_t>(pFinishedSubElt->size());

    WriteArrayElementsCount(sizeSubElement);

    if ( sizeSubElement > 0 ) {
        WriteData((void*)pFinishedSubElt->data(), sizeSubElement, 1);
    }

    m_subElements.pop_front();
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
    doReadData(&n, sizeof(int32_t), 1);

    //LG(INFO, "KeysLoad::ReadKeysCount returns %d", n);
    return n;
}


int32_t KeysPersist::countWriteKeyOperations() const
{
    return m_countLevelZeroKeys;
}

int32_t KeysPersist::WriteKey(char key, bool bCheckUnicity)
{
    Assert(!keyReadOnly(key));
    
    int32_t size = WriteData(&key, sizeof(char), 1);
    
    if ( m_curSubElt == m_subElements.end() )
    {
        m_countLevelZeroKeys++;

        // check that no key in rootelement is written twice

        if(bCheckUnicity)
        {
            auto it = rootKeys.find(key);
            Assert( it == rootKeys.end());
        }
        rootKeys.insert(key);
    }
    else
    {
        // check that no key in subelement is written twice

        if(bCheckUnicity)
        {
            auto it = m_curSubElt->keys.find(key);
            Assert( it == m_curSubElt->keys.end());
        }
        m_curSubElt->keys.insert(key);
    }
    
    return size;
}

char KeysLoad::ReadNextKey()
{
    //LG(INFO, "KeysLoad::ReadNextKey begin");

    char key;
    doReadData(&key, sizeof(char), 1);

    //LG(INFO, "KeysLoad::ReadNextKey returns %d", key);
    return key;
}

int32_t KeysPersist::WriteDataType(char keyDataType)
{
    int32_t size = sizeof(keyDataType);

    return WriteData(&keyDataType, size, 1);
}
char KeysLoad::ReadNextDataType()
{
    //LG(INFO, "KeysLoad::ReadNextDataType begin");

    char dataType;
    doReadData(&dataType, sizeof(dataType), 1);

    //LG(INFO, "KeysLoad::ReadNextDataType returns %d", dataType);
    return dataType;
}

int32_t KeysPersist::WriteArrayElementsCount(int32_t count)
{
    int32_t size = sizeof(count);
    

    return WriteData(&count, size, 1);
}
int32_t KeysLoad::ReadNextElementsCount()
{
    //LG(INFO, "KeysLoad::ReadNextElementsCount begin");

    int32_t count;
    doReadData(&count, sizeof(count), 1);

    //LG(INFO, "KeysLoad::ReadNextElementsCount returns %d", count);
    return count;
}

int32_t KeysPersist::WriteKeyData(char key, const std::string & sValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (string)%s )", key, sValue.c_str());

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_STRING_AS_CHAR_ARRAY);
    int32_t nElems = safe_cast<int32_t>(sValue.size());
    WriteSize += WriteArrayElementsCount(nElems);

    WriteSize += WriteData((void*)sValue.c_str(), nElems * sizeof(char), 1);

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, bool bValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (bool)%s )", key, (bValue ? "true" : "false"));

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_BOOL);

    char val = (bValue ? 0x01 : 0x00);
    
    WriteSize += WriteData(&val, sizeof(char), 1);

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, int32_t iValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (int)%d )", key, iValue);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_INT32);
    
    WriteSize += WriteData(&iValue, sizeof(int32_t), 1);

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, double dValue)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, (double)%f )", key, dValue);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_DOUBLE);

    WriteSize += WriteData(&dValue, sizeof(double), 1);

    //LG(INFO, "KeysPersist::Write returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, int32_t * iValueArray, size_t nElems)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);
    
    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_INT32_ARRAY);
    WriteSize += WriteArrayElementsCount((int32_t)nElems);
    
    int32_t size = (int32_t) nElems * sizeof(int32_t);
    WriteSize += WriteData((void*)iValueArray, size, 1);
    
    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, char * cValueArray, size_t nElems)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);
    
    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_CHAR_ARRAY);
    WriteSize += WriteArrayElementsCount((int32_t)nElems);
    
    int32_t size = (int32_t) nElems * sizeof(char);
    
    WriteSize += WriteData((void*)cValueArray, size, 1);;
    
    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, const std::vector<std::string> & sValueArray)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);

    size_t nElems = sValueArray.size();
    
    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_STRING_ARRAY);
    WriteSize += WriteArrayElementsCount((int32_t)nElems);
    
    for(auto const & s : sValueArray )
    {
        int32_t sizeWithEnd = (int32_t) (s.size()+1) * sizeof(char);
        
        WriteSize += WriteData((void*)s.c_str(), sizeWithEnd, 1);
    }
    
    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, double * dValueArray, size_t nElems)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_DOUBLE_ARRAY);
    WriteSize += WriteArrayElementsCount((int32_t)nElems);

    int32_t size = (int32_t) nElems * sizeof(double);
    
    WriteSize += WriteData((void*)dValueArray, size, 1);

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}

int32_t KeysPersist::WriteKeyData(char key, float * bValueArray, size_t nElems)
{
    //LG(INFO, "KeysPersist::WriteKeyData( %d, ..., (nElems:)%x )", key, nElems);

    int32_t WriteSize = WriteKey(key);
    WriteSize += WriteDataType(DATA_TYPE_FLOAT_ARRAY);
    WriteSize += WriteArrayElementsCount((int32_t)nElems);

    // write data
    int32_t size = (int32_t) nElems * sizeof(float);
    
    WriteSize += WriteData((void*)bValueArray, size, 1);

    //LG(INFO, "KeysPersist::WriteKeyData returns %d", WriteSize);
    return WriteSize;
}


KeysLoad::KeysLoad(DirectoryPath d, FileName f,bool bExhaustive) :
WritableStorage(d, f),
m_iCurReadSubElementLevel(-1),
m_firstLevelSubElementDataIt(nullptr),
m_controlSizeAfterIt(0)
, m_bExhaustive(bExhaustive)
{

}


void KeysLoad::doReadData(void * p, size_t size, size_t count)
{
    if (-1 == m_iCurReadSubElementLevel)
    {
        WritableStorage::ReadData(p, size, count);
    }
    else
    {
        Assert(m_iCurReadSubElementLevel >= 0);
        Assert(m_firstLevelSubElementDataIt);
        
        size_t add = size*count;

        m_controlSizeAfterIt -= add;
        Assert(m_controlSizeAfterIt >= 0);

        memcpy(p, m_firstLevelSubElementDataIt, add);
        m_firstLevelSubElementDataIt += add;
    }
}

void KeysLoad::StartSubElement(int32_t nElems)
{
    Assert(m_iCurReadSubElementLevel >= -1);

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
    Assert(m_iCurReadSubElementLevel >= 0);
    
    Assert(m_firstLevelSubElementDataIt);

    m_iCurReadSubElementLevel--;

    if (m_iCurReadSubElementLevel == -1)
    {
        Assert(m_controlSizeAfterIt == 0);
        m_firstLevelSubElement.clear();
        m_firstLevelSubElementDataIt = nullptr;
    }    
}

eResult KeysLoad::ReadAllKeys()
{
    eResult ret = OpenForRead();
    if ( unlikely(ret != ILE_SUCCESS))
    {
        LG(ERR, "PathSuiteLoad::Load : OpenForRead returned %d", ret);
        return ret;
    }
    
    ReadAllKeysInternal();
    
    onLoadFinished();
    
    return ILE_SUCCESS;
}

void KeysLoad::ReadAllKeysInternal()
{
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
            if ( likely(nElems >= 0))
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
            if ( likely(nElems >= 0))
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
            if ( likely(nElems >= 0))
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
                if ( likely(nElems >= 0))
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
                if (likely(nElems >= 0))
                {
                    StartSubElement(nElems);

                    LoadCharForKey(KEY_SUBELT_KEY_START, key);
                    
                    ReadAllKeysInternal();
                    
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
                if (likely(nElems >= 0))
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
            
            case DATA_TYPE_STRING_ARRAY:
            {
                int32_t nElems = ReadNextElementsCount();
                if (likely(nElems >= 0))
                {
                    ReadNextStringArray(nElems);
                    LoadStringArrayForKey(key, m_tmpStrings);
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
            doReadData(&iVal, sizeof(iVal), 1);
            LoadInt32ForKey(key, iVal);
            break;
        }

        case DATA_TYPE_BOOL:
        {
            char cVal;
            doReadData(&cVal, sizeof(cVal), 1);
            bool bVal = false;
            if (cVal)
                bVal = true;
            LoadBoolForKey(key, bVal);
            break;
        }

        case DATA_TYPE_DOUBLE:
        {
            double dVal;
            doReadData(&dVal, sizeof(dVal), 1);
            LoadDoubleForKey(key, dVal);
            break;
        }

        case DATA_TYPE_FLOAT:
        {
            float fVal;
            doReadData(&fVal, sizeof(fVal), 1);
            LoadFloatForKey(key, fVal);
            break;
        }

        case DATA_TYPE_CHAR:
        {
            char cVal;
            doReadData(&cVal, sizeof(cVal), 1);
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

    doReadData((void*)m_tmpString.data(), sizeof(char), nChars);

    //LG(INFO, "KeysLoad::ReadNextCharArrayAsString returns");
}


void KeysLoad::ReadNextCharArray(int32_t nChars)
{
    //LG(INFO, "KeysLoad::ReadNextCharArray(%d)", nChars);
    
    m_tmpChars.resize(nChars);
    
    doReadData((void*)m_tmpChars.data(), sizeof(char), nChars);
    
    //LG(INFO, "KeysLoad::ReadNextCharArray returns");
}

void KeysLoad::ReadNextInt32Array(int32_t nInts)
{
    //LG(INFO, "KeysLoad::ReadNextInt32Array(%d)", nInts);
    
    m_tmpInts32.resize(nInts);
    
    doReadData((void*)m_tmpInts32.data(), sizeof(int32_t), nInts);
    
    //LG(INFO, "KeysLoad::ReadNextInt32Array returns");
}
void KeysLoad::ReadNextStringArray(int32_t nElts)
{
    //LG(INFO, "KeysLoad::ReadNextStringArray(%d)", nElts);
    
    m_tmpStrings.clear();

    m_tmpStrings.resize(nElts);
    
    for(auto & s : m_tmpStrings)
    {
        char c;
        do
        {
            doReadData((void*)&c, sizeof(char), 1);
            if(c) {
                s.push_back(c);
            }
        }
        while (c); // assume string ends with 0
    }
    
    //LG(INFO, "KeysLoad::ReadNextStringArray returns");
}

void KeysLoad::ReadNextDoubleArray(int32_t nElems)
{
    //LG(INFO, "KeysLoad::ReadNextDoubleArray(%d)", nElems);

    m_tmpDoubles.resize(nElems);
    //LG(INFO, "KeysLoad::ReadNextDoubleArray after resize");
    doReadData((void*)m_tmpDoubles.data(), sizeof(double), nElems);

    //LG(INFO, "KeysLoad::ReadNextDoubleArray end");
}

void KeysLoad::ReadNextFloatArray(int32_t nElems)
{
    //LG(INFO, "KeysLoad::ReadNextFloatArray(%d)", nElems);

    m_tmpFloats.resize(nElems);
    doReadData((void*)m_tmpFloats.data(), sizeof(float), nElems);

    //LG(INFO, "KeysLoad::ReadNextFloatArray end");
}

void KeysLoad::LoadDoubleArrayForKey(char key, double * pdVal, size_t nElems){
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadDoubleArrayForKey(%d, ..., %d) should not be called", key, nElems);
        Assert(0);
    }
}
void KeysLoad::LoadBoolForKey(char key, bool bVal){
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadBoolForKey(%d, %s) should not be called", key, bVal?"true":"false");
        Assert(0);
    }
}
void KeysLoad::LoadStringForKey(char key, std::string & str){
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadStringForKey(%d, %s) should not be called", key, str.c_str() ? str.c_str() : "nullptr");
        Assert(0);
    }
}
void KeysLoad::LoadInt32ForKey(char key, int32_t iVal){
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadInt32ForKey(%d, %d) should not be called", key, iVal);
        Assert(0);
    }
}
void KeysLoad::LoadDoubleForKey(char key, double fVal){
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadDoubleForKey(%d, %d) should not be called", key, fVal);
        Assert(0);
    }
}
void KeysLoad::LoadCharArrayForKey(char key, char * /*pcVal*/, size_t nElems) {
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadCharArrayForKey(%d, ..., %d) should not be called", key, nElems);
        Assert(0);
    }
}
void KeysLoad::LoadInt32ArrayForKey(char key, int32_t * /*piVal*/, size_t nElems) {
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadInt32ArrayForKey(%d, ..., %d) should not be called", key, nElems);
        Assert(0);
    }
}
void KeysLoad::LoadStringArrayForKey(char key, const std::vector<std::string> &)
{
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadStringArrayForKey(%d, ...) should not be called", key);
        Assert(0);
    }
}
void KeysLoad::LoadFloatArrayForKey(char key, float * /*pfVal*/, size_t nElems) {
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadFloatArrayForKey(%d, ..., %d) should not be called", key, nElems);
        Assert(0);
    }
}
void KeysLoad::LoadCharForKey(char key, char cVal) {
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadCharForKey(%d, %d) should not be called", key, cVal);
        Assert(0);
    }
}
void KeysLoad::LoadFloatForKey(char key, float fVal) {
    if(m_bExhaustive)
    {
        LG(ERR, "KeysLoad::LoadFloatForKey(%d, %f) should not be called", key, fVal);
        Assert(0);
    }
}
