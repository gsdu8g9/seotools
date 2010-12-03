<?php

function _seotools_presets() {
  return array(
    'seotools' => array(
      'title' => 'SEO Tools',
      'description' => 'Presets for SEO optimization',
      'file' => 'plugins/seotools.presets.inc',
      'modules_footer' => l('Download all modules', 'http://leveltendesign.com') . ' from LevelTen Design',
      'modules' => array(
        'seotools' => array(
          'name' => 'SEO Tools',
          'footer' => 'To change the settings, go to ' . l('admin/settings/seotools', 'admin/settings/seotools'),
          'settings' => array(
            'seotools_uid'
          ),
        ),
        'wordstream' => array(
          'name' => 'Wordstream',
        ),
        'nodewords' => array(
          'name' => 'Node Words',
          'submodules' => array(
            'nodewords_extra',
            'nodewords_verification_tags',
            'nodewords_basic',
          ),
        ),
        'page_title' => array(
          'name' => 'Page Title',
          'settings' => array(
            'nodes',
          ),
        ),
        'globalredirect' => array(
          'name' => 'Global Redirect'
        ),
        'xmlsitemap' => array(
          'name' => 'XML Sitemap',
          'submodules' => array(
            'xmlsitemap_custom',
            'xmlsitemap_node',
            'xmlsitemap_engines',
            'xmlsitemap_user',
            'xmlsitemap_menu',
            'xmlsitemap_taxonomy',
           ),
        ),
        'contentanalysis' => array(
          'name' => 'Content Analysis'
        ),
        'contentoptimizer' => array(
          'name' => 'Content Optimizer'
        ),
        'scribeseo' => array(
          'name' => 'Scribe SEO',
          'settings' => array(
            'scribeseo_apikey',
          ),
        ),
        'alchemy' => array(
          'name' => 'Alchemy',
          'submodules' => array(
            'alchemy_contentanalysis',
          ),
          'settings' => array(
            'alchemy_apikey',
          ),
        ),
        'kwresearch' => array(
          'name' => 'Keyword Research',
          'submodules' => array(
            'kwresearch_google',
          ),
          'settings' => array(
            'kwresearch_apikey',
            'kwresearch_keyword_sync_vocabulary',
          ),
        ),
      ),
    ),
  );
}

function seotools_seotools_seotools_uid() {
  $setting = array();
  if (variable_get('seotools_uid', 0) == 0) {
    $setting['title'] = 'SEO User';
    $setting['value'] = drupal_get_form('seotools_seotools_seotools_uid_form');
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  else {
    $account = user_load(variable_get('seotools_uid', 0));
    $setting['title'] = 'SEO User';
    $setting['value'] = 'User set to: ' . $account->name . ' (' . $account->uid . ')';
    $setting['severity'] = 0;
  }

  return $setting;
}

function seotools_seotools_seotools_uid_form() {
  // Populate the users array
  $users = array();
  $result = db_query("select uid, name from {users} LIMIT 100");
  while($row = db_fetch_array($result)) {
    $users[$row['uid']] = $row['name'];
  }
  $users[0] = 'anonymous';

  $form['seotools_uid'] = array(
    '#type' => 'select',
    '#title' => t('Autotag User'),
    '#default_value' => variable_get('seotools_uid', 0),
    '#options' => $users,
    '#description' => '<div>When autotagging with cron, a user needs to be assigned to automatically tag content.',
  );
  return system_settings_form($form);
}

function seotools_seotools_seotools_uid_reset() {
  if(!variable_get('seotools_uid', 0)) {
    $details = array(
      'name' => 'SEO Tools',
      'pass' => substr(md5(rand()), 0, 10),
      'mail' => 'manager@seotools.com',
      'access' => 0,
      'status' => 1
    );

    $user = user_save(NULL, $details);
    variable_set('seotools_uid', $user->uid);
    drupal_set_message('A user was generated for you.');
  }
}

function seotools_page_title_nodes() {
  $titles_set = array();
  $titles_not_set = array();
  $types = node_get_types();
  foreach ($types as $type) {
    // Define the node-type key
    $key = 'page_title_type_'. $type->type . '_showfield';
    if (variable_get($key, 0)) {
      $titles_set[$type->type] = $type->name;
    }
    else {
      $titles_not_set[$type->type] = $type->name;
    }
  }
  $setting = array();
  $setting['title'] = 'Node Page Titles';
  if (empty($titles_set)) {
    $setting['value'] = 'No content types are set to use page titles';
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  elseif (!empty($titles_not_set)) {
    $account = user_load(variable_get('seotools_uid', 0));
    $setting['value'] = 'Not using Page Titles: ' . implode(', ', $titles_not_set);
    $setting['severity'] = REQUIREMENT_WARNING;
  }
  else {
    $account = user_load(variable_get('seotools_uid', 0));
    $setting['value'] = 'All content types are set up to use page titles.';
    $setting['severity'] = REQUIREMENT_OK;
  }

  return $setting;
}

function seotools_page_title_nodes_reset() {
  $types = node_get_types();
  foreach ($types as $type) {
    // Define the node-type key
    $key = 'page_title_type_'. $type->type . '_showfield';
    variable_set($key, 1);
  }
}

function seotools_scribeseo_scribeseo_apikey() {
  $setting = array();
  $setting['title'] = 'API Key';
  if (variable_get('scribeseo_apikey', '') == '') {
    $setting['value'] = drupal_get_form('seotools_scribeseo_scribeseo_apikey_form');
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  else {
    $setting['value'] = 'Scribe SEO API Key set to: ' . variable_get('scribeseo_apikey', 0);
    $setting['severity'] = REQUIREMENT_OK;
  }

  return $setting;
}

function seotools_scribeseo_scribeseo_apikey_form() {
  $form['scribeseo_apikey'] = array(
    '#type' => 'textfield',
    '#title' => t('Scribe API Key'),
    '#default_value' => variable_get('scribeseo_apikey', ''),
    '#description' => t('You will need a Scribe SEO API Key to enable analysis. <a href="' . SCRIBESEO_LINK_GETKEY . '" target="_blank">Get your API key here</a>.'),
  );
  return system_settings_form($form);
}

function seotools_alchemy_alchemy_apikey() {
  $setting = array();
  $setting['title'] = 'API Key';
  if (variable_get('alchemy_apikey', '') == '') {
    $setting['value'] = drupal_get_form('seotools_alchemy_alchemy_apikey_form');
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  else {
    $setting['value'] = 'Alchemy API Key set to: ' . variable_get('alchemy_apikey', '');
    $setting['severity'] = REQUIREMENT_OK;
  }

  return $setting;
}

function seotools_alchemy_alchemy_apikey_form() {
  $form['alchemy_apikey'] = array(
    '#type' => 'textfield',
    '#title' => t('Alchemy API Key'),
    '#default_value' => variable_get('alchemy_apikey', ''),
    '#description' => t('You need an API key to use the Alchemy API service. !alchemy_link.',
      array(
        '!alchemy_link' => l(t('Get an Alchemy API key here'), 'http://www.alchemyapi.com/api/register.html', array('attributes' => array('target' => '_blank'))),
      )
    ),
  );
  return system_settings_form($form);
}

function seotools_kwresearch_kwresearch_apikey() {
  $setting = array();
  $setting['title'] = 'API Key';
  if (variable_get('kwresearch_apikey', '') == '') {
    $setting['value'] = drupal_get_form('seotools_kwresearch_kwresearch_apikey_form');
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  else {
    $setting['value'] = 'Keyword Reasearch API Key set to: ' . variable_get('kwresearch_apikey', '');
    $setting['severity'] = REQUIREMENT_OK;
  }

  return $setting;
}

function seotools_kwresearch_kwresearch_apikey_form() {
  $form['kwresearch_apikey'] = array(
    '#type' => 'textfield',
    '#title' => t('Wordtracker API Key'),
    '#default_value' => variable_get('kwresearch_apikey', ''),
    '#description' => t('You need an API key to enable keyword statistics and related phrase lookups. <br>If you have a Wordtracker API key enter it above. <br>To get a full API key, contact !wordtracker_link. You can also use a free !levelten_link <br>to enable demo level access to Wordtracker statitics.',
      array(
        '!wordtracker_link' => l(t('Wordtracker'), 'http://www.wordtracker.com/api', array('attributes' => array('target' => '_blank'))),
        '!levelten_link' => l(t('LevelTen SEO API key'), 'http://www.leveltendesign.com/user/0/l10seoapi', array('attributes' => array('target' => '_blank'))),
      )
    ),
  );
  return system_settings_form($form);
}

function seotools_kwresearch_kwresearch_keyword_sync_vocabulary() {
  $setting = array();
  $setting['title'] = 'Keyword Vocabulary';
  if (variable_get('kwresearch_keyword_sync_vocabulary', 0) == 0) {
    $setting['value'] = drupal_get_form('seotools_kwresearch_kwresearch_keyword_sync_vocabulary_form');
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  else {
    $vocab = taxonomy_vocabulary_load(variable_get('kwresearch_keyword_sync_vocabulary', 0));
    $setting['value'] = 'Vocabulary set to: ' . $vocab->name . ' (' . $vocab->vid . ')';
    $setting['severity'] = 0;
  }

  return $setting;
}

function seotools_kwresearch_kwresearch_keyword_sync_vocabulary_form() {
  // Populate the vocab_options array
  $vocabs = taxonomy_get_vocabularies();
  $vocab_options = array(0 => '<none>');
  foreach ($vocabs as $id => $vocab) {
    $vocab_options[$vocab->vid] = $vocab->name;
  }

  $form['kwresearch_keyword_sync_vocabulary'] = array(
    '#type' => 'select',
    '#title' => 'Vocabulary for Autotagging Keywords',
    '#options' => $vocab_options,
    '#default_value' => variable_get('kwresearch_keyword_sync_vocabulary', 0),
    '#description' => '<div>In order to use autotagging, it must be associated with a vocabulary.  Please select a vocabulary to use.</div><div> ' . l('Automatically generate a vocabulary', 'admin/settings/seotools/settings/kwresearch/taxonomy') . '</div>',
  );
  return system_settings_form($form);
}

function seotools_kwresearch_kwresearch_keyword_sync_vocabulary_reset() {
  $tags = array(
    'name' => 'Keywords',
    'description' => 'generated vocabulary for keyword content',
    'tags' => 1,
    'relations' => 1,
    'module' => 'taxonomy',
    'nodes' => array_combine(array_keys(node_get_types()), array_keys(node_get_types())),
  );
  taxonomy_save_vocabulary($tags);
  variable_set('kwresearch_keyword_sync_vocabulary', $tags['vid']);
  $report_vocabs = variable_get('kwresearch_report_vocabulary', array());
  $report_vocabs[$tags['vid']] = $tags['vid'];
  variable_set('kwresearch_report_vocabulary', $report_vocabs);
  drupal_set_message('A keyword vocabulary was generated for you.');
}

