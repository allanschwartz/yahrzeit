
/**
 *  @file   CommonValidation.js
 *
 *  JavaScript functions called from all screens to validate form data
 *
 *  @version
 *  $Id: CommonValidation.js,v 1.31 2006/12/11 22:55:26 rekha Exp $
 *
 *  @author
 *  Stephen Poley
 *  Velankani Team: vbabu, bvsrini, msarin, mkhanna
 *  Gemini Team: allan@geminimobile.com, rekha@geminimobile.com
 *
 *
 *  Copyright (c) 2004, 2005 Gemini Mobile Technologies
 *  All rights reserved.
 *
 *  This software is the confidential and proprietary information of Gemini
 *  Mobile Technologies.  You shall not disclose such confidential information
 *  and shall use it only in accordance with the terms of the license agreement
 *  you entered into with Gemini Mobile Technologies.
 */
/*
 *  CONTENTS
 *
 *   line    Function Desclarations
 *  -----    -----------------------------------------------------------
 *           function trim (str)
 *           function msg (fld, msgtype, message)
 *           function commonCheck (vfld, ifld, reqd)
 *           function validatePresent (vfld, ifld )
 *           function validateNonEmpty (hidden_field, item_field, ifld )
 *           function validateObjectPerRegex (vfld, ifld, reqd, validRegex, errmsg)
 *           function validateEmail (vfld, ifld, reqd)
 *           function validateEmailOrMsisdn (vfld, ifld, reqd)
 *           function validateIPv4Address (vfld, ifld, reqd)
 *           function validateFQDN (vfld, ifld, reqd)
 *           function validateURL (vfld, ifld, reqd)
 *           function isalpha_charcode( c )
 *           function isnumeric_charcode( c )
 *           function validateCharURL (vfld)
 *           function validateMSISDN (vfld, ifld, reqd)
 *           function validateDigits (vfld, ifld, reqd)
 *           function validateAlphaNumeric (vfld, ifld, reqd)
 *           function validateFileName (vfld, ifld, reqd)
 *           function validateFileNameSafeString (vfld, ifld, reqd)
 *           function validateIdentifier (vfld, ifld, reqd)
 *           function validatePositiveInteger (vfld, ifld, reqd)
 *           function validateNumberField (vfld, ifld, reqd)
 *           function validatePositiveNumber (vfld, ifld, reqd)
 *           function validateNumberInRangeGeneric (vfld, ifld, reqd, minnum, maxnum, errMsg)
 *           function validateNumberInRange (vfld, ifld, reqd, minnum, maxnum)
 *           function validateRangeNumber (vfld, ifld, reqd, maxnum)
 *           function convertToSeconds(vfld1, ufld1)

 */

var nbsp = 160;             // non-breaking space char
var node_text = 3;          // DOM text node-type
emptyString = /^\s*$/;

/**
 *  Trim leading/trailing whitespace off string
 *
 *  @param str     the value
 *
 *  @return
 *      trimmed value
 *
 */
function trim (str)
{
    return str.replace (/^\s+|\s+$/g, '');
}

/**
 *  Display warn/error message in HTML element
 *  commonCheck routine must have previously been called
 *
 *  @param fld         id of element to display message in
 *  @param msgtype     class to give element ("text" or "errorText")
 *  @param message     string to display
 *
 *  @return
 *      no return value
 */
function msg (fld, msgtype, message)
{
    // setting an empty string can give problems if later set to a
    // non-empty string, so ensure a space present. (For Mozilla and Opera one could
    // simply use a space, but IE demands something more, like a non-breaking space.)
    var dispmessage;
    if (emptyString.test (message) )
        dispmessage = String.fromCharCode (nbsp);
    else
        dispmessage = message;

    var elem = document.getElementById (fld);
    elem.firstChild.nodeValue = dispmessage;
    elem.className = msgtype;
}

/**
 *  Common code for all validation routines to:
 * (a) check for older / less-equipped browsers
 * (b) check if empty fields are required
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      true (validation passed),
 *      false (validation failed) or
 *      proceed (don't know yet)
 */
var proceed = 2;

function commonCheck (vfld, ifld, reqd)
{
    if (!document.getElementById)
        return true;        // not available on this browser - leave validation to the server
    var elem = document.getElementById (ifld);
    if (!elem.firstChild)
        return true;        // not available on this browser
    if (elem.firstChild.nodeType != node_text)
        return true;        // ifld is wrong type of node

    if (emptyString.test (vfld.value) ) {
        if (reqd) {
            msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.valueIsRequired'/>");
            vfld.focus ();
            return false;
        }
        else {
            msg (ifld, "text", "");   // OK
            return true;
        }
    }
    return proceed;
}

/**
 *  Validate if something has been entered
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *
 *  @return
 *      \c true if present \n
 *      \c false if invalid
 */
function validatePresent (vfld, ifld )
{
    var stat = commonCheck (vfld, ifld, true);
    if (stat != proceed)
        return stat;

    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate that a list (a comma-seperated list, typically)
 *    is non-empty.
 *  The second argument is a related control, where focus is
 *    placed, if the first element is found empty.
 *
 *  @param hidden_field   element to be validated, a list object
 *  @param item_field     where items are inputted, which create the list object
 *  @param ifld           id of element to receive info/error msg
 *
 *  @return
 *      \c true if present \n
 *      \c false if invalid
 */
function validateNonEmpty (hidden_field, item_field, ifld )
{
    // we can't call commonCheck, because the default "move focus" logic
    // doesn't work with a hidden field
    if (!document.getElementById)
        return true;        // not available on this browser - leave validation to the server
    if (emptyString.test (hidden_field.value) ) {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.valueIsRequired'/>");
        item_field.focus ();
        return false;
    }

    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate if field is a positive Integer with max length 15 digits. 
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *  @param validRegex     validation regular expression
 *  @param errmsg   error message to display
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateObjectPerRegex (vfld, ifld, reqd, validRegex, errmsg)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed) 
        return stat;

    var tfld = trim (vfld.value);       // value of field with whitespace trimmed off
    if (!validRegex.test (tfld) ) {
        msg (ifld, "errorText", errmsg);
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate for e-mail address
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateEmail (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);  // value of field with whitespace trimmed off

    //var email = /^([\w\d\-\.]+)@{1}(([\w\d\-]{1,67})|([\w\d\-]+\.[\w\d\-]{1,67}))\.(([a-zA-Z\d]{1,67})(\.[a-zA-Z\d]{2,4})?)$/;
    /* per RFC 1035 */
    /* each component of the DNS name is called a "label".
     *      each label begins with a letter followed by a sequence up letters,hyphens,and digits, and lastly a letter or digit
     *
     *       <label> ::= <letter> [ [ <ldh-str> ] <let-dig> ]
     *       <ldh-str> ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
     *       <let-dig-hyp> ::= <let-dig> | "-"
     *       <let-dig> ::= <letter> | <digit>
     *       Limit the label to 63 octets or less. Each label is represented as a one octet length field
     */

     /*      in the following regex, the <label>  is   [a-zA-Z]([\w\-])*[\w]
      */
    var validEmail = /^([\w\d\-\.]+)@{1}([a-zA-Z]([\w\-]*[\w]))((\.[a-zA-Z]([\w\-]*[\w])){0,60})\.([a-zA-Z\d]{2,4})(\.[a-zA-Z\d]{2})?$/;

    if (!validEmail.test (tfld) ) {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidEmailAddress'/>");
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate for e-mail address or msisdn number
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateEmailOrMsisdn (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);  // value of field with whitespace trimmed off

    // For regular expression details  see validateEmail() function above
    var validEmail = /^([\w\d\-\.]+)@{1}([a-zA-Z]([\w\-]*[\w]))((\.[a-zA-Z]([\w\-]*[\w])){0,60})\.([a-zA-Z\d]{2,4})(\.[a-zA-Z\d]{2})?$/;
    var validMsisdn = /^[0-9]{2,15}$/;

    if (!validEmail.test (tfld) && !validMsisdn.test (tfld) ) {
        msg (ifld, "errorText","<fmt:message key='webclient.geminimobile.common.js.invalidEmailandMSISDN'/>");
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate IPv4 Address
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateIPv4Address (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);  // value of field with whitespace trimmed off
    var IPAddress = /^(([01]?\d?\d|2[0-4]\d|25[0-5])\.){3}([01]?\d?\d|2[0-4]\d|25[0-5])$/;
    if (!IPAddress.test (tfld) ) {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidIPAddress'/>");
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate a fully qualified domain name (without http://)
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateFQDN (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);  // value of field with whitespace trimmed off
    var fqdn = /^([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,6}$/;
    if (!fqdn.test (tfld) ) {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidFQDN'/>");
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate if URL is correct (http, ftp https and protocolless)
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateURL (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);  // value of field with whitespace trimmed off
    if (validateCharURL (vfld) ) {
        var url = /^(http|ftp|https):\/\/[\w-_]+(\.[\w-_]+)*/;
        if (!url.test (tfld) ) {
            msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidURL'/>");
            vfld.focus ();
            return false;
        }
        msg (ifld, "text", "");
        return true;
    }
    else {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidURL'/>");
        vfld.focus ();
        return false;
    }
}

/**
 *  character classification
 *
 *  @param c        single character
 *
 *  @return
 *      \c true if classified \n
 *      \c false if not
 */
function isalpha_charcode( c )
{
    //   "a"             "z"
    if ( 97 <= c && c <= 123 )
        return true;
    //   "A"             "Z"
    if ( 65 <= c && c <= 90 )
        return true;
    return false;
}

/**
 *  character classification
 *
 *  @param c        single character
 *
 *  @return
 *      \c true if classified \n
 *      \c false if not
 */
function isnumeric_charcode( c )
{
    //   "0"             "9"
    if ( 48 <= c && c <= 57 )
        return true;
    return false;
}

/**
 *  called from validateURL function
 *  Validate if characters entered for URL valid
 *  characters allowed (a-z, 0-9, -, _, :, @, ?, /)
 *
 *  @param vfld     element to be validated
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateCharURL (vfld)
{
    var tfld = vfld.value;
    for ( i = 0; i < tfld.length; i++ ) {
        var ch = tfld.charAt(i);
        var charCode  = tfld.charCodeAt(i);
        if ( !( isalpha_charcode( charCode )  || isnumeric_charcode( charCode ) ||
                (ch == ":") || (ch == "/") || (ch == ".") ||
                (ch == "-") || (ch == "-") || (ch == "?") ) ) {
            return false;
        }
    }
    return true;
}

/**
 *  Validate if field is a MSISDN number
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateMSISDN (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);       // value of field with whitespace trimmed off
    var validMsisdn = /^[0-9]{1,15}$/;
    if (!validMsisdn.test (tfld) ) {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidMSISDNNumber'/>");
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate if field is a positive Integer (just a sequence of digits)
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateDigits (vfld, ifld, reqd)
{
    var validDigits = /^[0-9]{1,15}$/;
    var errmsg = "<fmt:message key='webclient.geminimobile.common.js.invalidNumber'/>";

    return validateObjectPerRegex (vfld, ifld, reqd, validDigits, errmsg);
}

/**
 *  Validate AlphaNumeric
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateAlphaNumeric (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = vfld.value;

    for ( i=0; i<tfld.length; i++ ) {
        var ch = tfld.charAt(i);
        var charCode  = tfld.charCodeAt(i);
        if ( !( isalpha_charcode( charCode )  || isnumeric_charcode( charCode ) ||
                (ch == "-") || (ch == "_") ||
                (ch == ".") || (ch == " ") ) ) {
            msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.notAlphaNumeric'/>");
            vfld.focus ();
            return false;
        }
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate FileName with out blank space
 *
 *  XXX not referenced anywhere
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateFileName (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = vfld.value;

    for ( i=0; i<tfld.length; i++ ) {
        var ch = tfld.charAt(i);
        var charCode  = tfld.charCodeAt(i);
        if ( i == 0 ) {
            if ( !isalpha_charcode( charCode )  ) {
                msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidIdentifier1'/>");
                vfld.focus ();
                return false;
            }
        }
        else if ( !( isalpha_charcode( charCode )  || isnumeric_charcode( charCode ) ||
                    (ch == "-") || (ch == "_") || (ch == ".") ) ) {
                msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidIdentifier2'/>");
                vfld.focus ();
                return false;
        }
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate if field is file name safe
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateFileNameSafeString (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);  // value of field with whitespace trimmed off
    var filenameSafeString = /^[A-Za-z][-_\.A-Za-z0-9]*$/;
    if (!filenameSafeString.test (tfld) ) {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidIdentifier2'/>");
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  Validate if field is a legal "IDENTIFIER"
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateIdentifier (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim (vfld.value);  // value of field with whitespace trimmed off
    var filenameSafeString = /^[A-Za-z][-_\.A-Za-z0-9]*$/;
    if (!filenameSafeString.test (tfld) ) {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidIdentifier3'/>");
        vfld.focus ();
        return false;
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  ValidatePositiveInteger, that the field is a positive integer
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validatePositiveInteger (vfld, ifld, reqd)
{
    return validateNumberField (vfld, ifld, reqd);
}

/**
 *  Validate Number, that the field is a positive integer
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateNumberField (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = vfld.value;  // value of field with whitespace trimmed off

    if ((tfld > -1)  && (tfld.indexOf (".") == -1) && (tfld.indexOf("+") == -1) ){
        msg (ifld, "text", "");
        return true;
    }
    else {
        msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.notPositiveNum'/>");
        vfld.focus ();
        return false;
    }
}


/**
 *  Validate that the field is a positive real or integer number
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validatePositiveNumber (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = vfld.value;  // value of field with whitespace trimmed off

    if (tfld > 0) {
        msg (ifld, "text", "");
        return true;
    }
    else {
        if (dsplErr) {
            msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.notPositiveNum'/>");
            vfld.focus ();
        }
        return false;
    }
}


/**
 *  validate that a number v is in the range
 *
 *  minnum <=  v  &&  v <= maxnum
 *
 *  This is the most generic number validator ... three other number validators
 *  call this one, typically with unique error messages
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *  @param minnum   min port number value
 *  @param maxnum   max port number value
 *  @param errMsg   the message to display if the number is not in range
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateNumberInRangeGeneric (vfld, ifld, reqd, minnum, maxnum, errMsg)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = vfld.value;          // value of field with whitespace trimmed off

    if (tfld.indexOf (".")== -1 && tfld.indexOf("+") == -1 && tfld >= eval (minnum) && tfld <= eval (maxnum) ) {
        msg (ifld, "text", "");
        return true;
    }
    else {
        msg (ifld, "errorText", errMsg);
        vfld.focus ();
        return false;
    }
}

/**
 *  validate that a number v is in the range
 *
 *  minnum <=  v  &&  v <= maxnum
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *  @param minnum   min port number value
 *  @param maxnum   max port number value
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateNumberInRange (vfld, ifld, reqd, minnum, maxnum)
{
    var errMsg = "<fmt:message key='webclient.geminimobile.common.js.outOfMinMaxRange'/>" + " " + minnum + "-" + maxnum ;
    return validateNumberInRangeGeneric (vfld, ifld, reqd, minnum, maxnum, errMsg);
}

/**
 *  validate that a number v is in the range
 *
 *  0 <=  v  &&  v <= maxnum
 *
 *  @param vfld     element to be validated
 *  @param ifld     id of element to receive info/error msg
 *  @param reqd     true if required
 *  @param maxnum   max port number value
 *
 *  @return
 *      \c true if valid \n
 *      \c false if invalid
 */
function validateRangeNumber (vfld, ifld, reqd, maxnum)
{
    var errMsg = "<fmt:message key='webclient.geminimobile.common.js.outOfRange'/>" + maxnum ;
    return validateNumberInRangeGeneric (vfld, ifld, reqd, 0, maxnum, errMsg);
}


/**
 *  Convert the time values into seconds for comparing
 *
 *  @param vfld     element value to be converted into seconds
 *  @param ufld     Unit of element
 *
 *  @return
 *      \c value in seconds \n
 *      \c false if invalid
 */
function convertToSeconds(vfld1, ufld1)
{
    var valueInSeconds;
    switch (ufld1.value.toUpperCase())
    {
        case "SECONDS":
            valueInSeconds = vfld1.value;
            break;
        case "MINUTES":
            valueInSeconds = vfld1.value*60;
            break;
        case "HOURS":
            valueInSeconds = vfld1.value*60*60;
            break;
        case "DAYS":
            valueInSeconds = vfld1.value*60*60*24;
            break;
        default:
            return false;
    }
    return valueInSeconds;
}

