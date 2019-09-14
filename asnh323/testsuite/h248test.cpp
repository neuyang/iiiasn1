#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <media_gateway_control.h>
#include "asn1test.h"

namespace H248 = MEDIA_GATEWAY_CONTROL;

using namespace std;
const char ip[] = "\xA4\x30\x33\xEE";
const char packetAndProp[] = "\x00\x00\x10\x01";
const char d[]= "\x30\x55\xA1\x53\x80\x01\x01\xA1"
	"\x0C\xA0\x0A\x80\x04\xA4\x30\x33"
	"\xEE\x81\x02\x13\x9C\xA2\x40\xA1"
	"\x3E\xA0\x3C\x80\x01\x01"
	"\xA1\x37\x30\x35\x80\x04\x19\x99"
	"\x26\x9E\xA3\x2D\x30\x2B\xA0\x29"
	"\xA0\x27\xA0\x0A\x30\x08\xA0\x03"
	"\x04\x01\x00\x81\x01\x00\xA1\x19"
	"\xA0\x17\xA1\x15\xA0\x13"
	"\xA1\x11\xA0\x0F\x30\x0D\x30\x0B"
	"\x80\x04\x00\x00\x10\x01\xA1\x03"
	"\x04\x01\x01";

void H248Test()
{

	////////////////////////////////////////////////
	// Set termination capabilities (Package, Property and parameter values)

	cout << "setting termination capabilities" << endl;

	H248::MegacoMessage myReq, myReq2;
	myReq.set_mess().set_version(1);
	H248::Message& myMsg = myReq.ref_mess();

	ASN1::OCTET_STRING& address = myMsg.set_mId().select_ip4Address().set_address();
	address.assign(&ip[0], &ip[4]); 
	myMsg.ref_mId().ref_ip4Address().set_portNumber(5020);
	myMsg.set_messageBody().select_transactions().resize(1);

	H248::TransactionRequest& transReq = myMsg.ref_messageBody().ref_transactions()[0].select_transactionRequest();
	transReq.set_transactionId(1);
	transReq.set_actions().resize(1);

	H248::ActionRequest& actReq = transReq.ref_actions()[0];
	actReq.set_contextId(429467294);
	actReq.set_commandRequests().resize(1);
	
	H248::CommandRequest& commReq = actReq.ref_commandRequests()[0];
	commReq.set_command().select_addReq().set_terminationID().resize(1);
	
	H248::AmmRequest& ammReq= commReq.ref_command().ref_addReq();
	H248::TerminationID& termID = ammReq.ref_terminationID()[0];

	termID.set_wildcard().resize(1);
	termID.ref_wildcard()[0].push_back('\x0');
	termID.set_id().push_back('\x0');  //   <--  you didn't assign this field
	
	ammReq.set_descriptors().resize(1);
	H248::StreamParms& strmParam = ammReq.ref_descriptors()[0].select_mediaDescriptor().set_streams().select_oneStream();
	strmParam.set_localDescriptor().set_propGrps().resize(1);

	strmParam.set_localDescriptor().set_propGrps()[0].resize(1);
	H248::PropertyParm& propParam = strmParam.set_localDescriptor().set_propGrps()[0][0];
	propParam.set_name().assign(&packetAndProp[0], &packetAndProp[4]);
	propParam.set_value().resize(1);
	propParam.ref_value()[0].push_back('\x01');

	////////////////////////////////////////////////
	ASN1::CoderEnv env;
	env.set_encodingRule(ASN1::CoderEnv::ber);

	TEST("MegacoMessage ", env, myReq, myReq2, d);

}
