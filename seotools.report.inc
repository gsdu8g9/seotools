<?php
// $Id:$

function seotools_dashboard_page($range = 30) {
  drupal_add_css(drupal_get_path('module', 'seotools') . '/seotools.css');
  drupal_add_js(drupal_get_path('module', 'seotools') . '/seotools.js');

  $gid = variable_get('google_analytics_api_profile_id', 0);
  if(!$gid) {
    return t('Google Analytics id not set.');
  }
  // TODO make this dynamic
  $siteURL = 'http://www.leveltendesign.com/';

  $e = explode('-', $range);
  if(count($e) == 1) {
    $date_range = seotools_get_dates($range);
  }
  else {
    // TODO allow custom ranges
  }

  $output .= '<div id="seotools-dashboard">' . "\n";
  $output .= '<div id="dashboard-pane-top">' . "\n";
  $output .= seotools_dashboard_analytics_box($gid, $siteURL, $date_range);
  $output .= '</div>' . "\n";
  $output .= '<div id="dashboard-pane-left">' . "\n";
  $output .= seotools_dashboard_keywords_box($gid, $siteURL, $date_range);
  $output .= seotools_dashboard_sources_box($gid, $siteURL, $date_range, $mediums_data);
  //$output .= seotools_dashboard_sources_box($gid, $siteURL, $date_range, $filtered_mediums);
  $output .= '</div>' . "\n";
  $output .= '<div class="dashboard-spacer">&nbsp;</div>';
  $output .= '<div id="dashboard-pane-right">' . "\n";
  $output .= seotools_dashboard_content_box($gid, $siteURL, $date_range);
  $output .= seotools_dashboard_referrer_box($gid, $siteURL, $date_range, $mediums_data);
  $output .= '</div>' . "\n";
  $output .= '</div>' . "\n";
  return $output;
}

function seotools_dashboard_analytics_box($gid, $siteURL, $date_range) {
  if($output = seotools_cache_get('output_analytics_box', $date_range)) {
    $js = seotools_cache_get('js_analytics_box', $date_range);
    drupal_add_js( array('seotools' => $js), 'setting');
    return $output;
  }
  // Grab the data.
  if($rows = seotools_cache_get('data_ga_summary', $date_range, 6)) {
    $rows = unserialize($rows);
  }
  else {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('date'),
      //'#metrics' => array('visits', 'pageviews', 'bounces', 'uniquePageviews'),
      '#metrics' => array('visits', 'pageviews', 'timeOnSite', 'bounces', 'uniquePageviews', 'newVisits', 'goalCompletionsAll'),
      '#sort_metric' => array('date'),
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $rows = seotools_fetch_ga_data($request);
    seotools_cache_set('data_ga_summary', $rows, $date_range);
  }
  // Check for data.
  if (!$rows) {
    return '<p>No analytics data is currently available.</p>';
  }

  // Format and perform calculations to display charts.

  $totals = array(
    'visits' => 0,
    'goals' => 0,
    'pageviews' => 0,
    'average_pageviews' => 0,
    'bounces' => 0,
    'time_on_site' => 0,
    'vistor_type' => 0,
    'goals_per_visit' => 0,
  );
  $day_counts = array();
  foreach ($rows as $date => $row) {
    $day = array();
    $day_counts['visits'][] = $row['visits'];
    $totals['visits'] += $row['visits'];

    $day_counts['pageviews'][] = $row['pageviews'];
    $totals['pageviews'] += $row['pageviews'];

    $day_counts['average_pageviews'][] = $row['pageviews'] / $row['visits'];

    $day_counts['bounce_rate'][] = 100 * $row['bounces'] / $row['uniquePageviews'];
    $totals['bounces'] += $row['bounces'];

    $day_counts['time_on_site'][] = $row['timeOnSite'] / $row['visits'];
    $totals['time_on_site'] += $row['timeOnSite'];

    $day_counts['vistor_type'][] = 100 * $row['newVisits'] / $row['visits'];
    $totals['vistor_type'] += $row['newVisits'];

    $day_counts['goals'][] = $row['goalCompletionsAll'];
    $totals['goals'] += $row['goalCompletionsAll'];

    $day_counts['goal_conversion_rate'][] = 100 * $row['goalCompletionsAll'] / $row['visits'];

    $chart_dates[] = date('d', strtotime($date));
  }

  $output .= '<h4 id="chart-main-title">' . t('Visits') . '</h4>';
  $output .= '<div class="chart-main-wrapper">';
  //$output .= '<img src="' . $chart . '" />';
  $output .= '</div>';

  $output .= '<h3>' . t('Site Usage') . '</h3>';

  $links[] = l(t('analytics'), 'https://www.google.com/analytics/reporting/dashboard', array('query' => "id=$gid", 'attributes' => array('target' => 'googleanalytics')));
  $links[] = l(t('webmaster tools'), 'https://www.google.com/webmasters/tools/dashboard', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));


  $output .= '<div class="dashboard-links">';
  $output .= implode(' | ', $links);
  $output .= '</div>';

  $totals['time_on_site'] = seotools_format_time_on( $totals['time_on_site'] / $totals['visits']);
  $totals['bounce_rate'] = number_format($totals['bounces'] / $totals['visits'] * 100, 2) . "%";
  $totals['vistor_type'] = number_format($totals['vistor_type'] / $totals['visits'] * 100, 2) . "%";
  $totals['visits'] = number_format($totals['visits']);
  $totals['pageviews'] = number_format($totals['pageviews']);
  $totals['average_pageviews'] = number_format($totals['pageviews'] / $totals['visits'], 2);
  $totals['goals'] = number_format($totals['goals']);
  $totals['goal_conversion_rate'] = number_format($totals['goals'] / $totals['visits'] / 10, 2) . "%";


  $cells = array(
    'visits' => t('Visits'),
  'bounce_rate' => t('Bounce Rate'),
    'pageviews' => t('Pageviews'),
  'time_on_site' => t('Avg. Time on Site'),
    'average_pageviews' => t('Pages/Visit'),
    'vistor_type' => t('% New Visits'),
  'goals' => t('Goals'),
  'goal_conversion_rate' => t('Goal Conversion'),
  );
  foreach($cells AS $key => $title) {
    $output .= '<div id="' . $key . '-cell" class="dashboard-ga-cell">' . "\n";
    $output .= '<a href="#" onclick="seotools_show_main_chart(\'' . $key . '_main\')">';
    $output .= chart_render(_seotools_reports_micro_chart($key . '_micro', '', $day_counts[$key], $chart_dates));
    $output .= '</a>';
    $output .= ' ' . $totals[$key];
    $queryadd = '';
    if($key == 'goal_conversion_rate') {
      $queryadd = '&goal=-1';
    }
    $output .= ' ' . l($title, 'https://www.google.com/analytics/reporting/' . $key, array('query' => "id=$gid$queryadd", 'attributes' => array('target' => 'googleanalytics')));
    $output .= '</div>' . "\n";
    $id = $key . '_main';
    $chart = chart_url(_seotools_reports_liquid_chart($id, '', $day_counts[$key], $chart_dates));
    $js['charts'][$id] = array(
      'src' => $chart,
      'location' => 'chart-main',
      'title' => $title,
    );
    if($id != 'visits_main') {
      $js['charts'][$id]['load_only'] = 1;
    }
  }
  drupal_add_js( array('seotools' => $js), 'setting');
  seotools_cache_set('js_analytics_box', $js, $date_range);
  seotools_cache_set('output_analytics_box', $output, $date_range);
  return $output;
}

function seotools_format_time_on($time) {
  $h = floor($time / 3600);
  $m = floor(($time - ($h * 3600)) / 60);
  $s = $time % 60;
  return (($h < 10)?'0':'')."$h:".(($m < 10)?'0':'')."$m:".(($s < 10)?'0':'').$s;
}

function seotools_get_dates($code = 30) {
  $date_range = array();
  $date_range['days'] = $code;
  if($code == 1) {
    $date_range['start_date'] = strtotime('-1 days');
    $date_range['end_date'] = strtotime('-1 days');
  }
  else if($code == 0) {
    $date_range['start_date'] = strtotime();
    $date_range['end_date'] = time();
    $date_range['days'] = .5;
  }
  else if($code == 7) {
    $date_range['start_date'] = strtotime('-7 days');
    $date_range['end_date'] = strtotime('-1 days');
  }
  else if($code == 6) {
    $date_range['start_date'] = strtotime('-7 days');
    $date_range['end_date'] = time();
    $date_range['days'] = 6.5;
  }
  else if($code == 90) {
    $date_range['start_date'] = strtotime('-90 days');
    $date_range['end_date'] = strtotime('-1 days');
  }
  else if($code == 89) {
    $date_range['start_date'] = strtotime('-90 days');
    $date_range['end_date'] = time();
    $date_range['days'] = 90.5;
  }
  else if($code == 365) {
    $date_range['start_date'] = strtotime('-365 days');
    $date_range['end_date'] = strtotime('-1 days');
  }
  else if($code == 364) {
    $date_range['start_date'] = strtotime('-364 days');
    $date_range['end_date'] = time();
    $date_range['days'] = 364.5;
  }
  else if($code == 29) {
    $date_range['start_date'] = strtotime('-29 days');
    $date_range['end_date'] = time();
    $date_range['days'] = 29.5;
  }
  else {
    $date_range['start_date'] = strtotime('-30 days');
    $date_range['end_date'] = strtotime('-1 days');
  }
  $date_range['start_date'] = date("Y-m-d", $date_range['start_date']);
  $date_range['end_date'] = date("Y-m-d", $date_range['end_date']);
  return $date_range;
}

function seotools_cache_get($cid, $date_range = NULL, $records = NULL) {
  // don't cache if end date is today
  if($date_range['end_date'] == date("Y-d-m")) {
    return FALSE;
  }
  $where = "cid = '%s'";
  $args[] = $cid;
  if($date_range['start_date']) {
    $where .= " AND start_date = '%s'";
    $args[] = $date_range['start_date'];
  }
  if($date_range['end_date']) {
    $where .= " AND end_date = '%s'";
    $args[] = $date_range['end_date'];
  }
  if($records) {
    $where .= " AND records >= %d";
    $args[] = $records;
  }
  $sql = "
    SELECT data
    FROM {seotools_cache}
    WHERE $where
  ";
  $data = db_result(db_query($sql, $args));
  if($data) {
    $ret = unserialize($data);
    return $ret;
  }
  else {
    return FALSE;
  }
}

function seotools_cache_set($cid, $data, $date_range = NULL) {
  $where = "cid = '%s'";
  $args[] = $cid;

  if($date_range['start_date']) {
    $fields .= ", start_date";
    $values .= ", '%s'";
    $where .= " AND start_date = '%s'";
    $args[] = $date_range['start_date'];
  }
  if($date_range['end_date']) {
    $fields .= ", end_date";
    $values .= ", '%s'";
    $where .= " AND end_date = '%s'";
    $args[] = $date_range['end_date'];
  }
  $sql = "
    DELETE FROM {seotools_cache}
    WHERE $where
  ";
  db_query($sql, $args);
  if(is_array($data)) {
    $fields .= ", records";
    $values .= ", %d";
    $args[] = count($data);
  }
  $args[] = serialize($data);
  $sql = "
    INSERT INTO {seotools_cache}
    (cid$fields, data)
    VALUES
    ('%s'$values,'%s')
  ";
  db_query($sql, $args);
  return TRUE;
}



function _seotools_reports_liquid_chart($id, $title, $data, $dates) {
  $chart = array(
    '#chart_id' => $id,
    '#title' => '',
    '#type' => CHART_TYPE_LINE,
    '#size' => chart_size(600, 150),
    '#adjust_resolution' => TRUE,
    '#data_colors' => chart_data_colors(array('0077CC')),
    '#line_styles' => array(chart_line_style(3, 10, 0)),
    '#shape_markers' => array(chart_shape_marker(0, -1, 'o', 9, '0077CC')),
    '#grid_lines' => chart_grid_lines(0, 20, 1, 3),
  );
  $chart['#data'] = $data;
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, max($data));
  foreach ($dates as $date) {
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label($date);
  }
  return $chart;
}

function _seotools_reports_multiline_chart($id, $title, $data, $dates, $colors) {
  $shade_colors = array(
    '058DC7' => 'E8F8FF',
    '50B432' => 'DDF4D7',
    'ED561B' => 'FCE4D9',
    'EDEF00' => 'FFFFD6'
  );
  $line_styles = array();
  $shape_markers = array();
  $i = 0;
  foreach($data AS $series) {
    $line_styles[] = chart_line_style(3);
    //$shape_markers[] = chart_shape_marker($i, -1, 'B', 6, $colors[$i]);
    $shape_markers[] = chart_shape_marker($i, -1, 'B', 6, $shade_colors[$colors[$i]]);
    $i++;
  }
  $chart = array(
    '#chart_id' => $id,
    '#title' => '',
    '#type' => CHART_TYPE_LINE,
    '#size' => chart_size(600, 150),
    '#adjust_resolution' => TRUE,
    '#data_colors' => $colors,
    '#line_styles' => $line_styles,
    '#shape_markers' => $shape_markers,
    '#grid_lines' => chart_grid_lines(0, 20, 1, 3),
  );
  $chart['#data'] = $data;
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, max($data));
  foreach ($dates as $date) {
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label($date);
  }
  return $chart;
}

function _seotools_reports_micro_chart($id, $title, $data, $dates) {
  $chart = array(
    '#chart_id' => $id,
    '#title' => '',
    '#type' => CHART_TYPE_LINE,
    '#size' => chart_size(80, 20),
    '#adjust_resolution' => TRUE,
    '#data_colors' => array('0077cc'),
    '#line_styles' => array(chart_line_style(2, 10, 0)),
    '#shape_markers' => array(chart_shape_marker(0, -1, 'B', 9, 'E8F8FF')),
    //'#grid_lines' => chart_grid_lines(0, 20, 1, 3),
  );
  $chart['#data'] = $data;
  //$chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, max($data));
  foreach ($dates as $date) {
    //$chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label($date);
  }
  return $chart;
}

/*
 * Function to collect together data for different aliases.
 */
function seotools_fetch_ga_data($request,  $group_by = 'date') {
  $data = google_analytics_api_report_data($request);

  // Add up all the statistics for different paths.
  $rows = array();
  foreach ($data as $item) {
    $dimensions = $item->getDimensions();
    if (is_array($group_by)) {
      $group_name = '';
      foreach ($group_by as $group_by_item) {
        $group_name .= $dimensions[$group_by_item];
      }
    }
    else {
      $group_name = $dimensions[$group_by];
    }
    if (isset($rows[$group_name])) {
      foreach ($item->getMetrics() as $key => $value) {
        $rows[$group_name][$key] += $value;
      }
    }
    else {
      $rows[$group_name] = $item->getMetrics();
    }
  }
  return $rows;
}



function seotools_dashboard_keywords_box($gid, $siteURL, $date_range) {

  $output = '<h3 class="keywords">' . t('Keywords') . '</h3>';

  $links[] = l(t('analytics'), 'https://www.google.com/analytics/reporting/keywords', array('query' => "id=$gid", 'attributes' => array('target' => 'googleanalytics')));
  $links[] = l(t('research'), 'admin/content/kwresearch/keyword_report', array('attributes' => array('target' => 'kwresearch')));
  $links[] = l(t('search queries'), 'https://www.google.com/webmasters/tools/top-search-queries', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));
  $links[] = l(t('found'), 'https://www.google.com/webmasters/tools/keywords', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));


  $output .= '<div class="dashboard-links">';
  $output .= implode(' | ', $links);
  $output .= '</div>';

  if(!$data = seotools_cache_get('data_ga_keywords', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('keyword'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 20,
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $data = seotools_fetch_ga_data($request, 'keyword');
    seotools_cache_set('data_ga_keywords', $data, $date_range);
  }
  $kw_priority_options = kwresearch_get_priority_options();
  $c = 1;
  foreach($data AS $keyword => $value) {
    if($keyword == '(not set)') {
      continue;
    }
    $kw_obj = kwresearch_load_site_keyword($keyword);
    $rows[] = array(
      l(seotools_format_text_data($keyword), 'https://www.google.com/analytics/reporting/keyword_detail', array('query' => "id=$gid&d1=$keyword", 'attributes' => array('target' => 'googleanalytics'))),
      array(
        'data' => number_format($value['visits']),
        'class' => 'numeric-cell',
      ),
      array(
        'data' => ($kw_obj->priority) ? $kw_priority_options[$kw_obj->priority] : '[not set]',
        'class' => 'text-cell',
      ),
    );
    $c++;
    if($c > 5) {
      break;
    }
  }
  $header = array(
    array('data' => t('Top traffic'), 'class' => 'label-header'),
    array('data' => t('Visits'), 'class' => 'numeric-header'),
    array('data' => t('Priority'), 'class' => 'text-header'),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-highest-traffic-keywords'));

  $filter = array();
  $options['mode'] = 'site';
  $header[] = array('data' => t('Priority'), 'field' => 'k.priority', 'sort' => 'desc');
  $result = kwresearch_load_filtered_keyword_result($filter, $options, $header = array(), $limit = 100000);
  $kw_counts = array(
    'top' => 0,
    'high' => 0,
    'standard' => 0,
    'total' => 0,
  );
  while($row = db_fetch_object($result)) {
    $kw_counts['total']++;
    if ($row->priority > 80) {
      $kw_counts['top']++;
    }
    else if ($row->priority > 60) {
      $kw_counts['high']++;
    }
    else if ($row->priority > 40) {
      $kw_counts['standard']++;
    }
  }
  $rows = array();
  $header = array(
    array('data' => t('By priority'), 'class' => 'label-header'),
    array('data' => t('Count'), 'class' => 'numeric-header')
  );

  $rows[] = array(
    l(t('All site keywords'), 'admin/content/kwresearch/keyword_list/site'),
    array(
      'data' => number_format($kw_counts['total']),
      'class' => 'numeric-cell',
    ),
  );
  $rows[] = array(
    '&nbsp;&nbsp;' . l(t('Top'), 'admin/content/kwresearch/keyword_list/site', array('query' => 'filters={"priority":90}', 'attributes' => array('target' => 'kwresearch'))),
    array(
      'data' => number_format($kw_counts['top']),
      'class' => 'numeric-cell',
    ),
  );
  $rows[] = array(
    '&nbsp;&nbsp;' . l(t('High'), 'admin/content/kwresearch/keyword_list/site', array('query' => 'filters={"priority":70}', 'attributes' => array('target' => 'kwresearch')))
    . '|' . l(t('& up'), 'admin/content/kwresearch/keyword_list/site', array('query' => 'filters={"priority":{"1":90,"2":70}}', 'attributes' => array('target' => 'kwresearch'))),
    array(
      'data' => number_format($kw_counts['high']),
      'class' => 'numeric-cell',
    ),
  );
  $rows[] = array(
    '&nbsp;&nbsp;' . l(t('Standard'), 'admin/content/kwresearch/keyword_list/site', array('query' => 'filters={"priority":50}', 'attributes' => array('target' => 'kwresearch')))
    . '|' . l(t('& up'), 'admin/content/kwresearch/keyword_list/site', array('query' => 'filters={"priority":{"1":90,"2":70,"3":50}}', 'attributes' => array('target' => 'kwresearch'))),
    array(
      'data' => number_format($kw_counts['standard']),
      'class' => 'numeric-cell',
    ),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-keyword-box'));
  return $output;
}

function seotools_dashboard_content_box($gid, $siteURL, $date_range) {
  $output = '<h3 class="content">' . t('Content') . '</h3>';

  $links[] = l(t('analytics'), 'https://www.google.com/analytics/reporting/content', array('query' => "id=$gid", 'attributes' => array('target' => 'googleanalytics')));
  $links[] = l(t('optimize'), 'admin/reports/seo/contentanalysis', array('query' => "sort=desc&order=Quick+SEO", 'attributes' => array('target' => 'seo_friend')));
  $links[] = l(t('html suggestions'), 'https://www.google.com/webmasters/tools/html-suggestions', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));
  $links[] = l(t('crawl errors'), 'https://www.google.com/webmasters/tools/crawl-errors', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));

  $output .= '<div class="dashboard-links">';
  $output .= implode(' | ', $links);
  $output .= '</div>';

  if(!$data = seotools_cache_get('data_ga_content', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('pagePath'),
      '#metrics' => array('pageviews'),
      '#sort_metric' => array('-pageviews'),
      '#max_results' => 20,
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $data = seotools_fetch_ga_data($request, 'pagePath');
    seotools_cache_set('data_ga_content', $data, $date_range);
  }
  $kw_priority_options = kwresearch_get_priority_options();
  $c = 1;
  $base_path = base_path();
  foreach($data AS $page => $value) {
    if($keyword == '(not set)') {
      continue;
    }
    $path = str_replace($base_path, '', $page);
    $page_settings = linkintel_load_page_settings($path);
    $rows[] = array(
      l(seotools_format_text_data($page), 'https://www.google.com/analytics/reporting/content_detail', array('query' => "id=$gid&d1=$page", 'attributes' => array('target' => 'googleanalytics'))),
      array(
        'data' =>  number_format($value['pageviews']),
        'class' => 'numeric-cell',
      ),
      array(
        'data' =>  ($page_settings->priority) ? $kw_priority_options[$page_settings->priority] : '[not set]',
        'class' => 'text-cell',
      ),
    );
    $c++;
    if($c > 5) {
      break;
    }
  }
  $header = array(
    array('data' => t('Top hits'), 'class' => 'label-header'),
    array('data' => t('Visits'), 'class' => 'numeric-header'),
    array('data' => t('Priority'), 'class' => 'text-header'),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-highest-traffic-keywords'));

  // content priority report
  //$filter = array(
  //  'where' => '(effective_priority >= 40 OR p.priority >= 0)',
  //  'args' => array(),
  //);
  $header[] = array('data' => t('Priority'), 'field' => 'effective_priority', 'sort' => 'desc');
  $result = linkintel_load_filtered_pages_result($filter, $options, $header = array(), $limit = 100000);
  $counts = array(
    'top' => 0,
    'high' => 0,
    'standard' => 0,
    'total' => 0,
  );
  while($row = db_fetch_object($result)) {
    $counts['total']++;
    if ($row->effective_priority > 80) {
      $counts['top']++;
    }
    else if ($row->effective_priority > 60) {
      $counts['high']++;
    }
    else if ($row->effective_priority > 40) {
      $counts['standard']++;
    }
  }
  $rows = array();
  $header = array(
    array('data' => t('By priority'), 'class' => 'label-header'),
    array('data' => t('Count'), 'class' => 'numeric-header'),
  );

  $rows[] = array(
    l(t('All prioritized pages'), 'admin/content/linkintel/pages'),
    array(
      'data' => number_format($counts['total']),
      'class' => 'numeric-cell',
    ),
  );
  $rows[] = array(
    '&nbsp;&nbsp;' . l(t('Top'), 'admin/content/linkintel/pages', array('query' => 'filters={"priority":90}', 'attributes' => array('target' => 'linkintel'))),
    array(
      'data' => number_format($counts['top']),
      'class' => 'numeric-cell',
    ),
  );
  $rows[] = array(
    '&nbsp;&nbsp;' . l(t('High'), 'admin/content/linkintel/pages', array('query' => 'filters={"priority":70}', 'attributes' => array('target' => 'linkintel')))
    . '|' . l(t('& up'), 'admin/content/linkintel/pages', array('query' => 'filters={"priority":{"1":90,"2":70}}', 'attributes' => array('target' => 'linkintel'))),
    array(
      'data' => number_format($counts['high']),
      'class' => 'numeric-cell',
    ),
  );
  $rows[] = array(
    '&nbsp;&nbsp;' . l(t('Standard'), 'admin/content/linkintel/pages', array('query' => 'filters={"priority":50}', 'attributes' => array('target' => 'linkintel')))
    . '|' . l(t('& up'), 'admin/content/linkintel/pages', array('query' => 'filters={"priority":{"1":90,"2":70,"3":50}}', 'attributes' => array('target' => 'linkintel'))),
    array(
      'data' => number_format($counts['standard']),
      'class' => 'numeric-cell',
    ),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-keyword-box'));

  return $output;
}

function seotools_dashboard_sources_box($gid, $siteURL, $date_range, &$mediums_data) {
  // Query for browser usage information.

  $output = '<h3 class="referrers">' . t('Sources') . '</h3>';

  $links[] = l(t('analytics'), 'https://www.google.com/analytics/reporting/sources', array('query' => "id=$gid", 'attributes' => array('target' => 'googleanalytics')));
  $links[] = l(t('subscriber stats'), 'https://www.google.com/webmasters/tools/subscribers', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));

  $output .= '<div class="dashboard-links">';
  $output .= implode(' | ', $links);
  $output .= '</div>';

  if(!$data = seotools_cache_get('data_ga_mediums', $date_range)) {
    $request = array(
      '#dimensions' => array('medium'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 20,
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $data = seotools_fetch_ga_data($request, 'medium');
    seotools_cache_set('data_ga_mediums', $data, $date_range);
  }

  $mediums = array();
  $total_visits = 0;
  foreach ($data as $key => $value) {
    if($key == '(none)') {
      $key = 'direct';
    }
    $total_visits += $value['visits'];
    $mediums[$key] = $value['visits'];
  }

  // Any browsers with a marketshare below 0.1% don't get shown.
  $filtered_mediums = array();
  $filtered_mediums_legend = array();
  $total_filtered = 0;
  $threshold = $total_visits * 0.01;

  arsort($mediums);
  $segments = 0;
  foreach ($mediums as $key => $value) {
    if($segments >= 3) {
      break;
    }
    if ($value > $threshold) {
      $percent = $value / $total_visits;
      $filtered_mediums[$key] = $percent;
      $filtered_mediums_legend[$key] = $key . " (" . number_format($value) . ")";
      $total_filtered  += $value;
      $segments++;
    }
  }
  $filtered_mediums['other'] = ($total_visits - $total_filtered) / $total_visits;
  $filtered_mediums_legend['other'] =  "other (" . number_format($total_visits - $total_filtered) . ")";

  // Create browser chart.
  $colors = seotools_get_data_colors();
  arsort($filtered_mediums);
  $mediums_data = array();
  $i=0;
  foreach($filtered_mediums AS $medium => $value) {
    $mediums_data[$medium] = $colors[$i];
    $i++;
  }
  $chart = array(
    '#chart_id' => 'medium_box',
    '#title' => '',
    '#type' => CHART_TYPE_PIE,
    '#size' => chart_size(300, 150),
    '#data_colors' => chart_data_colors($mediums_data),
    '#data' => $filtered_mediums,
    '#legends' => $filtered_mediums_legend,
  );

  $r_chart = chart_url($chart);
  $js['charts']['sources_pie'] = array(
    'src' => $r_chart,
    'location' => 'chart-sources',
  );
  drupal_add_js( array('seotools' => $js), 'setting');

  $output .= '<div class="chart-sources-wrapper">';
  //$output .= $r_chart;
  $output .= '</div>';



  // sources table
  if(!$data = seotools_cache_get('data_ga_sources', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('source'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 20,
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $data = seotools_fetch_ga_data($request, 'source');
    seotools_cache_set('data_ga_sources', $data, $date_range);
  }

  // get yesterday's data
  if(!$yesterdays_data = seotools_cache_get('data_ga_sources_yesterday', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('source'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 200,
      '#start_date' => date("Y-m-d", strtotime('-1 days')),
      '#end_date' => date("Y-m-d", strtotime('-1 days'))
    );
    $yesterdays_data = seotools_fetch_ga_data($request, 'source');
    seotools_cache_set('data_ga_sources_yesterday', $yesterdays_data, $date_range);
  }

    // get last weeks data
  if(!$aweekago_data = seotools_cache_get('data_ga_source_aweekago', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('source'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 200,
      '#start_date' => date("Y-m-d", strtotime('-7 days')),
      '#end_date' => date("Y-m-d", strtotime('-7 days'))
    );
    $aweekago_data = seotools_fetch_ga_data($request, 'source');
    seotools_cache_set('data_ga_source_aweekago', $aweekago_data, $date_range);
  }

   // get today's data
  $request = array(
    '#dimensions' => array('source'),
    '#metrics' => array('visits'),
    '#sort_metric' => array('-visits'),
    '#max_results' => 200,
    '#start_date' => date("Y-m-d"),
  );
  $todays_data = seotools_fetch_ga_data($request, 'source');

  $c = 1;
  foreach($data AS $source => $value) {
    //if($source == '(direct)') {
    //  continue;
    //}
    $today = '-';
    if($todays_data[$source]) {
      $today = number_format($todays_data[$source]['visits']);
    }
    $yesterday = '-';
    if($yesterdays_data[$source]) {
      $yesterday = number_format($yesterdays_data[$source]['visits']);
    }
    $aweekago = '-';
    if($aweekago_data[$source]) {
      $aweekago = number_format($aweekago_data[$source]['visits']);
    }
    $fsource = $source;
    if($source == '(none)') {
      $fsource = 'direct';
    }
    $rows[] = array(
      l(seotools_format_text_data($fsource), 'https://www.google.com/analytics/reporting/referring_source_detail', array('query' => "id=$gid&d1=$source", 'attributes' => array('target' => 'googleanalytics'))),
      array(
        'data' => number_format($value['visits']),
        'class' => 'numeric-cell',
      ),
      array(
        'data' => number_format($value['visits'] / $date_range['days'], 0),
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $aweekago,
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $yesterday,
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $today,
        'class' => 'numeric-cell',
      ),
    );
    $c++;
    if($c > 10) {
      break;
    }
  }
  $header = array(
    array('data' => t('Top sources'), 'class' => 'label-header'),
    array('data' => t('Visits'), 'class' => 'numeric-header'),
    array('data' => t('/day'), 'class' => 'numeric-header'),
    array('data' => t('Wkago'), 'class' => 'numeric-header'),
    array('data' => t('Yda'), 'class' => 'numeric-header'),
    array('data' => t('Today'), 'class' => 'numeric-header'),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-top-sources'));

  // geographic data
  if(!$data = seotools_cache_get('data_ga_country', $date_range)) {
    $request = array(
      '#dimensions' => array('pagePath', 'country'),
      '#metrics' => array('pageviews'),
      '#sort_metric' => array('country', 'pagePath'),
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $data = seotools_fetch_ga_data($request, 'country');
    seotools_cache_set('data_ga_country', $data, $date_range);
  }
  $name_to_iso = countries_api_get_array('name', 'iso2');
  $countries = array();
  $max_pageviews = 0;
  foreach ($data as $key => $value) {
    $match = $name_to_iso[strtoupper($key)];
    if ($match) {
      $max_pageviews = max($max_pageviews, $value['pageviews']);
      $countries[$match] = $value['pageviews'];
    }
  }

  // Find out proportion share from 1-100 for each country.
  $countries_share = array();
  foreach ($countries as $key => $value) {
    $countries_share[$key] = round(($value / $max_pageviews) * 100);
  }

  // Create the geographical chart.
  $chart = array(
    '#chart_id' => 'countries',
    '#type' => CHART_TYPE_MAP,
    '#size' => chart_size(440, 220),
    '#georange' => 'world',
    '#countries' => array_keys($countries_share),
    '#data' => array_values($countries_share),
    '#data_colors' => array('ffffff', 'edf0d4', '13390a'),
  );

  $output .= '<h3>' . t('Map Overlay') . '</h3>';

  //$r_chart = chart_url($chart);
  $r_chart = chart_render($chart);
  $js['charts']['sources_map'] = array(
    'src' => $r_chart,
    'location' => 'chart-map',
  );
  // TODO figure out why js crashing
  //drupal_add_js( array('seotools' => $js), 'setting');

  $output .= '<div class="chart-map-wrapper">';
  $output .= $r_chart;
  $output .= '</div>';

  return $output;
}

function seotools_dashboard_referrer_box($gid, $siteURL, $date_range, &$mediums_data) {
  $output = '<h3 class="referrers">' . t('Referrers') . '</h3>';

  $links[] = l(t('analytics'), 'https://www.google.com/analytics/reporting/referring_sources', array('query' => "id=$gid", 'attributes' => array('target' => 'googleanalytics')));
  $links[] = l(t('backlinks'), 'https://www.google.com/webmasters/tools/external-links', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));
  $links[] = l(t('internal links'), 'https://www.google.com/webmasters/tools/internal-links', array('query' => "siteUrl=$siteURL#", 'attributes' => array('target' => 'googlewebmastertools')));


  $output .= '<div class="dashboard-links">';
  $output .= implode(' | ', $links);
  $output .= '</div>';

  if(!$data = seotools_cache_get('data_ga_mediums_by_date', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('date', 'medium'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('date'),
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $rows = seotools_fetch_ga_data($request, array('date','medium'));
    seotools_cache_set('data_ga_mediums_by_date', $data, $date_range);
  }

  $totals = array();
  $day_counts = array();
  $top_mediums = array_keys($mediums_data);
  foreach ($rows as $key => $row) {
    $date = substr($key, 0, 8);
    $medium = substr($key, 8);
    if($medium == '(none)') {
      $medium = 'direct';
    }
    if(in_array($medium, $top_mediums)) {
      $day_counts[$medium][$date] = $row['visits'];
    }
    else {
      $day_counts['other'][$date] += $row['visits'];
    }
    $chart_dates[$date] = date('d', strtotime($date));
  }
  // sort by most visits
  $dc = $day_counts;
  $day_counts = array();
  foreach($top_mediums AS $medium) {
    $day_counts[$medium] = $dc[$medium];
  }
  $colors = seotools_get_data_colors();
  $i = 0;
  $series_colors = array();
  foreach($day_counts AS $medium => $series) {
    $series_colors[] = $mediums_data[$medium];
  }
  $chart = chart_url(_seotools_reports_multiline_chart('mediums_by_day', '', $day_counts, $chart_dates, $series_colors));
  $js['charts']['mediums_by_day'] = array(
    'src' => $chart,
    'location' => 'chart-refferrers',
    'title' => '',
  );
  drupal_add_js( array('seotools' => $js), 'setting');
  $output .= '<div class="chart-refferrers-wrapper">';
  $output .= '</div>';

  if(!$data = seotools_cache_get('data_ga_referring_pages', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('source', 'referralPath'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 200,
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $data = seotools_fetch_ga_data($request, array('source', 'referralPath'));
    seotools_cache_set('data_ga_referring_pages', $data, $date_range);
  }

    // get yesterday's data
  if(!$yesterdays_data = seotools_cache_get('data_ga_referring_pages_yesterday', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('source', 'referralPath'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 200,
      '#start_date' => date("Y-m-d", strtotime('-1 days')),
      '#end_date' => date("Y-m-d", strtotime('-1 days'))
    );
    $yesterdays_data = seotools_fetch_ga_data($request, array('source', 'referralPath'));
    seotools_cache_set('data_ga_referring_pages_yesterday', $yesterdays_data, $date_range);
  }

    // get last weeks data
  if(!$aweekago_data = seotools_cache_get('data_ga_referring_pages_aweekago', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('source', 'referralPath'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 200,
      '#start_date' => date("Y-m-d", strtotime('-7 days')),
      '#end_date' => date("Y-m-d", strtotime('-7 days'))
    );
    $aweekago_data = seotools_fetch_ga_data($request, array('source', 'referralPath'));
    seotools_cache_set('data_ga_referring_pages_aweekago', $aweekago_data, $date_range);
  }

   // get today's data
  $request = array(
    '#dimensions' => array('source', 'referralPath'),
    '#metrics' => array('visits'),
    '#sort_metric' => array('-visits'),
    '#max_results' => 200,
    '#start_date' => date("Y-m-d"),
  );
  $todays_data = seotools_fetch_ga_data($request, array('source', 'referralPath'));

  $c = 1;
  $rows = array();
  foreach($data AS $source => $value) {
    if(strpos($source, '(not set)') !== FALSE) {
      continue;
    }
    $today = '-';
    if($todays_data[$source]) {
      $today = number_format($todays_data[$source]['visits']);
    }
    $yesterday = '-';
    if($yesterdays_data[$source]) {
      $yesterday = number_format($yesterdays_data[$source]['visits']);
    }
    $aweekago = '-';
    if($aweekago_data[$source]) {
      $aweekago = number_format($aweekago_data[$source]['visits']);
    }
    $img = '<img src="' . base_path() . drupal_get_path('module', 'seotools') . '/images/url_icon.gif">';
    $rows[] = array(
      l($img, 'http://' . $source, array('html' => TRUE, 'attributes' => array('target' => '_blank')))
      . ' ' . l(seotools_format_text_data($source), 'https://www.google.com/analytics/reporting/referring_source_detail', array('query' => "id=$gid&d1=$source", 'attributes' => array('target' => 'googleanalytics'))),
      array(
        'data' => number_format($value['visits']),
        'class' => 'numeric-cell',
      ),
      array(
        'data' => number_format($value['visits'] / $date_range['days'], 0),
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $aweekago,
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $yesterday,
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $today,
        'class' => 'numeric-cell',
      ),
    );
    $c++;
    if($c > 10) {
      break;
    }
  }
  $header = array(
    array('data' => t('Top referrers'), 'class' => 'label-header'),
    array('data' => t('Visits'), 'class' => 'numeric-header'),
    array('data' => t('/day'), 'class' => 'numeric-header'),
    array('data' => t('Wkago'), 'class' => 'numeric-header'),
    array('data' => t('Yda'), 'class' => 'numeric-header'),
    array('data' => t('Today'), 'class' => 'numeric-header'),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-top-sources'));

  // Trending data
  $c = 1;
  $rows = array();
  foreach($todays_data AS $source => $value) {
    if(strpos($source, '(not set)') !== FALSE) {
      continue;
    }
    $yesterday = '-';
    if($yesterdays_data[$source]) {
      $yesterday = number_format($yesterdays_data[$source]['visits']);
    }
    $aweekago = '-';
    if($aweekago_data[$source]) {
      $aweekago = number_format($aweekago_data[$source]['visits']);
    }
    $histavg = '-';
    if($data[$source]) {
      $histavg = number_format($data[$source]['visits'] / $date_range['days'], 0);
    }
    $img = '<img src="' . base_path() . drupal_get_path('module', 'seotools') . '/images/url_icon.gif">';
    $rows[] = array(
      l($img, 'http://' . $source, array('html' => TRUE, 'attributes' => array('target' => '_blank')))
      . ' ' . l(seotools_format_text_data($source), 'https://www.google.com/analytics/reporting/referring_source_detail', array('query' => "id=$gid&d1=$source", 'attributes' => array('target' => 'googleanalytics'))),
      array(
        'data' => number_format($value['visits']),
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $histavg,
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $aweekago,
        'class' => 'numeric-cell',
      ),
      array(
        'data' => $yesterday,
        'class' => 'numeric-cell',
      ),
    );
    $c++;
    if($c > 10) {
      break;
    }
  }
  $header = array(
    array('data' => t('Trending referrers'), 'class' => 'label-header'),
    array('data' => t('Today'), 'class' => 'numeric-header'),
    array('data' => t('Avg/day'), 'class' => 'numeric-header'),
    array('data' => t('Wkago'), 'class' => 'numeric-header'),
    array('data' => t('Yda'), 'class' => 'numeric-header'),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-top-sources'));

  return $output;
}

function seotools_dashboard_goals_box($gid, $siteURL, $date_range) {
  $output = '<h3 class="referrers">' . t('Referrers') . '</h3>';
  $output .= '<div class="dashboard-links">';
  $output .= l(t('Traffic analytics'), 'https://www.google.com/analytics/reporting/sources', array('query' => "id=$gid", 'attributes' => array('target' => 'googleanalytics')));
  $output .= '</div>';

  if(!$data = seotools_cache_get('data_ga_referring_sites', $date_range)) {
    // for help with query see: http://code.google.com/apis/analytics/docs/gdata/gdataExplorer.html
    $request = array(
      '#dimensions' => array('source'),
      '#metrics' => array('visits'),
      '#sort_metric' => array('-visits'),
      '#max_results' => 20,
      '#start_date' => $date_range['start_date'],
      '#end_date' => $date_range['end_date']
    );
    $data = seotools_fetch_ga_data($request, 'source');
    seotools_cache_set('data_ga_referring_sites', $data, $date_range);
  }
  $kw_priority_options = kwresearch_get_priority_options();
  $c = 1;
  foreach($data AS $source => $value) {
    if($source == '(direct)') {
      continue;
    }
    $rows[] = array(
      l($source, 'https://www.google.com/analytics/reporting/referring_source_detail', array('query' => "id=$gid&d1=$source", 'attributes' => array('target' => 'googleanalytics'))),
      number_format($value['visits']),
    );
    $c++;
    if($c > 5) {
      break;
    }
  }
  $header = array(
    array('data' => t('Top sources'), 'class' => 'label-header'),
    array('data' => t('Visits'), 'class' => 'numeric-header'),
  );
  $output .= theme('table', $header, $rows, array('id' => 'seotools-dashboard-top-sources'));


  return $output;
}

function seotools_get_data_colors() {
  return array('058DC7','50B432','ED561B','EDEF00');
}

function seotools_format_text_data($text) {
  if(strlen($text) > 40) {
   return substr($text, 0, 40);
  }
  return $text;
}
