#pragma once

#include <bango/processor/lisp.h>
#include <bango/processor/file.h>

#include <cstdint>

namespace bango { namespace processor {

    class XParser
    {
    public:
        enum TOKEN
        {
            T_END, T_STRING, T_INTEGER, T_FLOAT, T_OPEN, T_CLOSE, T_ERROR
        };

        enum FLAG
        {
            F_NUMBER, F_INTEGER
        };

        typedef	std::intptr_t(*PARSE_CALLBACK)(std::intptr_t dwParam, lisp::var var);

    protected:
        XFile * m_pFile;
        int		m_nLine;
        int		m_nDepth;
        int		m_nPosition;
        std::string	    m_strSymbol;
        std::uint64_t	m_dwFlags;


    public:
        XParser(void);
        ~XParser(void);
        void	Open(XFile *pFile);
        lisp::var Load(const char* szPath);
        lisp::var Load(XFile *pFile);
        int	GetLine() { return m_nLine; }
        int	GetDepth() { return m_nDepth; }

        TOKEN	GetToken();
        const char* GetString() { return m_strSymbol.c_str(); }
        int		GetInteger() { return strtol(m_strSymbol.c_str(), 0, 0); }
        float	GetFloat() { return (float)atof(m_strSymbol.c_str()); }
        std::intptr_t	ParseList(PARSE_CALLBACK Callback, std::intptr_t dwParam);
        void	ResetDepth() { m_nDepth = 0; }

    protected:
        lisp::var OnLoad();

    };

}}