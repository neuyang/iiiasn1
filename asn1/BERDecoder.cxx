/*
 * BERDecoder.cxx
 *
 * Abstract Syntax Notation 1 Encoding Rules
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * The code is adapted from asner.cxx of PWLib, but the dependancy on PWLib has
 * been removed.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <memory>
#include "asn1.h"

namespace ASN1 {

extern unsigned CountBits(unsigned range);

bool CHOICE::setID(unsigned tagVal, unsigned tagClass)
{
	bool result = true;
	unsigned tag = tagClass << 16 | tagVal;

	if (info()->tags == NULL)
		choiceID = tagVal;
	else
	{
		unsigned* tagInfo = std::lower_bound(info()->tags, info()->tags+info()->numChoices, tag);
		choiceID = tagInfo - info()->tags;
		result = (choiceID != info()->numChoices) && (*tagInfo == tag); 
	}

	if (result)
	{
		if (!createSelection())
			return false;
	}
	else if ( info()->tags[0] == 0)
    {
		choiceID = 0;
		createSelection();
		CHOICE* obj = std::static_cast<CHOICE*>(choice.get());
		if (obj->setID(tagVal, tagClass))
			result = true;
		
        if (!result)
			choiceID = unknownSelection_;
    }


	return result;
}

inline bool BERDecoder::atEnd() 
{ 
	return beginPosition >= endPosition; 
}

inline unsigned char BERDecoder::decodeByte() 
{ 
	assert(!atEnd()); 
	return *beginPosition++; 
}


bool BERDecoder::do_decode(Null& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return false;

	beginPosition += len;
	return true;
}

bool BERDecoder::do_decode(BOOLEAN& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return false;

	while (len-- > 0) {
		if (atEnd())
			return false;
		value = (decodeByte() != 0);
	}

	return true;
}

bool BERDecoder::do_decode(INTEGER& value)
{
	unsigned len;
	if (!decodeHeader(value, len) || len == 0 || atEnd())
		return false;

	int accumulator = static_cast<signed char>(decodeByte()); // sign extended first byte
	while (--len > 0) {
		if (atEnd())
			return false;
		accumulator = (accumulator << 8) | decodeByte();
	}

	value = accumulator;
	return true;
}

bool BERDecoder::do_decode(ENUMERATED& value)
{
	unsigned len;
	if (!decodeHeader(value, len) || len == 0 || atEnd())
		return false;

	unsigned val = 0;
	while (len-- > 0) {
		if (atEnd())
			return false;
		val = (val << 8) |  decodeByte();
	}

	value.setFromInt(val);
	return true;
}


bool BERDecoder::do_decode(OBJECT_IDENTIFIER& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return false;

	beginPosition += len;
	if (atEnd()) return false;

	return value.decodeCommon(beginPosition-len, len);
}

bool BERDecoder::do_decode(BIT_STRING& value)
{
	unsigned len;
	if (!decodeHeader(value, len) || len == 0 || atEnd())
		return false;
	value.totalBits = (len-1)*8 - decodeByte();
	unsigned nBytes = (value.totalBits+7)/8;
	value.bitData.resize(nBytes);
	return decodeBlock(&*value.bitData.begin(), nBytes) == nBytes;
}

bool BERDecoder::do_decode(OCTET_STRING& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return false;
	value.resize(len);
	return decodeBlock(&*value.begin(), len) == len;
}

bool BERDecoder::do_decode(AbstractString& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return false;
	value.resize(len);
	return decodeBlock(&*value.begin(), len) == len;
}

bool BERDecoder::do_decode(BMPString& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return false;
	value.resize(len/2);
	if ((beginPosition+len) >= endPosition)
		return false;
	for (unsigned i = 0; i < len/2; ++i)
		value[i] = (decodeByte() << 8) | decodeByte();
	return true;
}

bool BERDecoder::decodeChoicePreamle(CHOICE& value, memento_type& nextPosition)
{
	const char* savedPosition = beginPosition;

	unsigned tag;
	bool primitive;
	unsigned entryLen;
	if (!decodeHeader(tag, primitive, entryLen))
		return false;

	if (dontCheckTag || value.getTag() != 0)
	{
		savedPosition = beginPosition;
		if (!decodeHeader(tag, primitive, entryLen))
			return false;
	}
	nextPosition = beginPosition + entryLen;
	beginPosition = savedPosition;
	if (value.setID(tag & 0xffff, tag >> 16))
	{
		if (value.getSelectionTag() != 0) 
			dontCheckTag = 1;
		return true;
	}
	return false;
}


bool BERDecoder::do_decode(CHOICE& value)
{
	memento_type memento;
	if (decodeChoicePreamle(value,memento))
	{
		if (!value.isUnknownSelection() && 
			!value.getSelection()->accept(*this))
			return false;
		rollback(memento);
		return true;
	}
	return false;
}

bool BERDecoder::do_decode(SEQUENCE_OF_Base& value)
{
	value.clear();

	unsigned len;
	if (!decodeHeader(value, len))
		return false;

	const char* endPos = beginPosition + len;

	if (endPos > endPosition ) return false;


	SEQUENCE_OF_Base::iterator it = value.begin(), last = value.end();
	while (beginPosition < endPos && it != last)
	{
		if (!(*it)->decode(*this))
		{
			value.erase(it, last);
			return false;
		}
		++it;
	}

	if (it != last)
		value.erase(it, last);

	while (beginPosition < endPos) {
		std::unique_ptr<AbstractData> obj(value.createElement());
		if (!obj->decode(*this))
			return false;
		value.push_back(obj.release());
	}

	beginPosition = endPos;

	return true;
}

bool BERDecoder::do_decode(OpenData& value)
{
  const char* savedPosition = beginPosition;

  unsigned tag;
  bool primitive;
  unsigned entryLen;
  if (!decodeHeader(tag, primitive, entryLen))
    return false;

  if (value.getTag() == 0)
	beginPosition = savedPosition;

  if (!value.has_buf())
	  value.grab(new OpenBuf);
  value.get_buf().resize(entryLen);
  decodeBlock(&*value.get_buf().begin(), entryLen);
  return true;
}

bool BERDecoder::do_revisit(OpenData& value)
{
	if (!value.has_buf() || !value.has_data())
		return false;
	BERDecoder decoder(&*value.get_buf().begin(), &*value.get_buf().end(), get_env());
	return value.get_data().decode(decoder);
}

bool BERDecoder::do_decode(TypeConstrainedOpenData& value)
{
	assert(value.has_data());
	const char* savedPosition = beginPosition;
	
	unsigned tag;
	bool primitive;
	unsigned entryLen;
	if (!decodeHeader(tag, primitive, entryLen))
		return false;
	
	if (value.getTag() == 0)
		beginPosition = savedPosition;
	return value.get_data().decode(*this);
}


bool BERDecoder::do_decode(GeneralizedTime& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return false;

	std::vector<char> block(len);
	if (decodeBlock(&*block.begin(), len) == len)
	{
		value.set(&*block.begin());
		return true;
	}
	return false;
}

Visitor::VISIT_SEQ_RESULT BERDecoder::preDecodeExtensionRoots(SEQUENCE& value)
{
	unsigned len;
	if (!decodeHeader(value, len))
		return FAIL;

	endSEQUENCEPositions.push_back(beginPosition + len);
	return !atEnd() ? CONTINUE : FAIL;
}

Visitor::VISIT_SEQ_RESULT BERDecoder::decodeExtensionRoot(SEQUENCE& value, int index, int optional_id)
{
	if (atEnd())
		if (optional_id == -1)
			return FAIL;
		else
			return CONTINUE;

	const char* savedPosition = beginPosition;
    
	if ( (endSEQUENCEPositions.back() == savedPosition && optional_id == -1) || 
		(endSEQUENCEPositions.back() < savedPosition))
	return FAIL;
    
    unsigned tag;
    bool primitive;
    unsigned entryLen;
    if (!decodeHeader(tag, primitive, entryLen))
        return FAIL;
    beginPosition = savedPosition;
    unsigned fieldTag = value.getFieldTag(index);

    if ((fieldTag == tag) || (fieldTag == 0))
    {

        if (optional_id != -1)
            value.includeOptionalField(optional_id, index);
        
        AbstractData* field = value.getField(index);
        if (field)
        {
			if (value.tagMode() != SEQUENCE::IMPLICIT_TAG)
				dontCheckTag = 1;
            if (field->decode(*this))
					return CONTINUE;
            
            if (optional_id != -1)
            {
                value.removeOptionalField(optional_id);
                if (fieldTag == 0)
                    return CONTINUE;
            }
            return FAIL;
        }
        return optional_id != -1 ? CONTINUE : FAIL;
    }
    return CONTINUE;
}


Visitor::VISIT_SEQ_RESULT BERDecoder::decodeKnownExtension(SEQUENCE& value, int index, int optional_id)
{
	return decodeExtensionRoot(value, index, optional_id);
}

bool BERDecoder::decodeUnknownExtensions(SEQUENCE& value)
{
  beginPosition = endSEQUENCEPositions.back();
  endSEQUENCEPositions.pop_back();
  return true;
}

bool BERDecoder::decodeHeader(unsigned& tag,
                      bool & primitive,
                      unsigned & len)
{
	unsigned tagVal, tagClass;
	unsigned char ident = decodeByte();
	tagClass = ident & 0xC0;
	primitive = (ident & 0x20) == 0;
	tagVal = ident&31;
	if (tagVal == 31) {
		unsigned char b;
		tagVal = 0;
		do {
			if (atEnd())
				return false;

			b = decodeByte();
			tagVal = (tagVal << 7) | (b&0x7f);
		} while ((b&0x80) != 0);
	}

	tag = tagVal | (tagClass << 16);

	if (atEnd())
		return false;

	unsigned char len_len = decodeByte();
	if ((len_len & 0x80) == 0) {
		len = len_len;
		return true;
	}

	len_len &= 0x7f;

	len = 0;
	while (len_len-- > 0) {
		if (atEnd())
			return false;
		len = (len << 8) | decodeByte();
	}

	return true;
}

bool BERDecoder::decodeHeader(AbstractData & obj, unsigned & len)
{
	const char* pos = beginPosition;

	unsigned tag;
	bool primitive;
	if (decodeHeader(tag, primitive, len) &&
		(tag == obj.getTag() || dontCheckTag--))
		return true;

	beginPosition = pos;
	return false;
}

unsigned BERDecoder::decodeBlock(char * bufptr, unsigned nBytes)
{

	if (beginPosition+nBytes > endPosition)
		nBytes = endPosition - beginPosition;

	if (nBytes == 0)
		return 0;

	memcpy(bufptr, beginPosition, nBytes);

	beginPosition += nBytes;
	return nBytes;
}

}

