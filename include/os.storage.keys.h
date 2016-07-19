#pragma once
#include "os.storage.h"
#include "os.log.h"
#include <stdint.h>
#include <list>
#include <set>

/////////////////////// read only keys
#define KEY_SUBELT_KEY_START              -126 // char
#define KEY_SUBELT_KEY_END              -127 // char

namespace imajuscule
{
	class KeysPersist : public Storage
	{
	public:
		KeysPersist(DirectoryPath const &, FileName const &);
		virtual ~KeysPersist();

		int32_t WriteKeyData(char key, bool bValue);
		int32_t WriteKeyData(char key, int32_t iValue);
		int32_t WriteKeyData(char key, double dValue);
		int32_t WriteKeyData(char key, const std::string & sValue);
		int32_t WriteKeyData(char key, int32_t * iValueArray, size_t nElems);
		int32_t WriteKeyData(char key, char * cValueArray, size_t nElems);
		int32_t WriteKeyData(char key, std::vector<std::string> const &);
		int32_t WriteKeyData(char key, float * bValueArray, size_t nElems);
		int32_t WriteKeyData(char key, double * bValueArray, size_t nElems);

		void StartSubElement(char key);
		void EndSubElement();

	protected:
		void DoUpdateFileHeader() override;

		int32_t countWriteKeyOperations() const;

		void WriteData(void * p, size_t size, size_t count) override;

	private:
		int32_t m_countLevelZeroKeys;

		int32_t WriteKey(char key, bool bCheckUnicity = true);
		int32_t WriteDataType(char keyDataType);
		int32_t WriteArrayElementsCount(int32_t count);

		struct SubElement
		{
			std::vector<char> content;
			std::multiset<char> keys;
		};
		typedef std::list< SubElement > subElts;
		subElts m_subElements;
		subElts::iterator m_curSubElt;

		std::multiset<char> rootKeys;
	};

	class KeysLoad : public Storage
	{
	public:
		KeysLoad(DirectoryPath, FileName, bool bExhaustive = true);
		~KeysLoad();

		eResult ReadAllKeys();

	protected:

		virtual void LoadCharForKey(char key, char cVal);
		virtual void LoadStringForKey(char key, std::string & str);
		virtual void LoadInt32ForKey(char key, int32_t iVal);
		virtual void LoadBoolForKey(char key, bool bVal);
		virtual void LoadFloatForKey(char key, float fVal);
		virtual void LoadDoubleForKey(char key, double fVal);
		virtual void LoadStringArrayForKey(char key, const std::vector<std::string> &);
		virtual void LoadCharArrayForKey(char key, char * /*pcVal*/, size_t nElems);
		virtual void LoadInt32ArrayForKey(char key, int32_t * /*piVal*/, size_t nElems);
		virtual void LoadFloatArrayForKey(char key, float * pfVal, size_t nElems);
		virtual void LoadDoubleArrayForKey(char key, double * pdVal, size_t nElems);

		virtual void StartSubElement(int32_t nElems);
		virtual void EndSubElement();

		virtual void ReadData(void * p, size_t size, size_t count);

		virtual void onLoadFinished() {};
	private:
		std::string m_tmpString;
		std::vector<double> m_tmpDoubles;
		std::vector<float> m_tmpFloats;
		std::vector<char> m_tmpChars;
		std::vector<int32_t> m_tmpInts32;
		std::vector<std::string> m_tmpStrings;

		int m_iCurReadSubElementLevel;// -1 : normal (file) / n : nth level subelement 
		std::vector<char> m_firstLevelSubElement;
		char * m_firstLevelSubElementDataIt;
		long m_controlSizeAfterIt;

		bool m_bExhaustive;

		int32_t ReadKeysCount();
		char ReadNextKey();
		char ReadNextDataType();
		int32_t ReadNextElementsCount();
		void ReadNextCharArrayAsString(int32_t nChars);
		void ReadNextCharArray(int32_t nChars);
		void ReadNextInt32Array(int32_t nChars);
		void ReadNextStringArray(int32_t nElts);
		void ReadNextDoubleArray(int32_t nChars);
		void ReadNextFloatArray(int32_t nChars);

		void ReadAllKeysInternal();
	};
}
