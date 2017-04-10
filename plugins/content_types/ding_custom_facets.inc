<?php

/**
 * @file
 * Custom Facet groups pane.
 */

$plugin = array(
  'single' => TRUE,
  'title' => t('Custom facet groups + terms'),
  'description' => t('Display Groups of terms for extending existing facets.'),
  'category' => t('Ding!'),
  'edit form' => 'ding_custom_facets_settings',
  'render callback' => 'ding_custom_facets_render',
  'defaults' => array(),
  'all contexts' => TRUE,
);

/**
 * Pane settings form.
 */
function ding_custom_facets_settings($form, &$form_state) {
  $conf = $form_state['conf'];

  $groups = db_select('ding_custom_facets_groups', 'groups')
    ->fields('groups')
    ->execute()
    ->fetchAll();

  $options = array();
  foreach ($groups as $group) {
    $options[$group->gid] = $group->name;
  }

  $form['displayed_groups'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Groups',
    '#description' => 'Choose which groups will be displayed.',
    '#options' => $options,
    '#default_value' => $conf['displayed_groups'],
  );

  return $form;
}

/**
 * Pane settings submit handler.
 */
function ding_custom_facets_settings_submit($form, &$form_state) {
  $form_state['conf']['displayed_groups'] = $form_state['values']['displayed_groups'];
}

/**
 * Pane rendering function.
 */
function ding_custom_facets_render($subtype, $conf, $args, $contexts) {
  if (empty($conf)) {
    return;
  }

  $groups = array_filter($conf['displayed_groups']);

  $output = '';
  foreach ($groups as $group) {
    $object = db_query('SELECT gid, name FROM {ding_custom_facets_groups} WHERE gid = :gid', array(':gid' => $group))->fetchObject();

    $terms = db_select('ding_custom_facets_terms', 'terms')
      ->fields('terms')
      ->condition('gid', $group)
      ->execute()
      ->fetchAll();


    $form = drupal_get_form('ding_custom_facets_render_form', $object, $terms);
    $output .= drupal_render($form);
  }

  $block = new stdClass();
  $block->title = '';
  $block->content = $output;

  return $block;
}
