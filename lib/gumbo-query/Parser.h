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

#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <gumbo.h>
#include "Selector.h"

class CParser
{
	private:

		CParser(std::string aInput);

	public:

		virtual ~CParser();

	public:

		static CSelector* create(std::string aInput);

	private:

		CSelector* parseSelectorGroup();

		CSelector* parseSelector();

		CSelector* parseSimpleSelectorSequence();

		void parseNth(int& aA, int& aB);

		int parseInteger();

		CSelector* parsePseudoclassSelector();

		CSelector* parseAttributeSelector();

		CSelector* parseClassSelector();

		CSelector* parseIDSelector();

		CSelector* parseTypeSelector();

		bool consumeClosingParenthesis();

		bool consumeParenthesis();

		bool skipWhitespace();

		std::string parseString();

		std::string parseName();

		std::string parseIdentifier();

		bool nameChar(char c);

		bool nameStart(char c);

		bool hexDigit(char c);

		std::string parseEscape();

		std::string error(std::string message);

	private:

		std::string mInput;

		size_t mOffset;
};

#endif /* PARSER_H_ */

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
