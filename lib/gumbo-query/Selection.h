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

#ifndef SELECTION_H_
#define SELECTION_H_

#include "Object.h"
#include <vector>
#include <string>
#include <gumbo.h>

class CNode;

class CSelection: public CObject
{

	public:

		CSelection(GumboNode* apNode);

		CSelection(std::vector<GumboNode*> aNodes);

		virtual ~CSelection();

	public:

		CSelection find(std::string aSelector);

		CNode nodeAt(size_t i);

		size_t nodeNum();

	private:

		std::vector<GumboNode*> mNodes;
};

#endif /* SELECTION_H_ */

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
