/*
 * h323test.cpp
 * 
 * Copyright (c) 2001 Institute for Information Industry, Taiwan, Republic of China 
 * (http://www.iii.org.tw/iiia/ewelcome.htm)
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "asn1test.h"
 


bool CoderTest(const char* str, CoderEnv& env, const AbstractData& v1, AbstractData& v2, 
			   const char* encodedStrm, int lineno)
{
	printf("%s\n", str);
	bool result = true;
	std::vector<char> strm;
	printf("encode ");
	if (encode(v1, &env, std::back_inserter(strm)) && std::equal(strm.begin(), strm.end(), encodedStrm))
		printf("success \n");
	else 
	{
		std::vector<char>::const_iterator pc = std::mismatch(strm.begin(), strm.end(), encodedStrm).first;
		result = false;
		printf("FAIL !!!! \n");
		printf("The encoded sequence is wrong in the %dth byte : 0x%02x <-> 0x%02x\n"
			, pc-strm.begin(), (unsigned char)*pc, (unsigned char)encodedStrm[pc-strm.begin()]);
	}
	
	result = decode(strm.begin(), strm.end(), &env, v2);
	if (result && v1 == v2)
		printf( "decode success \n");
	else if (!result)
	{
		printf("decode error\n");
	}
	else
	{
		result = false;
		printf( "decode result doesn't match the original value\n");
	}
	return result;
}



extern void H225Test(void);
extern void H245Test(void);
extern void H450Test(void);
extern void H248Test(void);

extern "C" void SMC1Init();
int main(int argc, char* argv[])
{
#ifdef _MPC850
	SMC1Init();
#endif
	H245Test();
    H225Test();
	H248Test();    
	H450Test();
	return 0; 
}

