/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 * @author Vadim Revyakin
 * @author Sumeet Kukreja, Dell Inc.
 */

#ifndef _WSMAN_NAMESPACES_H_
#define _WSMAN_NAMESPACES_H_


// NameSpaces
#define XML_NS_SOAP_1_1             "http://schemas.xmlsoap.org/soap/envelope"
#define XML_NS_SOAP_1_2             "http://www.w3.org/2003/05/soap-envelope"


#define XML_NS_XML_NAMESPACES       "http://www.w3.org/XML/1998/namespace"
#define XML_NS_ADDRESSING           "http://schemas.xmlsoap.org/ws/2004/08/addressing"
#define XML_NS_DISCOVERY            "http://schemas.xmlsoap.org/ws/2004/10/discovery"
#define XML_NS_EVENTING             "http://schemas.xmlsoap.org/ws/2004/08/eventing"
#define XML_NS_ENUMERATION          "http://schemas.xmlsoap.org/ws/2004/09/enumeration"
#define XML_NS_TRANSFER             "http://schemas.xmlsoap.org/ws/2004/09/transfer"
#define XML_NS_XML_SCHEMA           "http://www.w3.org/2001/XMLSchema"
#define XML_NS_SCHEMA_INSTANCE      "http://www.w3.org/2001/XMLSchema-instance"
#define XML_NS_SCHEMA_INSTANCE_PREFIX      "xsi"
#define XML_NS_SCHEMA_INSTANCE_NIL      "nil"


#define XML_NS_OPENWSMAN            "http://schema.openwsman.org/2006/openwsman"

#define XML_NS_CIM_SCHEMA           "http://schemas.dmtf.org/wbem/wscim/1/common"
#define XML_NS_CIM_CLASS            "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2"
#define XML_NS_CIM_BINDING          "http://schemas.dmtf.org/wbem/wsman/1/cimbinding.xsd"


// WS-Management
#define XML_NS_WS_MAN               "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
#define XML_NS_WSMAN_FAULT_DETAIL   "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail"

#define XML_NS_WS_MAN_CAT           "http://schemas.xmlsoap.org/ws/2005/06/wsmancat"

#define XML_NS_WSMAN_ID             "http://schemas.dmtf.org/wbem/wsman/identity/1/wsmanidentity.xsd"



#define DMTF_WSMAN_SPEC_1

#define SOAP1_2_CONTENT_TYPE       "application/soap+xml;charset=UTF-8"
#define SOAP_CONTENT_TYPE          "application/soap+xml"

#define SOAP_SKIP_DEF_FILTERS       0x01
#define SOAP_ACTION_PREFIX          0x02 // otherwise exact
#define SOAP_ONE_WAY_OP             0x04
#define SOAP_NO_RESP_OP             0x08
#define SOAP_DONT_KEEP_INDOC        0x10

#define SOAP_CLIENT_RESPONSE        0x20 // internal use
#define SOAP_CUSTOM_DISPATCHER      0x40 // internal use
#define SOAP_IDENTIFY_DISPATCH      0x80 // internal use


#define WSMID_IDENTIFY              "Identify"
#define WSMID_IDENTIFY_RESPONSE     "IdentifyResponse"
#define WSMID_PROTOCOL_VERSION      "ProtocolVersion"
#define WSMID_PRODUCT_VENDOR        "ProductVendor"
#define WSMID_PRODUCT_VERSION       "ProductVersion"


#define XML_SCHEMA_NIL              "nil"


#define WSA_TO_ANONYMOUS     \
        "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous"
#define WSA_MESSAGE_ID              "MessageID"
#define WSA_ADDRESS                 "Address"
#define WSA_EPR                     "EndpointReference"
#define WSA_ACTION                  "Action"
#define WSA_RELATES_TO              "RelatesTo"
#define WSA_TO                      "To"
#define WSA_REPLY_TO                "ReplyTo"
#define WSA_FROM                    "From"
#define WSA_FAULT_TO                "FaultTo"
#define WSA_REFERENCE_PROPERTIES    "ReferenceProperties"
#define WSA_REFERENCE_PARAMETERS    "ReferenceParameters"
#define WSA_ACTION_FAULT    \
         "http://schemas.xmlsoap.org/ws/2004/08/addressing/fault"



#define SOAP_ENVELOPE               "Envelope"
#define SOAP_HEADER                 "Header"
#define SOAP_BODY                   "Body"
#define SOAP_FAULT                  "Fault"
#define SOAP_CODE                   "Code"
#define SOAP_VALUE                  "Value"
#define SOAP_SUBCODE                "Subcode"
#define SOAP_REASON                 "Reason"
#define SOAP_TEXT                   "Text"
#define SOAP_LANG                   "lang"
#define SOAP_DETAIL                 "Detail"
#define SOAP_FAULT_DETAIL           "FaultDetail"
#define SOAP_MUST_UNDERSTAND        "mustUnderstand"
#define SOAP_VERSION_MISMATCH       "VersionMismatch"
#define SOAP_UPGRADE                "Upgrade"
#define SOAP_SUPPORTED_ENVELOPE     "SupportedEnvelope"



/* Action URI */

#define TRANSFER_ACTION_GET     \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/Get"
#define TRANSFER_ACTION_GETRESPONSE     \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/GetResponse"
#define TRANSFER_GET                 "Get"
#define TRANSFER_GET_RESP            "GetResponse"

#define TRANSFER_ACTION_PUT     \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/Put"
#define TRANSFER_ACTION_PUTRESPONSE     \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/PutResponse"
#define TRANSFER_PUT                 "Put"
#define TRANSFER_PUT_RESP            "PutResponse"


#define TRANSFER_ACTION_CREATE   \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/Create"
#define TRANSFER_CREATE              "Create"

#define TRANSFER_ACTION_DELETE   \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/Delete"
#define TRANSFER_DELETE              "Delete"
#define TRANSFER_ACTION_DELETERESPONSE     \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/DeleteResponse"
#define TRANSFER_DELETE_RESP            "DeleteResponse"


#define WSXF_ACTION_FAULT        \
          "http://schemas.xmlsoap.org/ws/2004/09/transfer/fault"


#define ENUM_ACTION_ENUMERATE   \
         "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Enumerate"
#define ENUM_ACTION_ENUMERATERESPONSE   \
         "http://schemas.xmlsoap.org/ws/2004/09/enumeration/EnumerateResponse"
#define WSENUM_ENUMERATE              "Enumerate"
#define WSENUM_ENUMERATE_RESP         "EnumerateResponse"

#define ENUM_ACTION_RELEASE     \
           "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Release"
#define WSENUM_RELEASE                "Release"
#define WSENUM_RELEASE_RESP           "ReleaseResponse"

#define ENUM_ACTION_PULL        \
           "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Pull"
#define WSENUM_PULL                   "Pull"
#define WSENUM_PULL_RESP              "PullResponse"


#define ENUM_ACTION_RENEW       \
           "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Renew"
#define ENUM_ACTION_GETSTATUS    \
           "http://schemas.xmlsoap.org/ws/2004/09/enumeration/GetStatus"
#define ENUM_ACTION_ENUMEND       \
           "http://schemas.xmlsoap.org/ws/2004/09/enumeration/EnumerationEnd"

#define EVT_ACTION_SUBSCRIBE      \
           "http://schemas.xmlsoap.org/ws/2004/08/eventing/Subscribe"
#define EVT_ACTION_GETSTATUS      \
           "http://schemas.xmlsoap.org/ws/2004/08/eventing/GetStatus"
#define EVT_ACTION_UNSUBSCRIBE     \
           "http://schemas.xmlsoap.org/ws/2004/08/eventing/Unsubscribe"
#define EVT_ACTION_SUBEND           \
           "http://schemas.xmlsoap.org/ws/2004/08/eventing/SubscriptionEnd"
#define EVT_ACTION_RENEW           \
           "http://schemas.xmlsoap.org/ws/2004/08/eventing/Renew"
#define EVT_ACTION_FAULT          \
           "http://schemas.xmlsoap.org/ws/2004/08/eventing/fault"




#define WSMAN_ACTION_EVENTS       \
           "http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Events"
#define WSMAN_ACTION_HEARTBEAT     \
           "http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Heartbeat"
#define WSMAN_ACTION_DROPPEDEVENTS     \
           "http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/DroppedEvents"
#define WSMAN_ACTION_ACK     \
           "http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Ack"
#define WSMAN_ACTION_EVENT     \
           "http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Event"



#define WSENUM_ACTION_FAULT     \
           "http://schemas.xmlsoap.org/ws/2004/09/enumeration/fault"
#define WSENUM_SUBSCRIBE                  "Subscribe"
#define WSENUM_SUBSCRIBE_RESP             "SubscribeResponse"
#define WSENUM_UNSUBSCRIBE                "UnSubscribe"
#define WSENUM_UNSUBSCRIBE_RESP           "UnSubscribeResponse"
#define WSENUM_GETSTATUS               	  "GetStatus"
#define WSENUM_GETSTATUS_RESP             "GetStatusResponse"

#define WSENUM_RENEW                  "Renew"
#define WSENUM_RENEW_RESP             "RenewResponse"

#define WSENUM_GET_STATUS             "GetStatus"
#define WSENUM_GET_STATUS_RESP        "GetStatusResponse"

#define WSMAN_ACTION_FAULT      \
         "http://schemas.dmtf.org/wbem/wsman/1/wsman/fault"




#define WSENUM_END_TO                 "EndTo"
#define WSENUM_EXPIRES                "Expires"
#define WSENUM_FILTER                 "Filter"
#define WSENUM_DIALECT                "Dialect"
#define WSENUM_ENUMERATION_CONTEXT    "EnumerationContext"
#define WSENUM_MAX_TIME               "MaxTime"
#define WSENUM_MAX_ELEMENTS           "MaxElements"
#define WSENUM_MAX_CHARACTERS         "MaxCharacters"
#define WSENUM_ITEMS                  "Items"
#define WSENUM_END_OF_SEQUENCE        "EndOfSequence"
#define WSENUM_ENUMERATION_END        "EnumerationEnd"
#define WSENUM_REASON                 "Reason"
#define WSENUM_CODE                   "Code"
#define WSENUM_SOURCE_SHUTTING_DOWN   "SourceShuttingDown"
#define WSENUM_SOURCE_CANCELING       "SourceCanceling"





#define WSMB_ASSOCIATED_INSTANCES   "AssociatedInstances"
#define WSMB_ASSOCIATION_INSTANCES  "AssociationInstances"
#define WSMB_OBJECT                 "Object"
#define WSMB_ASSOCIATION_CLASS_NAME "AssociationClassName"
#define WSMB_RESULT_CLASS_NAME      "ResultClassName"
#define WSMB_ROLE                   "Role"
#define WSMB_RESULT_ROLE            "ResultRole"
#define WSMB_INCLUDE_RESULT_PROPERTY "IncludeResultProperty"


#define CIM_RESOURCE_NS_PREFIX	 	"p"


#define WSM_SYSTEM                     "System"
#define WSM_LOCALE                     "Locale"
#define WSM_RESOURCE_URI               "ResourceURI"
#define WSM_SELECTOR_SET               "SelectorSet"
#define WSM_SELECTOR                   "Selector"
#define WSM_NAME                       "Name"
#define WSM_REQUEST_TOTAL              "RequestTotalItemsCountEstimate"
#define WSM_TOTAL_ESTIMATE             "TotalItemsCountEstimate"
#define WSM_OPTIMIZE_ENUM              "OptimizeEnumeration"
#define WSM_MAX_ELEMENTS               "MaxElements"
#define WSM_ENUM_EPR                   "EnumerateEPR"
#define WSM_ENUM_OBJ_AND_EPR           "EnumerateObjectAndEPR"
#define WSM_ENUM_MODE                  "EnumerationMode"
#define WSM_ITEM                       "Item"
#define WSM_INCLUDE_RESULT_PROPERTY             "IncludeResultProperty"
#define WSM_FRAGMENT_TRANSFER          "FragmentTransfer"
#define WSM_XML_FRAGMENT               "XmlFragment"
#define WSM_OPTION_SET             	"OptionSet"
#define WSM_OPTION             		"Option"

#define WSM_MAX_ENVELOPE_SIZE           "MaxEnvelopeSize"
#define WSM_OPERATION_TIMEOUT           "OperationTimeout"
#define WSM_FAULT_SUBCODE               "FaultSubCode"
#define WSM_FILTER                      "Filter"
#define WSM_DIALECT                     "Dialect"


#define WXF_RESOURCE_CREATED                      "ResourceCreated"


// WSMB - Binding
#define WSMB_POLYMORPHISM_MODE          "PolymorphismMode"
#define WSMB_INCLUDE_SUBCLASS_PROP      "IncludeSubClassProperties"
#define WSMB_EXCLUDE_SUBCLASS_PROP      "ExcludeSubClassProperties"
#define WSMB_NONE      "None"
#define WSMB_DERIVED_REPRESENTATION     "DerivedRepresentation"
#define WSMB_ACTION_FAULT     		"http://schemas.dmtf.org/wbem/wsman/1/cimbinding/fault"
#define WSMB_SHOW_EXTENSION		"ShowExtensions"


// Catalog

#define WSMANCAT_RESOURCE               "Resource"
#define WSMANCAT_RESOURCE_URI           "ResourceUri"
#define WSMANCAT_VERSION                "Version"
#define WSMANCAT_NOTES                  "Notes"
#define WSMANCAT_VENDOR                 "Vendor"
#define WSMANCAT_DISPLAY_NAME           "DisplayName"
#define WSMANCAT_KEYWORDS               "Keywords"
#define WSMANCAT_ACCESS                 "Access"
#define WSMANCAT_RELATIONSHIPS          "Relationsships"
#define WSMANCAT_COMPLIANCE             "Compliance"
#define WSMANCAT_OPERATION              "Operation"
#define WSMANCAT_SELECTOR_SET           "SelectorSet"
#define WSMANCAT_SELECTOR               "Selector"
#define WSMANCAT_OPTION_SET             "OptionSet"
#define WSMANCAT_ACTION                 "Action"
#define WSMANCAT_SELECTOR_SET_REF       "SelectorSetRef"
#define WSMANCAT_LOCATION               "Location"
#define WSMANCAT_NAME                   "Name"
#define WSMANCAT_TYPE                   "Type"


// Filter Dialects
#define WSM_SELECTOR_FILTER_DIALECT     \
        "http://schemas.dmtf.org/wbem/wsman/1/wsman/SelectorFilter"
#define WSM_WQL_FILTER_DIALECT          \
        "http://schemas.microsoft.com/wbem/wsman/1/WQL"
#define WSM_XPATH_FILTER_DIALECT        \
        "http://www.w3.org/TR/1999/REC-xpath-19991116"
#define WSM_ASSOCIATION_FILTER_DIALECT     \
        "http://schemas.dmtf.org/wbem/wsman/1/cimbinding/associationFilter"


#define WSFW_RESPONSE_STR              "Response"
#define WSFW_INDOC                     "indoc"

#define WSFW_ENUM_PREFIX               "_en."


#define CIM_NAMESPACE_SELECTOR          "__cimnamespace"
#define CIM_ALL_AVAILABLE_CLASSES	"http://schemas.dmtf.org/wbem/wscim/1/*"
#define XML_NS_CIM_ALL_CLASS		"http://schemas.dmtf.org/wbem/wscim/1"

//static char XML_END1[3] =  {0x0d,0x0a,0x00};
//static char XML_END2[5] =  {0x0d,0x0a,0x0d,0x0a,0x00};

#endif

