/*
 * avnencoder.cxx
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
#ifdef ASN1_HAS_IOSTREAM
#include <boost/smart_ptr.hpp>
#include <iomanip>
#include "ios_helper.h"

namespace ASN1 {

std::string AbstractData::asValueNotation() const
{
    std::ostringstream strm;
    AVNEncoder encoder(strm);
    encode(encoder);
    return strm.str();
}

std::ios_base::iostate AbstractData::print_on(std::ostream & strm) const
{
    AVNEncoder encoder(strm);
    if (encode(encoder))
        return std::ios_base::goodbit;
    return std::ios_base::failbit;
}

std::ostream & operator<<(std::ostream & os, const AbstractData & arg)
{
    if (!os.good()) return os;
    return g_inserter(os, arg);
}


bool IntegerWithNamedNumber::getName(std::string& str) const
{
	const NameEntry* begin = info()->nameEntries;
	const NameEntry* end   = begin+ info()->entryCount;
	const NameEntry* i = std::lower_bound(begin, end, value, NameEntryCmp());

	if (i != end && i->value == value)
	{
		str = i->name;
		return true;
	}
	return false;
}

const char* ENUMERATED::getName() const
{
	if (value >=0 && value <= getMaximum())
		return info()->names[value];
	return 0;
}

bool AVNEncoder::do_encode(const Null& value)
{
	strm << "NULL";
	return strm.good();
}

bool AVNEncoder::do_encode(const BOOLEAN& value)
{
	if (value) 
		strm << "TRUE";
	else
		strm << "FALSE";
	return strm.good();
}

bool AVNEncoder::do_encode(const INTEGER& value)
{
	if (!value.constrained() || value.getLowerLimit() < 0)
		strm << (int)value.getValue();
	else
		strm << (unsigned)value.getValue();
	return strm.good();
}

bool AVNEncoder::do_encode(const IntegerWithNamedNumber& value)
{
	std::string str;
	if (value.getName(str))
		strm << str;
	else 
		return encode(static_cast<const INTEGER&>(value));
	return strm.good();
}

bool AVNEncoder::do_encode(const ENUMERATED& value)
{
	const char* name = value.getName();
	if (name != 0)
		strm << name;
	else 
		strm << value.asInt();
	return strm.good();
}

bool AVNEncoder::do_encode(const OBJECT_IDENTIFIER& value)
{
	strm << "{ ";
	for (unsigned i = 0;  i < value.levels() && strm.good(); ++i) 
		strm << value[i] << ' ';
	strm << "}";
	return strm.good();
}

bool AVNEncoder::do_encode(const BIT_STRING& value)
{
	strm << '\'';
	for (unsigned i = 0; i < value.size() && strm.good(); ++i)
		if (value[i])
			strm << '1';
		else
			strm << '0';
	if (strm.good())
		strm << "\'B";
	return strm.good();
}

bool AVNEncoder::do_encode(const OCTET_STRING& value)
{
	std::ios_base::fmtflags flags = strm.flags();
	strm << '\'';
	
	for (unsigned i = 0; i < value.size() && strm.good(); ++i)
	{
		strm << std::hex << std::setw(2) << std::setfill('0') << (0xFF & value[i]);
		if (i != value.size()-1) 
			strm << ' ';
	}
	
	if (strm.good())
		strm << "\'H";
	strm.setf(flags);
	strm << std::setfill(' ') ;
	return strm.good();
}

bool AVNEncoder::do_encode(const AbstractString& value)
{
	strm << '\"' << static_cast<const std::string&>(value) << '\"';
	return strm.good();
}

bool AVNEncoder::do_encode(const BMPString& value)
{
	boost::scoped_array<char> tmp(new char[value.size()*2+1]);
	int len = wcstombs(tmp.get(), &*value.begin(), value.size());
	if (len != -1)
	{
		tmp[len] = 0;
		strm << '\"' << tmp.get() << '\"';
	}
	else // output Quadruple form
	{
		strm << '{';
		for (unsigned i = 0; i < value.size() && strm.good(); ++i)
		{
			strm << "{ 0, 0, " << (int) (value[i] >> 8) << ", " << (int) value[i]  << '}';
			if (strm.good() && i != value.size()-1)
				strm << ", ";
		}
		if (strm.good())
			strm << '}';
	}
	return strm.good();
}

bool AVNEncoder::do_encode(const CHOICE& value)
{
	if (value.currentSelection() >= 0)
	{
		strm << value.getSelectionName() << " : ";
		return value.getSelection()->encode(*this);
	}
	return false;
}

bool AVNEncoder::do_encode(const OpenData& value)
{
	if (value.has_data())
	{
		return value.get_data().encode(*this);
	}
	else if (value.has_buf())
	{
		OCTET_STRING ostr(value.get_buf());
		return ostr.encode(*this);
	}
	return false;
}

bool AVNEncoder::do_encode(const GeneralizedTime& value)
{
	strm << '\"' << value.get() << '\"';
	return strm.good();
}

bool AVNEncoder::do_encode(const SEQUENCE_OF_Base& value)
{
	strm << "{\n";
    SEQUENCE_OF_Base::const_iterator first = value.begin(), last = value.end();
    indent +=2;
	for (; first != last && strm.good(); ++first)
	{
		strm << std::setw(indent) << " ";
		if (!(*first)->encode(*this))
			return false;
		if (first != last-1)
			strm << ",\n";
	}
    indent -=2;
	if (value.size() && strm.good())
		strm << '\n';
	strm << std::setw(indent+1) << "}";
	return strm.good();
}

bool AVNEncoder::preEncodeExtensionRoots(const SEQUENCE& value) 
{
	outputSeparators.push_back(false);
	strm << "{\n";
	return strm.good();
}

bool AVNEncoder::encodeExtensionRoot(const SEQUENCE& value, int index)
{
	if (outputSeparators.back())
		strm << ",\n";
	strm << std::setw(strlen(value.getFieldName(index)) + indent +2) 
		<< value.getFieldName(index) << ' ';
    indent +=2;
	if (!value.getField(index)->encode(*this))
		return false;
    indent -=2;
	outputSeparators.back() = true;
	return true;
}

bool AVNEncoder::encodeKnownExtension(const SEQUENCE& value, int index)
{
	return encodeExtensionRoot(value, index);
}

bool AVNEncoder::afterEncodeSequence(const SEQUENCE& value)
{
	if (outputSeparators.back())
		strm  << '\n';
	outputSeparators.pop_back();
	strm << std::setw(indent + 1) << "}";
	return strm.good();
}

}

#endif
