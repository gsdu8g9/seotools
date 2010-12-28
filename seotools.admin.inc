<?php
// $Id: seotools.admin.inc,v 1.2 2010/09/26 00:36:50 tomdude48 Exp $
/**
 * @file * Admin include file. 
*/

/** 
 * Displays the form for the standard settings tab.
 * 
 * @return array 
 * A structured array for use with Forms API. 
*/ 
function seotools_admin_settings() {
  global $base_url;
  $form['seotools_base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Google Analytics Base Url'),
    '#default_value' => variable_get('seotools_base_url', $base_url),
    '#description' => t('If you have a different url on google_analytics, enter it here (defaults to base_url'),
  );
  
  //Add the system buttons to the form
  return system_settings_form($form);
}
