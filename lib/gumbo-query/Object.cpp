/***************************************************************************
 * 
 * $Id$
 * 
 **************************************************************************/

/**
 * @file $HeadURL$
 * @author $Author$(hoping@baimashi.com)
 * @date $Date$
 * @version $Revision$
 * @brief 
 *  
 **/

#include "Object.h"

CObject::CObject()
{
	mReferences = 1;
}

CObject::~CObject()
{
	if (mReferences != 1)
	{
		throw "something wrong, reference count not zero";
	}
}

void CObject::retain()
{
	mReferences++;
}

void CObject::release()
{
	if (mReferences < 0)
	{
		throw "something wrong, reference count is negative";
	}

	if (mReferences == 1)
	{
		delete this;
	}
	else
	{
		mReferences--;
	}
}

unsigned int CObject::references()
{
	return mReferences;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

