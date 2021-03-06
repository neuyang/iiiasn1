/*
 * ValidChecker.cxx
 *
 * Copyright (c) 2001 Institute for Information Industry, Taiwan, Republic of China 
 * (http://www.iii.org.tw/iiia/ewelcome.htm)
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is III ASN.1 Tool
 *
 * The Initial Developer of the Original Code is Institute for Information Industry.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "asn1.h"

namespace ASN1 {

class StrictlyValidChecker : public ConstVisitor
{
public:
	bool encode(const Null& value) { return true;}
	bool encode(const BOOLEAN& value) { return true;}
	bool encode(const INTEGER& value) { return value.isStrictlyValid(); }
	bool encode(const ENUMERATED& value) { return value.isStrictlyValid(); }
	bool encode(const OBJECT_IDENTIFIER& value) { return value.isStrictlyValid(); }
	bool encode(const BIT_STRING& value) { return value.isStrictlyValid(); }
	bool encode(const OCTET_STRING& value) { return value.isStrictlyValid(); }
	bool encode(const ConstrainedString& value) { return value.isStrictlyValid(); }
	bool encode(const BMPString& value) { return value.isStrictlyValid(); }
	bool encode(const CHOICE& value) { return value.isStrictlyValid(); }
	bool encode(const OpenData& value){ return value.isStrictlyValid(); }
	bool encode(const GeneralizedTime& value)  { return value.isStrictlyValid(); }
	bool encode(const SEQUENCE_OF_Base& value){ return value.isStrictlyValid(); }

	bool encodeExtensionRoot(const SEQUENCE& value, int index) { 
		return value.getField(index)->isStrictlyValid(); 
	}
	bool encodeKnownExtension(const SEQUENCE& value, int index) { 
		return value.getField(index)->isStrictlyValid(); 
	}
};

class ValidChecker : public ConstVisitor
{
public:
	bool encode(const Null& value){ return true;}
	bool encode(const BOOLEAN& value){ return true;}
	bool encode(const INTEGER& value){ return value.isValid(); }
	bool encode(const ENUMERATED& value){ return value.isValid(); }
	bool encode(const OBJECT_IDENTIFIER& value){ return value.isValid(); }
	bool encode(const BIT_STRING& value){ return value.isValid(); }
	bool encode(const OCTET_STRING& value){ return value.isValid(); }
	bool encode(const ConstrainedString& value){ return value.isValid(); }
	bool encode(const BMPString& value){ return value.isValid(); }
	bool encode(const CHOICE& value){ return value.isValid(); }
	bool encode(const OpenData& value){ return value.isValid(); }
	bool encode(const GeneralizedTime& value){ return value.isValid(); }
	bool encode(const SEQUENCE_OF_Base& value){ return value.isValid(); }

	bool encodeExtensionRoot(const SEQUENCE& value, int index) { 
		return value.getField(index)->isValid(); 
	}
	bool encodeKnownExtension(const SEQUENCE& value, int index) { 
		return value.getField(index)->isValid(); 
	}
};

bool AbstractData::isValid() const
{
	ValidChecker checker;
	return encode(checker);
}

bool AbstractData::isStrictlyValid() const 
{
	StrictlyValidChecker checker;
	return encode(checker);
}

bool INTEGER::isStrictValid() const
{
	if (getLowerLimit() >= 0)
	{
		int v = static_cast<int>(value);
		return v >= getLowerLimit() && value <= getUpperLimit();
	} else {
		int v = static_cast<int>(value);
		return v >= getLowerLimit() && value <= static_cast<signed>(getUpperLimit());
	}
}


bool ConstrainedString::isValid() const
{
	return size() >= static_cast<unsigned>(getLowerLimit()) && 
		(size() <= getUpperLimit() || extendable()) &&
		(find_first_invalid() == std::string::npos);
}

bool ConstrainedString::isStrictlyValid() const
{
	return size() >= static_cast<unsigned>(getLowerLimit()) && 
		size() <= getUpperLimit() &&
		(find_first_invalid() == std::string::npos);
}

bool BMPString::legalCharacter(wchar_t ch) const
{
	if (ch < getFirstChar())
		return false;

	if (ch > getLastChar())
		return false;

	return true;
}

BMPString::size_type BMPString::first_illegal_at() const
{
	const_iterator first = begin(), last = end();
	for (; first != end(); ++first)
		if (!legalCharacter(*first))
			break;

	return first-begin();
}

bool BMPString::isValid() const
{
	return size() >= static_cast<unsigned>(getLowerLimit()) &&
		(size() <= getUpperLimit() || extendable()) &&
		first_illegal_at() == size();
}

bool BMPString::isStrictlyValid() const
{
	return size() >= static_cast<unsigned>(getLowerLimit()) &&
		size() <= getUpperLimit() &&
		first_illegal_at() == size();
}

bool GeneralizedTime::isStrictlyValid() const
{
	return (year > 0 ) &&
		(month > 0) && (month < 13) &&
		(day >0) && (day < 32) &&
		(hour >=0) && (hour <= 24) &&
		(minute >=0) && (minute < 60) &&
		(second >=0) && (second < 60) && 
		(mindiff <= 60*12) && (mindiff >= -60*12);
}

bool CHOICE::isValid() const
{
	return choiceID >=0 && (static_cast<unsigned>(choiceID) < info()->numChoices || extendable()) && choice->isValid();
}

bool CHOICE::isStrictlyValid() const
{
	return choiceID >= 0 && static_cast<unsigned>(choiceID) < info()->numChoices && choice->isStrictlyValid();
}

bool SEQUENCE_OF_Base::isValid() const
{
	if (container.size() >= static_cast<unsigned>(getLowerLimit()) && 
		(extendable() || container.size() <= getUpperLimit()))
	{
		Container::const_iterator first = container.begin(),
			                      last  = container.end();
		for (; first != last; ++ first)
		{
			if (!(*first)->isValid())
				return false;
		}

		return true;
	}
	return false;
}

bool SEQUENCE_OF_Base::isStrictlyValid() const
{
	if (container.size() >= static_cast<unsigned>(getLowerLimit()) && 
		container.size() <= getUpperLimit())
	{
		Container::const_iterator first = container.begin(),
			                      last  = container.end();
		for (; first != last; ++ first)
		{
			if (!(*first)->isStrictlyValid())
				return false;
		}

		return true;
	}
	return false;
}

bool OpenData::isValid() const
{
	if (has_data())
		return get_data().isValid();
	return has_buf();
}

bool OpenData::isStrictlyValid() const
{
	if (has_data())
		return get_data().isStrictlyValid();
	return has_buf();
}

}// namespace ASN1
