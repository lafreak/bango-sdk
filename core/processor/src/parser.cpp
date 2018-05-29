#include <bango/processor/parser.h>

#include <cstring>
#include <cwctype>
#include <cassert>

#include <alloca.h>

using namespace bango::processor;

#define TRACE printf
#define ASSERT assert
#define _alloca alloca

XParser::XParser(void)
{

}

XParser::~XParser(void)
{
}

void	XParser::Open(XFile *pFile)
{
	m_pFile = pFile;
	m_nLine = 1;
	m_nDepth = 0;
}

lisp::var	XParser::Load(const char* szPath)
{
	XFileEx	file;
	if (!file.Open(szPath))
		return lisp::error;
	return Load(&file);
}

lisp::var	XParser::Load(XFile *pFile)
{
	m_pFile = pFile;
	m_nLine = 1;
	return OnLoad();
}

lisp::var	XParser::OnLoad()
{
	TOKEN token;
	lisp::var varList;
	lisp::var *pList = &varList;
	for (; ; ) {
		switch (token = GetToken()) {
			case T_END:
			case T_CLOSE:
				return varList;
			case T_OPEN:
			{
				lisp::var child = OnLoad();
				if (!child.listp()) {
					varList.destroy();
					return child;
				}
				*pList = new lisp::_cons(child, lisp::nil);
				pList = &pList->cdr();
			}
			break;
			case T_STRING:
			{
				char *p = (char *)malloc(sizeof(lisp::_string) + m_strSymbol.size() + 1);

				*pList = new lisp::_cons(new (p) lisp::_string(strcpy(p + sizeof(lisp::_string), GetString())), lisp::nil);
				pList = &pList->cdr();
			}
			break;
			case T_FLOAT:
			{
				*pList = new lisp::_cons(new lisp::_float(GetFloat()), lisp::nil);
				pList = &pList->cdr();
			}
			break;
			case T_INTEGER:
			{
				*pList = new lisp::_cons(new lisp::_integer(GetInteger()), lisp::nil);
				pList = &pList->cdr();
			}
			break;
			default:
				varList.destroy();
				TRACE("XParser::OnLoad(): Invalid format at line %d\n", GetLine());
				return lisp::error;
		}
	}
}

XParser::TOKEN		XParser::GetToken()
{
	int ch;

	{
	st_start:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_END;
		switch (ch = *m_pFile->m_pView++) {
			case 0:
				m_pFile->m_pView--;
				return T_END;
			case ';':	// comment
				for (; ; ) {
					if (m_pFile->m_pView == m_pFile->m_pViewEnd)
						return T_END;
					switch (*m_pFile->m_pView++) {
						case 0:
						case '\n':
							m_pFile->m_pView--;
							goto st_start;
					}
				}
			case '\n':
				m_nLine++;
				goto st_start;
			case '(':
#ifdef	GAMECONVERT
			case '{':
#endif
				m_nDepth++;
				return T_OPEN;
			case ')':
#ifdef	GAMECONVERT
			case '}':
#endif
				m_nDepth--;
				return T_CLOSE;
			case '"':
				m_strSymbol.clear();
				for (; ; ) {
					if (m_pFile->m_pView == m_pFile->m_pViewEnd)
						return T_ERROR;
					switch (ch = *m_pFile->m_pView++) {
						case 0:
							m_pFile->m_pView--;
							return T_ERROR;
						case '"':
							if (m_pFile->m_pView == m_pFile->m_pViewEnd)
								return T_STRING;
							if ((ch = *m_pFile->m_pView) != '"')
								return T_STRING;
							else
								m_strSymbol += '"';
							break;
						case '\r':
						case '\n':
							m_pFile->m_pView--;
							return T_ERROR;
						default:
							m_strSymbol += ch;
							break;
					}
				}
			case '\'':
				m_strSymbol.clear();
				for (; ; ) {
					if (m_pFile->m_pView == m_pFile->m_pViewEnd)
						return T_ERROR;
					switch (ch = *m_pFile->m_pView++) {
						case 0:
							m_pFile->m_pView--;
							return T_ERROR;
						case '\'':
							if (m_pFile->m_pView == m_pFile->m_pViewEnd)
								return T_STRING;
							if ((ch = *m_pFile->m_pView) != '\'')
								return T_STRING;
							else
								m_strSymbol += '\'';
							break;
						case '\r':
						case '\n':
							m_pFile->m_pView--;
							return T_ERROR;
						default:
							m_strSymbol += ch;
							break;
					}
				}
			case ' ':
			case '\t':
			case '\r':
				goto st_start;
			case '+':
			case '-':
				m_strSymbol.clear();
				m_strSymbol += ch;
				goto st_sign;
			case '0':
				m_strSymbol.clear();
				m_strSymbol += ch;
				goto st_zero;
			case '.':
				m_strSymbol.clear();
				m_strSymbol += ch;
				goto st_dot;
			case '*':
			case '/':
			case '_':
				m_strSymbol.clear();
				m_strSymbol += ch;
				goto st_string;
			default:
				if (iswdigit(ch)) {
					m_strSymbol.clear();
					m_strSymbol += ch;
					goto st_digit;
				}
				else if (iswalnum(ch)) {
					m_strSymbol.clear();
					m_strSymbol += ch;
					goto st_string;
				}
				return T_ERROR;
		}
	st_string:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_STRING;
		switch (ch = *m_pFile->m_pView++) {
			case '+':
			case '-':
			case '*':
			case '/':
			case '.':
			case '_':
#ifdef DEF_MD5_OEN_071107
			case '\\':
#endif
				m_strSymbol += ch;
				goto st_string;
			default:
				if (iswalnum(ch)) {
					m_strSymbol += ch;
					goto st_string;
				}
				m_pFile->m_pView--;
				return T_STRING;
		}
	st_sign:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_STRING;
		switch (ch = *m_pFile->m_pView++) {
			case '0':
				m_strSymbol += ch;
				goto st_zero;
			case '.':
				m_strSymbol += ch;
				goto st_dot;
			default:
				if (iswdigit(ch)) {
					m_strSymbol += ch;
					goto st_digit;
				}
				if (iswalnum(ch))
					return T_ERROR;
				m_pFile->m_pView--;
				return T_STRING;
		}
	st_zero:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_INTEGER;
		switch (ch = *m_pFile->m_pView++) {
			case 'x':
			case 'X':
				m_strSymbol += ch;
				goto st_hexa;
			case '8':
			case '9':
				return T_ERROR;
			case '.':
				m_strSymbol += ch;
				goto st_dot;
			default:
				if (iswdigit(ch)) {
					m_strSymbol += ch;
					goto st_octal;
				}
				if (iswalnum(ch))
					return T_ERROR;
				m_pFile->m_pView--;
				return T_INTEGER;
		}
	st_octal:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_INTEGER;
		switch (ch = *m_pFile->m_pView++) {
			case '8':
			case '9':
				return T_ERROR;
			default:
				if (iswdigit(ch)) {
					m_strSymbol += ch;
					goto st_octal;
				}
				if (iswalnum(ch))
					return T_ERROR;
				m_pFile->m_pView--;
				return T_INTEGER;
		}
	st_hexa:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_INTEGER;
		if (isxdigit(ch = *m_pFile->m_pView++)) {
			m_strSymbol += ch;
			goto st_hexa;
		}
		if (iswalnum(ch))
			return T_ERROR;
		m_pFile->m_pView--;
		return T_INTEGER;
	st_digit:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_INTEGER;
		switch (ch = *m_pFile->m_pView++) {
			case '.':
				m_strSymbol += ch;
				goto st_dot;
			case 'd':
			case 'D':
			case 'e':
			case 'E':
				m_strSymbol += ch;
				goto st_exp;
			default:
				if (iswdigit(ch)) {
					m_strSymbol += ch;
					goto st_digit;
				}
				if (iswalnum(ch))
					return T_ERROR;
				m_pFile->m_pView--;
				return T_INTEGER;
		}
	st_dot:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_ERROR;
		if (iswdigit(ch = *m_pFile->m_pView++)) {
			m_strSymbol += ch;
			goto st_dot_digit;
		}
		// 1.#QNAN0
		m_pFile->m_pView--;
		return T_ERROR;

	st_dot_digit:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_FLOAT;
		switch (ch = *m_pFile->m_pView++) {
			case 'd':
			case 'D':
			case 'e':
			case 'E':
				m_strSymbol += ch;
				goto st_exp;
			default:
				if (iswdigit(ch)) {
					m_strSymbol += ch;
					goto st_dot_digit;
				}
				if (iswalnum(ch))
					return T_ERROR;
				m_pFile->m_pView--;
				return T_FLOAT;
		}
	st_exp:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_ERROR;
		switch (ch = *m_pFile->m_pView++) {
			case '+':
			case '-':
				m_strSymbol += ch;
				goto st_exp_sign;
			default:
				if (iswdigit(ch)) {
					m_strSymbol += ch;
					goto st_exp_digit;
				}
				m_pFile->m_pView--;
				return T_ERROR;
		}
	st_exp_sign:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_ERROR;
		if (iswdigit(ch = *m_pFile->m_pView++)) {
			m_strSymbol += ch;
			goto st_exp_digit;
		}
		m_pFile->m_pView--;
		return T_ERROR;
	st_exp_digit:
		if (m_pFile->m_pView == m_pFile->m_pViewEnd)
			return T_FLOAT;
		if (iswdigit(ch = *m_pFile->m_pView++)) {
			m_strSymbol += ch;
			goto st_exp_digit;
		}
		if (iswalnum(ch))
			return T_ERROR;
		m_pFile->m_pView--;
		return T_FLOAT;
	}
}

std::intptr_t	XParser::ParseList(PARSE_CALLBACK Callback, std::intptr_t dwParam)
{
	TOKEN token;
	lisp::var top;
	lisp::var stack;

	for (; ; ) {
		token = GetToken();
		switch (token) {
			case T_END:
				if (m_nDepth != 0) {
					TRACE("Unmatched open parenthesis\n");
					top = lisp::error;
					goto quit;
				}
				top = lisp::nreverse(top);
				ASSERT(stack.null());
				goto quit;

			case T_CLOSE:
				top = lisp::nreverse(top);
				// ������ �����Ѵٸ�                
				if (stack.consp()) {
					// ���� ������ ������ ���
					lisp::_cons *pObject = (lisp::_cons *) stack.m_pObject;
					// ���ÿ� ������ ������ �����Ѵ�.(����)
					stack = pObject->m_car;
					// ������ ž�� ������Ʈ�� ž�� ���δ�.
					pObject->m_car = top;
					// ž�� ������Ʈ�� �Է�
					top = pObject;
				}
				else if (m_nDepth < 0) {
					TRACE("Unmatched close parenthesis %d\n", GetLine());
					top = lisp::error;
					m_nDepth = 0;
					goto quit;
				}
				else
					goto quit;
				break;
			case T_OPEN:
				// �ٽ� ������ ���� ���ÿ� ���� ž�� �ּҸ� ���� ��Ƶд�.
				stack = new(_alloca(sizeof(lisp::_cons))) lisp::_cons(stack, top);
				// ž�� ����(�ٽ� �ε��ؾ� �ϹǷ�)
				top = lisp::nil;

				break;
			case T_STRING:
			{
				std::pair<lisp::_cons, lisp::_string> *p = (std::pair<lisp::_cons, lisp::_string> *)
					_alloca(sizeof(std::pair<lisp::_cons, lisp::_string>) + m_strSymbol.size() + 1);
				new (&p->second) lisp::_string(strcpy((char *)(p + 1), m_strSymbol.c_str()));

				// cons �� ���� ��(���� ����Ʈ) �� �����Ѵ�.
				top = new(&p->first) lisp::_cons(&p->second, top);
			}
			break;
			case T_INTEGER:
			{
				std::pair<lisp::_cons, lisp::_integer> *p = (std::pair<lisp::_cons, lisp::_integer> *)
					_alloca(sizeof(std::pair<lisp::_cons, lisp::_integer>));
				new (&p->second) lisp::_integer(GetInteger());
				top = new(&p->first) lisp::_cons(&p->second, top);
			}
			break;
			case T_FLOAT:
			{
				std::pair<lisp::_cons, lisp::_float> *p = (std::pair<lisp::_cons, lisp::_float> *)
					_alloca(sizeof(std::pair<lisp::_cons, lisp::_float>));
				new(&p->second) lisp::_float(GetFloat());
				top = new(&p->first) lisp::_cons(&p->second, top);
			}
			break;
			default:
				top = lisp::error;
				goto quit;
		}
	}
quit:
	return (*Callback)(dwParam, top);
}
