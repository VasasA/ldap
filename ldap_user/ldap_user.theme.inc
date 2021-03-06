<?php

/**
 * @file
 * Theme functions for ldap_user module.
 */

/**
 * Turns array of variables to a form embedded in one table for each ldap server.
 */
function theme_ldap_user_conf_form($variables) {

  $form = $variables['form'];
  $tables = [];

  foreach ($form as $key => $element) {
    $parts = explode('__', $key);
    if (count($parts) == 4 && isset($element['#row']) && isset($element['#col'])) {
      list($direction, $discard, $column_name, $i) = $parts;
      $row = $element['#row'];
      $col = $element['#col'];
      $tables[$direction]['rows'][$row][$col] = ['data' => backdrop_render($form[$key]), 'class' => ['module'], 'colspan' => 1];
    }
  }

  $output = backdrop_render_children($form);
  $ldap_user_conf_admin = ldap_user_conf('admin');

  foreach ($tables as $direction => $table) {
    if ($direction == LDAP_USER_PROV_DIRECTION_TO_BACKDROP_USER) {

      $direction_text = 'tobackdrop';

      $header = [
        [
          'data' => t('Remove'),
          'rowspan' => 1,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('Source LDAP Tokens. (See https://backdropcms.org/project/ldap#tokens) ') ,
          'class' => 'mixedcase',
          'rowspan' => 1,
          'colspan' => 2,
        ],
        [
          'data' => t('Target Backdrop Attribute'),
          'rowspan' => 1,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('When Should Attribute Be Synchronized to Backdrop?'),
          'colspan' => count($ldap_user_conf_admin->provisionsBackdropEvents),
          'rowspan' => 1,
          'class' => 'mixedcase',
        ],

      ];

      $second_header = [
        [
          'data' => t(''),
          'header' => TRUE,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('e.g: "[sn]", "[mail:0]", "[ou:last]", "[sn], [givenName]" etc.
                Constants such as "17" or "imported" should not be enclosed in [].'),
          'header' => TRUE,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('Convert From Binary?'),
          'class' => 'mixedcase',
          'header' => TRUE,
        ],
        [
          'data' => "",
          'header' => TRUE,
        ],
      ];

      foreach ($ldap_user_conf_admin->provisionsBackdropEvents as $col_id => $col_name) {
        $second_header[] = ['data' => $col_name, 'header' => TRUE, 'class' => 'mixedcase'];
      }
    }
    // To ldap.
    else {

      $direction_text = 'toldap';

      $header = [
        [
          'data' => t('Remove'),
          'rowspan' => 1,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('Source Backdrop User Attribute') ,
          'class' => 'mixedcase',
          'rowspan' => 1,
          'colspan' => 3,
        ],
        [
          'data' => t('Target LDAP Token'),
          'rowspan' => 1,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('When Should Attribute Be Synchronized to LDAP?'),
          'colspan' => count($ldap_user_conf_admin->provisionsLdapEvents),
          'rowspan' => 1,
          'class' => 'mixedcase',
        ],
      ];

      $second_header = [
        [
          'data' => t(''),
          'header' => TRUE,
        ],
        [
          'data' => t('(Select "user tokens" to use token field)'),
          'header' => TRUE,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('Source Backdrop User tokens such as: "[property.name]", "[field.field_fname] [field.field_lname]". Constants such as "from_backdrop" or "18" should not be enclosed in []'),
          'header' => TRUE,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('Convert From Binary'),
          'header' => TRUE,
          'class' => 'mixedcase',
        ],
        [
          'data' => t('Use singular token format such as [sn], [givenName], etc.'),
          'header' => TRUE,
          'class' => 'mixedcase',
        ],
      ];
      foreach ($ldap_user_conf_admin->provisionsLdapEvents as $col_id => $col_name) {
        $second_header[] = ['data' => $col_name, 'header' => TRUE, 'class' => 'mixedcase'];
      }
    }

    // #col is ignored, so sort table columns here.
    foreach ($table['rows'] as $row_id => $row) {
      $discard = ksort($row);
      $table['rows'][$row_id] = $row;
    }

    $table['header'] = $header;
    $table['attributes'] = ['class' => ['ldap-provision-' . $direction_text]];
    array_unshift($table['rows'], $second_header);
    $table_themed = theme('table', $table);
    $rendered_table = theme('table', $table);
    $output = str_replace('[replace_with_table__' . $direction . ']', $rendered_table, $output);
  }

  return $output;

}
