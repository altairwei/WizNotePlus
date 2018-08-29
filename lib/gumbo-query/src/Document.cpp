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

#include "Document.h"

CDocument::CDocument()
{
	mpOutput = NULL;
}

void CDocument::parse(const std::string& aInput)
{
	reset();
	mpOutput = gumbo_parse(aInput.c_str());
}

CDocument::~CDocument()
{
	reset();
}

CSelection CDocument::find(std::string aSelector)
{
	if (mpOutput == NULL)
	{
		throw "document not initialized";
	}

	CSelection sel(mpOutput->root);
	return sel.find(aSelector);
}

void CDocument::reset()
{
	if (mpOutput != NULL)
	{
		gumbo_destroy_output(&kGumboDefaultOptions, mpOutput);
		mpOutput = NULL;
	}
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

