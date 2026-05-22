
/**
 *  @file   CommonMisc.js
 *
 *  Globally usefull, but misc., JavaScript support functions
 *
 *  @version
 *  $Id: $
 *
 *  @author
 *  allan@geminimobile.com
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
 *    277    function changeButtonClass(button, isButtonDisabled )
 *    261    function disallowMultipleClicks() 
 *    238    function enableObj (checkObj, objChange)
 *    213    function setOperation(operationValue)

 */


/**
 *  To change the class of disabled buttons to "buttonDisabled"
 *
 *  @param button
 *  @param isButtonDisabled     Boolean
 *
 *  @return none
 */
function changeButtonClass(button, isButtonDisabled )
{
    if (isButtonDisabled == true) {
        button.disabled = true;
        button.className = "buttonDisabled";
    }
    else {
        button.disabled = false;
        button.className = "button";
    }
}


//variable to check the number of submits on the page.
var clickCounter = 0;

/**
 *  To disallow multiple clicks on the submit buttons
 *
 *  @param none
 *
 *  @return
 *      return boolean true if submit is allowed
 */
function disallowMultipleClicks() {
    clickCounter++;
    if (clickCounter > 1) {
        return false;
    }
    return true;
}


/**
 *  To enable the element objChange if the corresponding element
 *  checkObj is checked by the user.
 *
 *  @param checkObj    form element, a checkbox
 *  @param objChange   any dependent form object
 *
 *  @return
 *      no return value
 */
function enableObj (checkObj, objChange)
{
    if (checkObj.checked == true)
        objChange.disabled = false;
    else
        objChange.disabled = true;
        if (objChange.type == "select-one") {
            objChange.multiple = false;
            objChange.size = 0;
        }
}


/**
 *  Set the Operation form parameter
 *
 *  @param operationValue
 *
 *  @return
 */
function setOperation(operationValue)
{
    document.forms[0].operation.value=operationValue;
}


