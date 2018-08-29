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

#include "Selection.h"
#include "Parser.h"
#include "QueryUtil.h"
#include "Node.h"

CSelection::CSelection(GumboNode* apNode)
{
	mNodes.push_back(apNode);
}

CSelection::CSelection(std::vector<GumboNode*> aNodes)
{
	mNodes = aNodes;
}

CSelection::~CSelection()
{
}

CSelection CSelection::find(std::string aSelector)
{
	CSelector* sel = CParser::create(aSelector);
	std::vector<GumboNode*> ret;
	for (std::vector<GumboNode*>::iterator it = mNodes.begin(); it != mNodes.end(); it++)
	{
		GumboNode* pNode = *it;
		std::vector<GumboNode*> matched = sel->matchAll(pNode);
		ret = CQueryUtil::unionNodes(ret, matched);
	}
	sel->release();
	return CSelection(ret);
}

CNode CSelection::nodeAt(size_t i)
{
	if (i >= mNodes.size())
	{
		return CNode();
	}

	return CNode(mNodes[i]);
}

size_t CSelection::nodeNum()
{
	return mNodes.size();
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

