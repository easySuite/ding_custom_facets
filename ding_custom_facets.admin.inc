<?php

/**
 * @file
 * Administration page.
 */

/**
 * Groups overview page.
 */
function ding_custom_facets_admin_page() {
  $form['description'] = array(
    '#markup' => '<h3>Manage custom facet groups.</h3>',
  );

  $header = array(t('Name'), t('Operations'));

  $records = db_select('ding_custom_facets_groups', 'groups')
    ->fields('groups')
    ->execute()
    ->fetchAll();

  $rows = array();
  foreach ($records as $record) {
    $rows[] = array(
      'name' => l($record->name, "admin/config/ding/facet-groups/{$record->gid}/view"),
      'operations' => l(t('edit'), "admin/config/ding/facet-groups/{$record->gid}/edit") . " | " . l(t('delete'), "admin/config/ding/facet-groups/{$record->gid}/delete"),
    );
  }

  $output = theme('table',
    array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'groups',
      ),
    )
  );

  $output .= drupal_render($form['actions']);
  $output = drupal_render_children($form) . $output;
  return $output;
}

/**
 * Call "Add group" form.
 */
function ding_custom_facets_add_item_page() {
  $group = new stdClass();
  $group->gid = FALSE;
  $group->name = '';

  return drupal_get_form('ding_custom_facets_item_form', $group);
}

/**
 * Group create/update form.
 */
function ding_custom_facets_item_form($form, &$form_state, $group) {
  $form['#group'] = $group;

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Group name'),
    '#default_value' => $group->name,
    '#required' => TRUE,
  );

  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Group type'),
    '#required' => TRUE,
    '#options' => array(
      'simple' => t('Simple'),
      'advanced' => t('Advanced'),
      'os_facets' => t('OS Facets'),
    ),
    '#default_value' => (!empty($group->type)) ? $group->type : 'simple',
    '#description' => t('Select type of widget which will be processed. Simple: extends current search and MUST be placed on search page. Advanced: provide already formed search request.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Group create/update submit handler.
 */
function ding_custom_facets_item_form_submit($form, &$form_state) {
  $group = $form['#group'];

  if (isset($group->gid) && $group->gid) {
    db_update('ding_custom_facets_groups')
      ->fields(
        array(
          'name' => $form_state['values']['name'],
          'type' => $form_state['values']['type'],
        )
      )
      ->condition('gid', $group->gid)
      ->execute();
    $form_state['redirect'] = 'admin/config/ding/facet-groups/' . $group->gid;
  }
  else {
    $gid = db_insert('ding_custom_facets_groups')
      ->fields(
        array(
          'name' => $form_state['input']['name'],
          'type' => $form_state['input']['type'],
        )
      )
      ->execute();
    $form_state['redirect'] = 'admin/config/ding/facet-groups/' . $gid;
  }
}

/**
 * Group deletion form.
 */
function ding_custom_facets_item_delete_form($form, &$form_state, $group) {
  $form['#group'] = $group;

  $form['header'] = array(
    '#markup' => t('Are you sure you wish to delete the item with a @value group?', array('@value' => $group->name)),
    '#prefix' => '',
    '#suffix' => '',
  );
  $form['warning'] = array(
    '#markup' => t('Warning, this action cannot be undone'),
    '#prefix' => '',
    '#suffix' => '',
  );
  $form['delete_button'] = array(
    '#type' => 'submit',
    '#value' => t('Delete item'),
  );
  return $form;
}

/**
 * Group deletion submit handler.
 */
function ding_custom_facets_item_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == $form_state['values']['delete_button']) {
    $group = $form['#group'];

    db_delete('ding_custom_facets_groups')
      ->condition('gid', $group->gid)
      ->execute();
  }
  $form_state['redirect'] = 'admin/config/ding/facet-groups';
  drupal_set_message(t('The item has been deleted'));
}

/**
 * Group view page.
 */
function ding_custom_facets_view_item_page($group) {
  $page['group_item_page'] = array(
    '#markup' => $group->name,
    '#prefix' => '<h3>',
    '#suffix' => '</h3>',
  );

  $page['terms_form'] = drupal_get_form('ding_custom_facets_terms_form', $group);

  return $page;
}

/**
 * Rendering group's terms table.
 */
function ding_custom_facets_terms_form($form, &$form_state, $group) {
  form_load_include($form_state, 'inc', 'ding_custom_facets', 'ding_custom_facets.admin');
  $form = array();

  $terms = db_select('ding_custom_facets_terms', 'terms')
    ->fields('terms')
    ->condition('gid', $group->gid)
    ->execute()
    ->fetchAll();

  $form['description'] = array(
    '#markup' => '<p>' . t('Configure terms from present group.') . '</p>',
  );

  $form['group_terms_fieldset'] = array(
    '#tree' => TRUE,
    '#weight' => -20,
  );

  $i = 0;
  foreach ($terms as $term) {
    $form['group_terms_fieldset']['terms'][$i]['id'] = array(
      '#type' => 'hidden',
      '#default_value' => $term->id,
    );

    $form['group_terms_fieldset']['terms'][$i]['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#title_display' => 'invisible',
      '#default_value' => $term->label,
    );

    $form['group_terms_fieldset']['terms'][$i]['term'] = array(
      '#type' => 'textfield',
      '#title' => t('CQL Query'),
      '#title_display' => 'invisible',
      '#default_value' => $term->term,
    );

    $form['group_terms_fieldset']['terms'][$i]['delete'] = array(
      '#type' => 'link',
      '#title' => t('delete'),
      '#href' => 'admin/config/ding/facet-groups/' . $group->gid . '/term/' . $term->id . '/delete',
      '#options' => array('attributes' => array('title' => t('Delete facet.'))),
    );

    $i++;
  }

  $form['group_terms_fieldset']['terms'][$i]['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => '',
    '#title_display' => 'invisible',
  );

  $form['group_terms_fieldset']['terms'][$i]['term'] = array(
    '#type' => 'textfield',
    '#title' => t('CQL Query'),
    '#default_value' => '',
    '#title_display' => 'invisible',
  );

  $form['group_terms_fieldset']['gid'] = array(
    '#type' => 'hidden',
    '#default_value' => $group->gid,
  );

  $i++;

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );

  return $form;
}

/**
 * Group's terms creation handler.
 */
function ding_custom_facets_terms_form_submit($form, &$form_state) {
  $terms = $form_state['values']['group_terms_fieldset']['terms'];
  $gid = $form_state['values']['group_terms_fieldset']['gid'];

  foreach ($terms as $key => $term) {
    if (!empty($term['label'])) {
      if (!empty($term['id'])) {
        db_update('ding_custom_facets_terms')
          ->fields(array(
            'label' => $term['label'],
            'term' => $term['term'],
          ))
          ->condition('id', $term['id'], '=')
          ->condition('gid', $gid, '=')
          ->execute();
      }
      else {
        db_insert('ding_custom_facets_terms')
          ->fields(array(
            'gid' => $gid,
            'label' => $term['label'],
            'term' => $term['term'],
          ))
          ->execute();
      }
    }
    else {
      unset($terms[$key]);
    }
  }

  drupal_set_message(t('The configuration options have been saved.'));
}

/**
 * Theming function for group's terms table.
 */
function theme_ding_custom_facets_terms_form($vars) {
  $form = $vars['form'];

  $group_gid = $form['group_terms_fieldset']['gid']['#value'];

  $group_type = db_select('ding_custom_facets_groups', 'dcfg')
    ->fields('dcfg', array('type'))
    ->condition('gid', $group_gid)
    ->execute()
    ->fetchField();

  switch ($group_type) {
    case $group_type == 'os_facets':
      $form = drupal_get_form('ding_custom_facets_os_facets_form');
      $output = drupal_render($form);
      return $output;
      break;

    default:
      $rows = array();

      foreach (element_children($form['group_terms_fieldset']['terms']) as $key) {
        $term = &$form['group_terms_fieldset']['terms'][$key];

        $row = array();
        $row[] = drupal_render($term['label']);
        $row[] = drupal_render($term['term']);
        $row[] = drupal_render($term['delete']);
        $rows[] = array(
          'data' => $row,
          'class' => array('draggable'),
        );
      }

      $header = array(t('Label'), t('CQL Query'), t('Operations'));
      $output = theme('table',
        array(
          'header' => $header,
          'rows' => $rows,
          'attributes' => array(
            'id' => 'terms',
          ),
        )
      );
      $output .= drupal_render($form['actions']);
      $output = drupal_render_children($form) . $output;

      return $output;
      break;
  }
}

/**
 * Delete term from a group.
 */
function ding_custom_facets_terms_delete_term($group, $term_id) {
  db_delete('ding_custom_facets_terms')
    ->condition('id', $term_id)
    ->execute();

  drupal_set_message(t('Term was successfully deleted.'));
  drupal_goto("admin/config/ding/facet-groups/{$group->gid}/view");
}

/**
 * Custom form for OS Facets.
 */
function ding_custom_facets_os_facets_form($form, &$form_state) {
  $rows_new = isset($form_state['facet_rows']) ? $form_state['facet_rows'] : 1;
  $form_state['facet_rows'] = $rows_new;

  $form['fetch_facet_wrapper'] = array(
    '#type' => 'fieldset',
    '#title' => t('Fetch facet terms'),
  );

  $facets = variable_get('facets_wrapper', array());

  $form['fetch_facet_wrapper']['facet_identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Facet idenfitier'),
    '#default_value' => isset($facets['identifier']) ? $facets['identifier'] : 'department',
    '#required' => TRUE,
  );

  $form['fetch_facet_wrapper']['fetch_facet_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Fetch terms'),
    '#ajax' => array(
      'callback' => '_ding_custom_facets_fetch_facet_callback',
      'method' => 'replace',
      'effect' => 'fade',
      'wrapper' => 'ding-custom-facets-admin-form',
    ),
    '#submit' => array('_ding_custom_facets_fetch_submit'),
    '#limit_validation_errors' => array(
      array('facet_identifier'),
    ),
  );

  $form['facet_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Facet name'),
    '#required' => TRUE,
    '#default_value' => isset($facets['title']) ? $facets['title'] : '',
  );

  $form['facets_wrapper'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#theme' => 'ding_custom_facets_facet_element',
    '#prefix' => '<div id="ding-custom-facets-wrapper">',
    '#suffix' => '</div>',
  );

  if (isset($form_state['input']['facets_wrapper'])) {
    $form_state['input']['facets_wrapper'] = array_values($form_state['input']['facets_wrapper']);
  }

  $rows = isset($facets['facets']) ? $facets['facets'] : array();
  for ($i = 0; $i < count($rows); $i++) {
    $form['facets_wrapper'][] = ding_custom_facets_row_form($form_state, $rows[$i]);
  }

  for ($i = 0; $i < $rows_new; $i++) {
    $form['facets_wrapper'][] = ding_custom_facets_row_form($form_state);
  }

  $form['new'] = array(
    '#type' => 'submit',
    '#value' => t('Add option'),
    '#ajax' => array(
      'callback' => '_ding_custom_facets_row_callback',
      'effect' => 'fade',
      'method' => 'replace',
      'wrapper' => 'ding-custom-facets-wrapper',
    ),
    '#submit' => array('_ding_custom_facets_row_submit'),
    '#limit_validation_errors' => array(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  $form['#prefix'] = '<div id="ding-custom-facets-admin-form">';
  $form['#suffix'] = '</div>';

  return $form;
}

/**
 * Define single row form elements.
 *
 * @param array $form_state
 *   The form state.
 * @param array $default
 *   Default form elements values, if any.
 *
 * @return array
 *   Form structure.
 */
function ding_custom_facets_row_form(&$form_state, $default = array()) {
  $form = array();

  $form['weight'] = array(
    '#type' => 'weight',
    '#default_value' => isset($default['weight']) ? $default['weight'] : 0,
    '#attributes' => array(
      'class' => array('sort'),
    ),
  );

  $form['facet'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($default['facet']) ? $default['facet'] : '',
  );

  $terms = variable_get('ding_custom_facets_terms', array());
  $form['terms'] = array(
    '#type' => 'select',
    '#options' => array_combine($terms, $terms),
    '#default_value' => isset($default['terms']) ? $default['terms'] : array(),
    '#multiple' => TRUE,
  );

  return $form;
}

/**
 * AJAX callback for fetch facet terms button.
 *
 * @see ding_custom_facets_admin_form()
 */
function _ding_custom_facets_fetch_facet_callback($form, &$form_state) {
  return $form;
}

/**
 * Custom submit handler for fetching facet terms button.
 *
 * @see ding_custom_facets_admin_form()
 */
function _ding_custom_facets_fetch_submit($form, &$form_state) {
  $input = $form_state['input'];
  if (!empty($input['facet_identifier'])) {
    $terms = ding_custom_facets_facets('facet.' . $input['facet_identifier']);
  }

  $form_state['rebuild'] = TRUE;
}


/**
 * Request terms for certain facet.
 *
 * @param string $facet
 *   Facet identifier.
 *
 * @return array
 *   Sorted list of facet terms.
 */
function ding_custom_facets_facets($facet = 'facet.department') {
  module_load_include('client.inc', 'ting');

  $options = array(
    'facets' => array($facet),
    'numFacets' => 9999,
    'reply_only' => TRUE,
  );
  $result = ting_do_search("*", 0, 0, $options);

  $types = array();
  foreach ($result->facets[$facet]->terms as $term => $count) {
    $types[drupal_strtolower($term)] = $count;
  }

  $types = array_keys($types);
  sort($types);

  // Only save if we actually got any types.
  if (!empty($types)) {
    variable_set('ding_custom_facets_terms', $types);
  }

  return $types;
}

/**
 * Custom theming callback for facet rows element.
 *
 * Used to render draggable rows in a table.
 *
 * @see ding_custom_facets_admin_form()
 * @see ding_custom_facets_theme()
 */
function theme_ding_custom_facets_facet_element($variables) {
  $element = $variables['element'];
  drupal_add_tabledrag('ding-custom-facets-table', 'order', 'sibling', 'sort');

  $header = array(
    t('Facet label'),
    t('Facet terms'),
    t('Weight'),
  );

  $rows = array();

  foreach (element_children($element) as $k => $v) {
    $rows[] = array(
      'data' => array(
        drupal_render($element[$k]['facet']),
        drupal_render($element[$k]['terms']),
        drupal_render($element[$k]['weight']),
      ),
      'class' => array('draggable'),
    );
  }

  $table = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'ding-custom-facets-table',
    ),
  );

  $output = theme('table', $table);
  $output .= drupal_render_children($element);

  return $output;
}

/**
 * Custom validate handler for facets form.
 *
 * @see ding_custom_facets_admin_form()
 */
function ding_custom_facets_os_facets_form_validate($form, &$form_state) {
  $input = $form_state['input'];

  foreach ($input['facets_wrapper'] as $k => $v) {
    if (!empty($v['facet']) && empty($v['terms'])) {
      form_set_error('facets_wrapper][' . $k . '][terms', t('Select term(s) for the facet <em>@facet</em>.', array('@facet' => $v['facet'])));
    }
  }
}

/**
 * Custom submit handler for facets form.
 *
 * @see ding_custom_facets_admin_form()
 */
function ding_custom_facets_os_facets_form_submit($form, &$form_state) {
  $input = $form_state['input'];
  $facets = array_filter($input['facets_wrapper'], function ($v) {
    return (!empty($v['facet']) && !empty($v['terms']));
  });

  $facets = array(
    'title' => trim($input['facet_name']),
    'identifier' => trim($input['facet_identifier']),
    'facets' => array_values($facets),
  );

  variable_set('facets_wrapper', $facets);
  drupal_set_message(t('Custom facets saved.'));
}


/**
 * Custom submit handler for new row button.
 *
 * @see ding_custom_facets_admin_form()
 */
function _ding_custom_facets_row_submit($form, &$form_state) {
  $form_state['facet_rows']++;
  $form_state['rebuild'] = TRUE;
}


/**
 * AJAX callback for new row button.
 *
 * @see ding_custom_facets_admin_form()
 */
function _ding_custom_facets_row_callback($form, &$form_state) {
  return $form['facets_wrapper'];
}
