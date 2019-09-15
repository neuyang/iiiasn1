/*
 * InvalidTracer.cxx
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

#ifdef ASN1_HAS_IOSTREAM
#include "asn1.h"

namespace ASN1 {

class InvalidTracer : public ConstVisitor
{
public:
	friend std::ostream& operator << (std::ostream& os, const InvalidTracer& tracer) 
	{
		return os << tracer.strm.str();
	}
private:
	bool do_encode(const Null& value);
	bool do_encode(const BOOLEAN& value);
	bool do_encode(const INTEGER& value);
	bool do_encode(const ENUMERATED& value);
	bool do_encode(const OBJECT_IDENTIFIER& value);
	bool do_encode(const BIT_STRING& value) ;
	bool do_encode(const OCTET_STRING& value);
	bool do_encode(const ConstrainedString& value);
	bool do_encode(const BMPString& value);
	bool do_encode(const CHOICE& value);
	bool do_encode(const OpenData& value);
	bool do_encode(const GeneralizedTime& value);
	bool do_encode(const SEQUENCE_OF_Base& value);

	bool encodeExtensionRoot(const SEQUENCE& value, int index);
	bool encodeKnownExtension(const SEQUENCE& value, int index);

	std::stringstream strm;
};

bool InvalidTracer::do_encode(const Null& value) 
{ 
	return true;
}

bool InvalidTracer::do_encode(const BOOLEAN& value) 
{ 
	return true;
}

bool InvalidTracer::do_encode(const INTEGER& value) { 
	if (value.getLowerLimit() >= 0)
	{
		unsigned v = static_cast<unsigned>(value.getValue());
		if (value.getValue() < value.getLowerLimit())
			strm << " The INTEGER value " << v << " is smaller than lower bound " << value.getLowerLimit();
		else if (v > value.getUpperLimit() && !value.extendable())
			strm << " The INTEGER value " << v << " is greater than upper bound " << value.getUpperLimit();
		else 
			return true;
	} else
	{
		if (value.getValue() < value.getLowerLimit() )
			strm << " The INTEGER value " << value.getValue() << " is smaller than lower bound " << value.getLowerLimit();
		else if (static_cast<unsigned>(value.getValue()) > value.getUpperLimit() && !value.extendable())
			strm << " The INTEGER value " << value.getValue() << " is greater than upper bound " << value.getUpperLimit();
		else
			return true;
	}
	return false;
}

bool InvalidTracer::do_encode(const ENUMERATED& value)
{ 
	if (value.asInt() > value.getMaximum())
	{
		strm << " This ENUMERATED has invalid value " << value.asInt();
		return false;
	}
	return true; 
}

bool InvalidTracer::do_encode(const OBJECT_IDENTIFIER& value) 
{ 
	if (value.levels() == 0)
	{
		strm << " This OBJECT IDENTIFIER is not assigned";
		return false;
	}
	return true; 
}

bool InvalidTracer::do_encode(const BIT_STRING& value) 
{ 
	if (value.size() < static_cast<unsigned>(value.getLowerLimit()))
	{
		strm << " This BIT STRING has size " << value.size() << " smaller than its lower bound " << value.getLowerLimit();
	} else if (value.getConstraintType() == FixedConstraint && value.size() > value.getUpperLimit())
	{
		strm << " This BIT STRING has size " << value.size() << " greater than its upper bound " << value.getUpperLimit();
	} else
		return true;

	return false; 
}

bool InvalidTracer::do_encode(const OCTET_STRING& value) 
{ 
	if (value.size() < static_cast<unsigned>(value.getLowerLimit()))
	{
		strm << " This OCTET STRING has size " << value.size() << " smaller than its lower bound " << value.getLowerLimit();
	} else if (value.getConstraintType() == FixedConstraint && value.size() > value.getUpperLimit())
	{
		strm << " This OCTET STRING has size " << value.size() << " greater than its upper bound " << value.getUpperLimit();
	} else
		return true;

	return false; 
}

bool InvalidTracer::do_encode(const ConstrainedString& value)
{ 
	int pos;
	if (value.size() < static_cast<unsigned>(value.getLowerLimit()))
	{
		strm << " This ConstrainedString has size " << value.size() << " smaller than its lower bound " << value.getLowerLimit();
	} else if (value.getConstraintType() == FixedConstraint && value.size() > value.getUpperLimit())
	{
		strm << " This ConstrainedString has size " << value.size() << " greater than its upper bound " << value.getUpperLimit();
	} else if ((pos = value.find_first_invalid()) != std::string::npos)
	{
		strm << " The character '" << value[pos]  << "' is not valid for the string";
	} else
		return true;

	return false; 
}

bool InvalidTracer::do_encode(const BMPString& value)
{
	unsigned pos;
	if (value.size() < static_cast<unsigned>(value.getLowerLimit()))
	{
		strm << " This BMPString has size " << value.size() << " smaller than its lower bound " << value.getLowerLimit();
	} else if (value.getConstraintType() == FixedConstraint && value.size() > value.getUpperLimit())
	{
		strm << " This BMPString has size " << value.size() << " greater than its upper bound " << value.getUpperLimit();
	} else if ((pos = value.first_illegal_at()) < value.size())
	{
		strm << " The character '" << value[pos]  << "' is not valid for the string";
	} else
		return true;

	return false; 
}

bool InvalidTracer::do_encode(const CHOICE& value)
{
	if (value.currentSelection() == CHOICE::unselected_)
		strm << " This CHOICE is not selected";
	else if (value.currentSelection() == CHOICE::unknownSelection_)
		strm << " This selection is not understood by this decoder";
	else {
		strm << "." << value.getSelectionName();
		return value.getSelection()->encode(*this);
	}
	return false;
}

bool InvalidTracer::do_encode(const OpenData& value)
{ 
	if (value.has_data())
		return value.get_data().encode(*this);

	if (!value.has_buf())
	{
		strm << " This Open Type does not contain any valid data";
		return false;
	}
	return true; 
}

bool InvalidTracer::do_encode(const GeneralizedTime& value)  
{ 
	if (value.isStrictlyValid())
	{
		strm << " This GeneralizedTime is not valid";
		return false;
	}
	return true;
}

bool InvalidTracer::do_encode(const SEQUENCE_OF_Base& value)
{ 
	if (value.size() < static_cast<unsigned>(value.getLowerLimit()))
	{
		strm << " This SEQUENCE OF has size " << value.size() << " smaller than its lower bound " << value.getLowerLimit();
		return false;
	} else
	if (value.getConstraintType() == FixedConstraint && value.size() > value.getUpperLimit())
	{
		strm << " This SEQUENCE OF has size " << value.size() << " greater than its upper bound " << value.getUpperLimit();
		return false;
	}

	SEQUENCE_OF_Base::const_iterator first = value.begin(), last = value.end();
	for (; first != last; ++first)
	{
		InvalidTracer tracer;
		if (!(*first)->encode(tracer))
		{
			strm << "[" << first- value.begin() << "]" << tracer;
			return false;
		}
	}
	return true;
}

bool InvalidTracer::encodeExtensionRoot(const SEQUENCE& value, int index)
{ 
	InvalidTracer tracer;
	if (!value.getField(index)->encode(tracer))
	{
		strm <<  "." << value.getFieldName(index) << tracer;
		return false;
	}
	return true; 
}

bool InvalidTracer::encodeKnownExtension(const SEQUENCE& value, int index) 
{ 
	return encodeExtensionRoot(value, index); 
}

bool trace_invalid(std::ostream& os, const char* str, const AbstractData& data)
{
	InvalidTracer tracer;
	if (!data.encode(tracer))
	{
		os << str << tracer << std::endl;
		return false;
	}
	return true;
}

}
#endif
