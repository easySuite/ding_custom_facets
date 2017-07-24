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
    ),
    '#default_value' => (!empty($group->type)) ? $group->type : '',
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
      ->fields(array('name' => $form_state['input']['name']))
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
