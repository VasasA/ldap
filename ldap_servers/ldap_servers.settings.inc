<?php

/**
 * @file
 * Admin interface for general ldap api settings.
 */

/**
 *
 */
function ldap_servers_settings() {
  ldap_servers_module_load_include('inc', 'ldap_servers', 'ldap_servers.functions');

  if (!ldap_servers_ldap_extension_loaded()) {
    backdrop_set_message(t('PHP LDAP Extension is not loaded.'), "warning");
  }

  $form['#config'] = 'ldap_servers.settings';
  $form['#title'] = "Configure LDAP Preferences";

  $options = ldap_servers_encrypt_types();

  /**  when this is changed, need to decrypt and possibly encrypt pwd in newly selected format
   *   ... thus default needs to be "No Encryption" to avoid confusion.
   */
  $config = config('ldap_servers.settings');
  $form['previous_encryption'] = ['#type' => 'hidden', '#default_value' => $config->get('ldap_servers_encryption')];
  $form['encryption'] = ['#type' => 'fieldset', '#title' => t('Encryption')];
  $form['encryption']['ldap_servers_encryption'] = [
    '#type' => 'select',
    '#options' => $options,
    '#title' => t('Obfuscate LDAP Passwords?'),
    '#default_value' => $config->get('ldap_servers_encryption'),
    '#description' => t('With obfuscation enabled, passwords will be stored in encrypted form and decrypted with the site hash.'),
  ];

  // Disable the form field and explain this to the user.
  if (count($options) == 1) {
    $form['encryption']['ldap_servers_encryption']['#disabled'] = TRUE;
    $form['encryption']['ldap_servers_encryption']['#description'] = ' <strong>' . t('Obfuscation is not supported on this web server.') . '</strong>';
  }

  $form = system_settings_form($form);
  // Needs to be first.
  array_unshift($form['#submit'], 'ldap_servers_settings_submit');
  return $form;
}

/**
 *
 */
function ldap_servers_settings_submit($form, &$form_state) {
  if ($form_state['submitted']) {
    $new_encyption = $form_state['values']['ldap_servers_encryption'];
    $old_encyption = $form_state['values']['previous_encryption'];

    // Use db instead of functions to avoid classes encryption and decryption.
    if ($new_encyption != $old_encyption) {
      $servers = [];
      $configs = config_get_names_with_prefix('ldap.server.');
      foreach ($configs as $config) {
        $server_config = (object) config_get($config, 'config');
        if (strlen($server_config->bindpw) > 0) {
          $servers[$config] = $server_config;
        }
      }
      foreach ($servers as $config => $server) {
        if ($server->bindpw != '') {
          $decrypted_bind_pwd = ldap_servers_decrypt($server->bindpw, $old_encyption);
          $rencrypted = ldap_servers_encrypt($decrypted_bind_pwd, $new_encyption);
        }
        else {
          $rencrypted = '';
        }
        $server->bindpw = $rencrypted;
        config_set($config, 'config', $server);
      }
    }
  }
}
