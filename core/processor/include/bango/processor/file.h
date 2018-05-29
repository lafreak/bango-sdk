#pragma once

#include <cstdlib>
#include <cstdint>

#include <sys/mman.h>

namespace bango { namespace processor {

    class XFile {
    public:
        enum SeekPosition {
            begin = 0x0,
            current = 0x1,
            end = 0x2
        };

        XFile();
        XFile(void *pFile, size_t size);
        ~XFile();

        void	Open(void *pVoid, size_t size);
        void	Close();

        std::uint32_t	Read(void *lpBuf, std::uint32_t uiCount);

        long	Seek(long lOffset, std::uint32_t nFrom);
        size_t	SeekToEnd() { return Seek(0L, XFile::end); }
        void	SeekToBegin() { Seek(0L, XFile::begin); }
        size_t	GetLength()	const { return (size_t)(m_pViewEnd - m_pViewBegin); }
        size_t	GetPosition() const { return (size_t)(m_pView - m_pViewBegin); }

        char*   _getPointerStart() { return m_pViewBegin; }
        char*   _getPointerEnd() { return m_pViewEnd; }

        char *	m_pView;
        char *	m_pViewBegin;
        char *	m_pViewEnd;
    };

    class XFileEx : public XFile {
    public:
        XFileEx();
        ~XFileEx();

        bool	Open(const char* szPath);
        void	Close();

        int     m_fd;
        void*   m_mapView;
        size_t  m_size;
        //HANDLE	m_hFile;
        //HANDLE	m_hMapping;
    };

}}