<?php

function _seotools_presets() {
  return array(
    'seotools' => array(
      'title' => 'SEO Tools',
      'description' => 'Presets for SEO optimization',
      'file' => 'plugins/seotools.presets.inc',
      'modules_footer' => l('Download seo modules', 'http://leveltendesign.com') . ' from LevelTen Interactive',
      'modules' => array(
        'seotools' => array(
          'name' => 'SEO Tools',
          'footer' => 'To change the settings, go to ' . l('admin/settings/seotools', 'admin/settings/seotools'),
          'settings' => array(
            'seotools_uid',
            'seotools_feature',
          ),
        ),
        'path' => array(
          'name' => 'Path (Core)',
        ),
        'taxonomy' => array(
          'name' => 'Taxonomy (Core)',
        ),
        'features' => array(
          'name' => 'Features',
        ),
        'strongarm' => array(
          'name' => 'Strongarm',
        ),
        'wordstream' => array(
          'name' => 'Wordstream',
          'version' => '6.x-1.0-alpha1',
          'footer' => 'To change the settings, go to ' . l('admin/settings/wordstream', 'admin/settings/wordstream'),
          'settings' => array(
            'wordstream_username',
            'wordstream_free_embed_id',
          ),
        ),
        'nodewords' => array(
          'name' => 'Node Words',
          'version' => '6.x-1.11',
          'submodules' => array(
            'nodewords_extra',
            'nodewords_verification_tags',
            'nodewords_basic',
          ),
        ),
        'page_title' => array(
          'name' => 'Page Title',
          'version' => '6.x-2.3',
          'settings' => array(
            'nodes',
          ),
          'footer' => 'To change the settings, go to ' . l('admin/content/page_title', 'admin/content/page_title'),
        ),
        'globalredirect' => array(
          'name' => 'Global Redirect',
          'version' => '6.x-1.2',
        ),
        'xmlsitemap' => array(
          'name' => 'XML Sitemap',
          'version' => '6.x-2.0-beta1',
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
          'name' => 'Content Analysis',
          'version' => '6.x-1.4',
        ),
        'contentoptimizer' => array(
          'name' => 'Content Optimizer',
          'version' => '6.x-2.1',
        ),
        'alchemy' => array(
          'name' => 'Alchemy',
          'version' => '6.x-1.0-rc4',
          'submodules' => array(
            'alchemy_contentanalysis',
          ),
          'settings' => array(
            'alchemy_apikey',
          ),
          'footer' => 'To change the settings, go to ' . l('admin/settings/alchemy', 'admin/settings/alchemy'),
        ),
        'kwresearch' => array(
          'name' => 'Keyword Research',
          'version' => '6.x-1.0-rc4',
          'submodules' => array(
            'kwresearch_google',
          ),
          'settings' => array(
            'kwresearch_apikey',
            'kwresearch_keyword_sync_vocabulary',
          ),
          'footer' => 'To change the settings, go to ' . l('admin/settings/kwresearch', 'admin/settings/kwresearch'),
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
  // Check if the user already exists.
  $result = db_query("SELECT uid FROM {users} WHERE name='%s'", 'SEO Tools');
  if($row = db_fetch_array($result)) {
    $uid = $row['uid'];
  }
  // If the user was not found, generate a new user.
  if (!isset($uid)) {
    $details = array(
      'name' => 'SEO Tools',
      'pass' => substr(md5(rand()), 0, 10),
      'mail' => 'manager@seotools.com',
      'access' => 0,
      'status' => 1
    );
    $user = user_save(NULL, $details);
    $uid = $user->uid;
    drupal_set_message('A user was generated for you.');
  }
  if(!variable_get('seotools_uid', 0)) {
    variable_set('seotools_uid', $uid);
  }
}

function seotools_seotools_seotools_feature() {
  if (module_exists('features')) {
    module_load_include('inc', 'features', 'features.export');
    $status = features_get_storage('seotools');
    $setting = array();
    $setting['title'] = 'SEO Feature';
    if ($status == 0) {
      $setting['value'] = 'Feature is default';
      $setting['severity'] = 0;
    }
    else {
      $setting['value'] = 'Feature is overridden';
      $setting['description'] = l('Review overrides' , 'admin/build/features/seotools');
      $setting['severity'] = REQUIREMENT_ERROR;
    }

    return $setting;
  }
}

function seotools_seotools_seotools_feature_reset() {
  module_load_include('inc', 'features', 'features.export');
  features_include();
  $module = 'seotools';
  $seotools = features_get_features($module);
  $revert = array();
  foreach ($seotools->info['features'] as $component => $status) {
    $revert[$module][] = $component;
    drupal_set_message(t('Reverted all <strong>!component</strong> components for <strong>!module</strong>.', array('!component' => $component, '!module' => $module)));
  }
  features_revert($revert);

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

function seotools_wordstream_wordstream_username() {
  $setting = array();
  $setting['title'] = 'Wordstream API Account';
  if (variable_get('wordstream_username', '') == '' || variable_get('wordstream_password', '') == '') {
    $setting['value'] = drupal_get_form('seotools_wordstream_wordstream_username_form');
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  else {
    $setting['value'] = 'Wordstream API Username set to: ' . variable_get('wordstream_username', '');
    $setting['severity'] = REQUIREMENT_OK;
  }

  return $setting;
}

function seotools_wordstream_wordstream_username_form() {
  $form['wordstream_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Wordstream API Username'),
    '#default_value' => variable_get('wordstream_username', ''),
  );
  $form['wordstream_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Wordstream API Password'),
    '#default_value' => variable_get('wordstream_password', ''),
    '#description' => t('In order to enable the API tools you will need a WordStream account login (username & password). !wordstream_link.',
      array(
        '!wordstream_link' => l(t('Get your API account here'), WORDSTREAM_LINK_API_ACCOUNT, array('attributes' => array('target', 'wordstream'))),
      )
    ),
  );
  return system_settings_form($form);
}

function seotools_wordstream_wordstream_free_embed_id() {
  $setting = array();
  $setting['title'] = 'Free widgets embed id';
  if (variable_get('wordstream_free_embed_id', '') == '') {
    $setting['value'] = drupal_get_form('seotools_wordstream_wordstream_free_embed_id_form');
    $setting['severity'] = REQUIREMENT_ERROR;
  }
  else {
    $setting['value'] = 'Wordstream Free widgets embed id: ' . variable_get('wordstream_free_embed_id', '');
    $setting['severity'] = REQUIREMENT_OK;
  }

  return $setting;
}

function seotools_wordstream_wordstream_free_embed_id_form() {
  $form['wordstream_free_embed_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Free widgets embed id'),
    '#description' => t(''),
    '#default_value' => variable_get('wordstream_free_embed_id', ''),
    '#description' => t('In order to enable the free javascript tools you will need a WordStream widget embed id. <br>The embed id is a part of the widget embed code that you can generate on the WordStream site. <br>Enter in the field above the 20 character value following "embed_id=" in the widget embed code.<br> !wordstream_link.',
      array(
        '!wordstream_link' => l(t('Get your embed code here'), WORDSTREAM_LINK_WIDGET_GENERATOR, array('attributes' => array('target', 'wordstream'))),
      )
    )
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
  // First search if the vocabulary has already been created.
  $vocabularies = taxonomy_get_vocabularies();
  if (is_array($vocabularies)) {
    foreach($vocabularies as $vid => $info) {
      if ($info->name == 'Keywords') {
        $tags = (array) $info;
      }
    }
  }
  // If it doesn't exist yet, create it.
  if (!isset($tags)) {
    $tags = array(
      'name' => 'Keywords',
      'description' => 'generated vocabulary for keyword content',
      'tags' => 1,
      'relations' => 1,
      'module' => 'taxonomy',
      'nodes' => array_combine(array_keys(node_get_types()), array_keys(node_get_types())),
    );
    taxonomy_save_vocabulary($tags);
    drupal_set_message('A keyword vocabulary was generated for you.');
  }
  // Set the variables for the tags vocabulary.
  variable_set('kwresearch_keyword_sync_vocabulary', $tags['vid']);
  $report_vocabs = variable_get('kwresearch_report_vocabulary', array());
  $report_vocabs[$tags['vid']] = $tags['vid'];
  variable_set('kwresearch_report_vocabulary', $report_vocabs);
}

