#include <bango/processor/file.h>

#include <cstring>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>

using namespace bango::processor;

template <typename T> void	minimize(T &left, const T& right) { if (right < left) left = right; }

XFile::XFile()
{
	m_pViewBegin = 0;
	m_pView = 0;
	m_pViewEnd = 0;
}

XFile::XFile(void *pFile, size_t size)
{
	m_pView = 0;
	m_pViewEnd = 0;

	Open(pFile, size);
}

XFile::~XFile()
{
}

void XFile::Open(void *pVoid, size_t size)
{
	m_pViewBegin = (char *)pVoid;
	m_pView = m_pViewBegin;
	m_pViewEnd = m_pViewBegin + size;
}

std::uint32_t XFile::Read(void *lpBuf, std::uint32_t uiCount)
{
	minimize(uiCount, (std::uint32_t)(m_pViewEnd - m_pView));
	std::memcpy(lpBuf, m_pView, uiCount);
	m_pView += uiCount;
	return uiCount;
}

void XFile::Close()
{
	m_pView = 0;
	m_pViewEnd = 0;
}

long XFile::Seek(long lOffset, std::uint32_t nFrom)
{
	if (nFrom == begin)
		m_pView = m_pViewBegin + lOffset;
	else if (nFrom == current)
		m_pView += lOffset;
	else if (nFrom == end)
		m_pView = m_pViewEnd + lOffset;
	return (long)(m_pView - m_pViewBegin);
}

XFileEx::XFileEx()
{
	m_mapView = nullptr;
	m_fd = -1;
	m_size = 0;
}

XFileEx::~XFileEx()
{
	XFileEx::Close();
}

bool XFileEx::Open(const char* szPath)
{
    struct stat s;
    m_fd = ::open(szPath, O_RDONLY);

	if (m_fd == -1)
		return false;

	fstat(m_fd, &s);

	m_size = s.st_size;

	m_mapView = mmap(0, m_size, PROT_READ, MAP_PRIVATE, m_fd, 0);

    XFile::Open(m_mapView, m_size);
    return true;
}

void XFileEx::Close()
{
	if (m_fd != -1)
	{
		::close(m_fd);
		m_fd = -1;
	}

	if (m_mapView != nullptr)
	{
		munmap(m_mapView, m_size);
		m_mapView = nullptr;
		m_size = 0;
	}
}