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

#ifndef NODE_H_
#define NODE_H_

#include <gumbo.h>
#include <string>

class CSelection;

class CNode
{
	public:

		CNode(GumboNode* apNode = NULL);

		virtual ~CNode();

	public:

		bool valid();

		CNode parent();

		CNode nextSibling();

		CNode prevSibling();

		unsigned int childNum();

		CNode childAt(size_t i);

		std::string attribute(std::string key);

		std::string text();

		std::string ownText();

		size_t startPos();

		size_t endPos();

		size_t startPosOuter();

		size_t endPosOuter();

		std::string tag();

		CSelection find(std::string aSelector);

	private:

		GumboNode* mpNode;
};

#endif /* NODE_H_ */

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
