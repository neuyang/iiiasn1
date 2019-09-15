/*
 * CoderEnv.cxx
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
   
bool CoderEnv::decode(const char* first, const char* last, AbstractData& val, bool defered)
{
	if (get_encodingRule() == per_Basic_Aligned)
	{
		PERDecoder decoder(first, last, defered ? NULL : this);
		return val.decode(decoder);
	}
	if (get_encodingRule() == ber)
	{
		BERDecoder decoder(first, last, defered ? NULL : this);
		return val.decode(decoder);
	}
#ifdef ASN1_HAS_IOSTREAM
	if (get_encodingRule() == avn)
	{
		std::istringstream strm(std::string(first, last));
		AVNDecoder decoder(strm);
		return val.decode(decoder);
	}
#endif
	return false;

}
}
