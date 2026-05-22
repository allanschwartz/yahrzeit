
/**
 *  @file   GlobalSettings.js
 *
 *  JavaScript support functions call from the Global Settings screen
 *
 *  @version
 *  $Id: $
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
 *    146    function addNewManagerAddress()
 *    279    function addSnmpElem(listObj, newElem, hidAttr, ifld)
 *    253    function getSnmpList(listObj, hidAttr)
 *     45    function initGS(formObj)
 *     64    function loadSnmpToList (hiddenObj, listObj)
 *    198    function validateDomainComponent (vfld, ifld, reqd)
 *     83    function validateGS(formObj)
 *    227    function validateLicCollectAndSample()

 */

/**
 *  initGS -- called onLoad from the GlobalSettings form
 *
 *  @param formObj       the form object
 *
 *  @return
 *      no return value
 */
function initGS(formObj)
{
    with (formObj) {
    }
}

/**
 *  loadSnmpToList -- called onLoad from the GlobalSettings form
 *
 *  @param hiddenObj    space char separated hidden list of SNMP Ports 
 *  @param listObj      Form Element to display the SNMP Ports list
 *
 *  @return
 *      no return value
 */
function loadSnmpToList (hiddenObj, listObj)
{
    if (hiddenObj.value != "") {
        var eleArray = hiddenObj.value.split(" ");
        for (var i=0; i<eleArray.length; i++) {
            listObj.options[listObj.length] = new Option(eleArray[i], eleArray[i]);
        }
    }
}

/**
 *  validateGS --  called onSubmit from the Global Settings form
 *
 *  @param formObj       the form object
 *
 *  @return
 *      \c true     validation successfull  \n
 *      \c false    validation not sucessfull
 */
function validateGS(formObj)
{
    with (formObj) {


//        // Product Email ID validation
//        if (!validateNumberInRange(licenseAlertsMaxPerDay, 'licenseAlertsMaxPerDayErr', false, 1, 10))
//            return false;
//
//        if (!validateAlphaNumeric(ldapUserId, 'ldapUserIdErr', false))
//            return false;
//
//        if (!validateAlphaNumeric(imapUserId, 'userIdErr', false))
//            return false;
//
//        if (!validateNumberInRange(hmgCliTimeout, 'hmgCliTimeoutErr', false, 1, 99) )
//            return false;
//
//        if (!validateNumberInRange(hpgCliTimeout, 'hpgCliTimeoutErr', false, 1, 99) )
//            return false;

        return true;
    }
}


/**
 *  addNewManagerAddress -- Add a manager to the list of managers
 *
 *  @param
 *
 *  @return
 *      no return value
 */
function addNewManagerAddress()
{
    with (document.forms[0]) {
        var selectSize =  trapManagerList.options.length;
        if (!validateIPv4Address(newIp, "newManagerAddressErr", true))
            return;
        if (!validatePortNumber(newPort, "newManagerAddressErr", true))
            return;
        if (!validateAlphaNumeric(newCommunity, "newManagerAddressErr", true))
            return;
                
        var v = newIp.value + ":" +
                newPort.value + ":" +
                newCommunity.value;

        for (i=0;i< trapManagerList.length;i++) {
            if (v ==  trapManagerList.options[i].value) {
                msg("newManagerAddressErr", "errorText", "<fmt:message key='webclient.geminimobile.common.js.duplicateNotAllowed' />");
                return;
            }
        }

        setAllowedHosts(hiddenTrapManagerList, trapManagerList);

        var newLen = hiddenTrapManagerList.value.length + v.length + 1;
        if (newLen > 255)
        {
          msg ("newManagerAddressErr", "errorText", "<fmt:message key='webclient.common.maxlength.exceed255'/>");
          newIp.focus();
            return;
        }

        var newEntry = new Option(v, v);
        trapManagerList.options[selectSize] = newEntry;
        newIp.value = "";
        newPort.value = "";
        newCommunity.value = "";
    }
}

/**
 *  validateDomainComponent -- make sure domainComponent contains pair
 *           in the format \c dc=alphanumeric[,dc=alnum]
 *
 *  @param
 *      vfld     element to be validated
 *      ifld     id of the element to receive info/error msg
 *      reqd     true if required
 *
 *  @return
 *      no return value
 */
function validateDomainComponent (vfld, ifld, reqd)
{
    var stat = commonCheck (vfld, ifld, reqd);
    if (stat != proceed)
        return stat;

    var tfld = trim(vfld.value);    // value of field with whitespace trimmed off
    var dcArr = tfld.split(",");
    var dc= /dc=[A-Za-z0-9]{1,}/;
    for (var i = 0; i < dcArr.length; i++) {
        if (!dc.test(dcArr[i])) {
            msg (ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.invalidDomainComponent'/>");
            vfld.focus();
            return false;
        }
    }
    msg (ifld, "text", "");
    return true;
}

/**
 *  validateLicCollectAndSample -- make sure lic collect interval is a multiple
 *      of lic sample
 *
 *  @param
 *
 *  @return
 *      no return value
 */
function validateLicCollectAndSample()
{
    var coll = document.forms[0].licLogCollectInterval.value * 60;
    var sample = document.forms[0].licenseLogSampleInterval.value;
    var remainder = coll % sample;
    if (remainder != 0) {
        msg ("licLogCollectIntervalErr", "errorText", "<fmt:message key='webclient.admin.globalsettings.licCollectSampleInvalid'/>");
        document.forms[0].licLogCollectInterval.focus();
        return false;
    }
    msg ("licLogCollectIntervalErr", "text", "");
    return true;
}

/**
 *  getSnmpList -- Loads the token-seperated list, in a text input element, to
 *      a hidden parameter
 *
 *  @param
 *      listObj     typically a select list form element
 *      hidAttr     the hidden parameter
 *      strToken    seperator, typically a "," or " "
 *
 *  @return
 *      no return value
 */
function getSnmpList(listObj, hidAttr)
{
    var size = listObj.options.length;
    hidAttr.value = "";
    if (size == 0) {
        return;
    }
    for (var i=0; i<size-1; i++) {
       hidAttr.value  += listObj.options[i].value + " ";
    }

    hidAttr.value += listObj.options[size-1].value;
}

/**
 *  addSnmpElem -- add the list address element 'newElem' to the hidden attribute
 *
 *  @param
 *      listObj     the text element, typically a text input form element
 *      newElem     the new Element to be added to the hidAttr
 *      hidAttr     the hidden parameter containing the elements
 *      ifld        id of the element to receive info/error msg
 *
 *  @return
 *      no return value
 */
function addSnmpElem(listObj, newElem, hidAttr, ifld)
{
    var selectSize =  listObj.options.length;
    var newElement = trim(newElem.value);

    // check that newElement is non-blank
    if ( newElement == "" ) {
        return;
    }

    if (!validatePortNumber(newElem, ifld, false)) {
        return;
    }
    var spaceChar = " ";
    getSnmpList(listObj, hidAttr);

    var v = newElem.value;
    var newLen = hidAttr.value.length + v.length + 1;
    if (newLen > 255)
    {
      msg (ifld, "errorText", "<fmt:message key='webclient.common.maxlength.exceed255'/>");
      newElem.focus();
        return;
    }

     // check that newListElement is unique in the list
    for (i=0; i< listObj.length; i++) {
        if (v ==  listObj.options[i].value) {
            msg(ifld, "errorText", "<fmt:message key='webclient.geminimobile.common.js.duplicateNotAllowed' />");
            return false;
        }
    }

    listObj.options[selectSize] = new Option(newElement, newElement);
    newElem.value = null;
}

