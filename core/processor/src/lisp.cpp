#include <bango/processor/lisp.h>

#include <cstring>

using namespace bango::processor;

lisp::_null lisp::s_nil;
lisp::var lisp::nil;

lisp::_error lisp::s_error;
lisp::var const lisp::error(&s_error);

int	lisp::_string::GetInteger() const
{
	return strtol(m_pString, 0, 0);
}

float	lisp::_string::GetFloat() const
{
	return (float)atof(m_pString);
}

unsigned	lisp::_string::GetUnsigned() const
{
	return strtoul(m_pString, 0, 0);
}

lisp::_object* lisp::_string::copy() const
{
	char *p = (char *)malloc(sizeof(_string) + strlen(m_pString) + 1);
	return new (p) _string(strcpy(p + sizeof(_string), m_pString));
}

std::string lisp::_string::print(int level) const
{
	return std::string("\"") + m_pString + '"';
}

std::string lisp::_integer::print(int level) const
{
	char buf[20];
	sprintf(buf, "%d", m_nValue);
	return buf;
}

std::string lisp::_float::print(int level) const
{
	char buf[20];
	sprintf(buf, "%.2f", m_fValue);
	return buf;
}

int	lisp::_cons::length() const
{
	int len = 0;
	var v = m_cdr;
	for (; ; ) {
		++len;
		if (!v.consp()) {
			return len;
		}
		v = v.cdr();
	}
}

std::string	lisp::_cons::print(int level) const
{
	if (level >= 6)
		return "(...)";
	if (m_cdr.null())
		return '(' + m_car.print(level + 1) + ')';
	if (m_cdr.consp())
		return '(' + m_car.print(level + 5) + ' ' + (m_cdr.print(level + 1).c_str() + 1);
	return '(' + m_car.print(level + 5) + " . " + m_cdr.print(level + 1) + ')';
}

std::string lisp::_null::print(int level) const
{
	return "()";
}

std::string lisp::_error::print(int level) const
{
	return "#error";
}

lisp::var lisp::var::get(const char* name)
{
	var parent = *this;
	for (; !parent.null(); ) {
		var child = parent.pop();
		if (strcmp(name, child.car()) == 0)
			return child;
	}
	return nil;
}

lisp::var lisp::var::get(int pos)
{
	var v = *this;
	for (; --pos >= 0; ) {
		v = v.cdr();
	}
	return v.car();
}

lisp::var lisp::var::get(const char* name, int pos)
{
	var parent = *this;
	for (; !parent.null(); ) {
		var child = parent.pop();
		if (strcmp(name, child.car()) == 0 && --pos < 0)
			return child;
	}
	return nil;
}

lisp::var lisp::var::nthcdr(int pos)
{
	var v = *this;
	for (; --pos >= 0; ) {
		v = v.cdr();
	}
	return v;
}

lisp::_object*	lisp::_cons::copy() const
{
	std::intptr_t *vptr = *(std::intptr_t **)this;
	const _cons *parent = this;

	_object *root;
	_object **ppObject = &root;
	for (; ; ) {
		const _object *child = parent->m_cdr.m_pObject;
		_cons *cons = new _cons(parent->m_car.m_pObject->copy(), 0);
		*ppObject = cons;
		ppObject = &cons->m_cdr.m_pObject;
		if (*(std::intptr_t **)child != vptr) { // if (!child->consp())
			*ppObject = child->copy();
			break;
		}
		parent = (const _cons *)child;
	}
	return root;
}

void lisp::_cons::destroy()
{
	std::intptr_t *vptr = *(std::intptr_t **)this;
	_cons *parent = this;
	for (; ; ) {
		_object *child = parent->m_cdr.m_pObject;
		parent->m_car.m_pObject->destroy();
		delete parent;
		if (*(std::intptr_t **)child != vptr) { // if (!child->consp())
			child->destroy();
			break;
		}
		parent = (_cons *)child;
	}
}

lisp::var& lisp::find_nil(lisp::var& v, int nDepth)
{
	lisp::var *p = &v;
	lisp::var *prev = p;
	for (; ; ) {
		if (!p->consp()) {
			if (nDepth == 0)
				return *p;
			nDepth--;
			p = &prev->car();
			prev = p;
		}
		else {
			prev = p;
			p = &p->cdr();
		}
	}
}

/**
*  ���� ����Ʈ�� ������ �Լ�
* \date 2008-04-23
* \author oen
* \param v ������ ��
* \return ������ ��
*/
lisp::var	lisp::nreverse(lisp::var v)
{
	_object *pInput = v.m_pObject;

	if (!pInput->consp())
		return pInput;

	_object *pOutput = 0;

	for (; ; )
	{
		_cons *pObject = (_cons *)pInput;
		pInput = pObject->m_cdr.m_pObject;

		pObject->m_cdr.m_pObject = pOutput;
		pOutput = pObject;

		if (!pInput->consp())
			break;
	}

	((_cons *)v.m_pObject)->m_cdr.m_pObject = pInput;

	return pOutput;
}
