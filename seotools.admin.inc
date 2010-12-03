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
  $form['autotag']['seotools_autotag_alchemy_relevance_min'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimal relevancy'),
    '#default_value' => variable_get('seotools_autotag_alchemy_relevance_min', 50),
    '#description' => t('Use this to specify the minimum relevance a phrase must have in order to be included. Set between 0 - 100.'),
    '#field_suffix' => t('%'),
    '#size' => 3,
  );
  
  $form['autotag']['seotools_autotag_alchemy_usecache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use cached data if available.'), 
    '#default_value' => variable_get('seotools_autotag_alchemy_usecache', TRUE),
    '#description' => t('The responses from Alchemy are cached. If you want to enable fetching data from cache rather than fetching from the Alchemy API, check the above box. This setting is useful when Tagging is used with Content Analysis or Autotagging modules are both enabled since since those modules will refresh the cache.'),
    '#prefix' => '<div id="alchemy_tagging_suggest_cache">',
    '#suffix' => '</div>',  
  );
  
  $form['autotag']['seotools_autotag_batch_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Items per batch'),
    '#default_value' => variable_get('seotools_autotag_batch_limit', 10),
    '#description' => t('Specify how many nodes you want to process per batch (e.g. cron) run.'),
    '#size' => 3,
  );
  
  //Add the system buttons to the form
  $form = system_settings_form($form);  

  
  //Overide the theme function back to our own one
  return $form;
}
