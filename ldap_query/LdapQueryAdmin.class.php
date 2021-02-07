<?php

/**
 * @file
 * LDAP Query Admin Class.
 */

module_load_include('php', 'ldap_query', 'LdapQuery.class');
/**
 *
 */
class LdapQueryAdmin extends LdapQuery {

  /**
   * @param string $sid
   *   either 'all' or the ldap server sid.
   * @param $type
   *   = 'all', 'enabled'
   */
  public static function getLdapQueryObjects($sid = 'all', $type = 'enabled', $class = 'LdapQuery') {
    $queries = [];
    $select = [];
    try {
      $configs = config_get_names_with_prefix('ldap.query.');
      foreach ($configs as $config) {
        $select[] = (object) config_get($config, 'config');
      }
    }
    catch (Exception $e) {
      backdrop_set_message(t('query index query failed. Message = %message, query= %query',
        ['%message' => $e->getMessage(), '%query' => $e->query_string]), 'error');
      return [];
    }
    foreach ($select as $result) {
      $query = ($class == 'LdapQuery') ? new LdapQuery($result->qid) : new LdapQueryAdmin($result->qid);
      if (
          ($sid == 'all' || $query->sid == $sid)
          &&
          (!$type || $type == 'all' || ($query->status = 1 && $type == 'enabled'))
        ) {
        $queries[$result->qid] = $query;
      }
    }
    return $queries;

  }

  /**
   *
   */
  public function __construct($qid) {
    parent::__construct($qid);
  }

  /**
   *
   */
  protected function populateFromBackdropForm($op, $values) {

    foreach ($this->fields() as $field_id => $field) {
      if (isset($field['form']) && property_exists('LdapQueryAdmin', $field['property_name'])) {
        $value = $values[$field_id];
        if (isset($field['form_to_prop_functions'])) {
          foreach ($field['form_to_prop_functions'] as $function) {
            $value = call_user_func($function, $value);
          }
        }
        $this->{$field['property_name']} = $value;
      }
    }
    $this->inDatabase = ($op == 'edit');
  }

  /**
   *
   */
  public function save($op) {

    $op = $this->inDatabase ? 'edit' : 'insert';
    $values = [];
    foreach ($this->fields() as $field_id => $field) {
      if (isset($field['schema'])) {
        $values[$field_id] = $this->{$field['property_name']};
      }
    }
    $config = config('ldap.query.' . $values['qid']);
    $config->set('id', $values['qid']);
    $config->set('name', $values['name']);
    $config->set('config', $values);
    $config->save();
    $this->inDatabase = TRUE;

  }

  /**
   *
   */
  public function delete($qid) {
    if ($qid == $this->qid) {
      $this->inDatabase = FALSE;
      $config = config('ldap.query.' . $qid);
      $result = $config->delete();
      return !empty($result);
    }
    else {
      return FALSE;
    }
  }

  /**
   *
   */
  public function getActions() {
    $switch = ($this->status) ? 'disable' : 'enable';
    $actions = [];
    $actions[] = l(t('edit'), LDAP_QUERY_MENU_BASE_PATH . '/query/edit/' . $this->qid);
    if (property_exists($this, 'type')) {
      if ($this->type == 'Overridden') {
        $actions[] = l(t('revert'), LDAP_QUERY_MENU_BASE_PATH . '/query/delete/' . $this->qid);
      }
      if ($this->type == 'Normal') {
        $actions[] = l(t('delete'), LDAP_QUERY_MENU_BASE_PATH . '/query/delete/' . $this->qid);
      }
    }
    else {
      $actions[] = l(t('delete'), LDAP_QUERY_MENU_BASE_PATH . '/query/delete/' . $this->qid);
    }
    $actions[] = l(t('test'), LDAP_QUERY_MENU_BASE_PATH . '/query/test/' . $this->qid);
    $actions[] = l($switch, LDAP_QUERY_MENU_BASE_PATH . '/query/' . $switch . '/' . $this->qid);
    return $actions;
  }

  /**
   *
   */
  public function backdropForm($op) {
    $form['#prefix'] = t('<p>Setup an LDAP query to be used by other modules
      such as LDAP Feeds.</p>');

    $form['basic'] = [
      '#type' => 'fieldset',
      '#title' => t('Basic LDAP Query Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    ];

    $form['query'] = [
      '#type' => 'fieldset',
      '#title' => t('Query'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    ];

    $form['query_advanced'] = [
      '#type' => 'fieldset',
      '#title' => t('Advanced Query Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    ];

    foreach ($this->fields() as $field_id => $field) {
      $field_group = isset($field['form']['field_group']) ? $field['form']['field_group'] : FALSE;
      if (isset($field['form'])) {
        $form_item = $field['form'];
        $form_item['#default_value'] = $this->{$field['property_name']};
        if ($field_group) {
          $form[$field_group][$field_id] = $form_item;
          // Sirrelevant to form api.
          unset($form[$field_group][$field_id]['field_group']);
        }
        else {
          $form[$field_id] = $form_item;
        }
      }
    }

    $form['basic']['qid']['#disabled'] = ($op == 'edit');

    $servers = ldap_servers_get_servers(NULL, 'enabled');
    if (count($servers) == 0) {
      backdrop_set_message(t('No ldap servers configured. Please configure a server before an ldap query.'), 'error');
    }
    foreach ($servers as $sid => $server) {
      $server_options[$sid] = $server->name;
    }

    $form['basic']['sid']['#options'] = $server_options;

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => t('Save Query'),
    ];

    $action = ($op == 'add') ? 'Add' : 'Update';
    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $action,
      '#weight' => 100,
    ];

    return $form;
  }

  /**
   *
   */
  public function backdropFormValidate($op, $values) {
    $errors = [];

    if ($op == 'delete') {
      if (!$this->qid) {
        $errors['query_name_missing'] = 'Query name missing from delete form.';
      }
    }
    else {
      $this->populateFromBackdropForm($op, $values);
      $errors = $this->validate($op);
    }
    return $errors;
  }

  /**
   *
   */
  protected function validate($op) {
    $errors = [];
    if ($op == 'add') {
      $ldap_queries = $this->getLdapQueryObjects('all', 'all');
      if (count($ldap_queries)) {
        foreach ($ldap_queries as $qid => $ldap_query) {
          if ($this->qid == $ldap_query->qid) {
            $errors['qid'] = t('An LDAP Query with the name %qid already exists.', ['%qid' => $this->qid]);
          }
        }
      }
    }

    return $errors;
  }

  /**
   *
   */
  public function backdropFormSubmit($op, $values) {

    $this->populateFromBackdropForm($op, $values);

    if ($op == 'delete') {
      $this->delete($this);
    }
    // Add or edit.
    else {
      try {
        $save_result = $this->save($op);
      }
      catch (Exception $e) {
        $this->setError('Save Error',
          t('Failed to save object. Your form data was not saved.'));
      }
    }
  }

  /**
   *
   */
  protected function arrayToLines($array) {
    $lines = "";
    if (is_array($array)) {
      $lines = join("\n", $array);
    }
    elseif (is_array(@unserialize($array))) {
      $lines = join("\n", unserialize($array));
    }
    return $lines;
  }

  /**
   *
   */
  protected function arrayToCsv($array) {
    return join(",", $array);
  }

}
