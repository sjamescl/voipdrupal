<?php
// $Id$

function voip_admin_default_form($node, &$form_state) {
  $form = array();
  $options = array();
  
  $server_array = VoipServer::getServers();

  // sort the array by server name
  usort($server_array, "_server_cmp");
  
  foreach ($server_array as $server) {
    $id = $server->getId();
    $name = $server->getName();
    $options[$id] = '';
    $form[$name]['id'] = array('#value' => $id);
    if (function_exists($server->getConfigForm())) {
      $form[$name]['configure'] = array('#value' => l(t('configure'), 'admin/voip/servers/'. $id));
    }
    else {
      $form[$name]['configure'] = array('#value' => t('No configuration options'));
    }
  }
  $form['default'] = array('#type' => 'radios', '#options' => $options, '#default_value' => variable_get('voip_default_server', 'log'));
  
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Set default voip server'),
  );
  
  return $form;
}

function _server_cmp($s1, $s2) {
  $name1 = $s1->getName();
  $name2 = $s2->getName();
  if($name1 == $name2) {
    return 0;
  }
  return ($name1 < $name2) ? -1 : 1;
}

function voip_admin_default_form_submit($form, &$form_state) {
  // Process form submission to set the default voip server
  if ($form_state['values']['default']) {
    drupal_set_message(t('Default voip server updated.'));
    variable_set('voip_default_server', $form_state['values']['default']);
  }
}

function theme_voip_admin_default_form($variables) {
  $form = $variables['form'];
  $output = '';
  $rows = array();
  foreach (element_children($form) as $name => $element) {
	if(isset($form[$element]['configure']['#value'])) {
	  $rows[] = array(
		drupal_render($form['default'][$form[$element]['id']['#value']]),
		$element,
		$form[$element]['configure']['#value'],
	  );
	}
  }
  $header = array(t('Default'), t('Name'), array('data' => t('Operations'), 'colspan' => 1));
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);

  return $output;
}

function voip_admin_server_form($node, &$form_state, $server_id) {
  $form = array();

  $server = VoipServer::getServer($server_id);
  $config_form = $server->getConfigForm();
  $config = $server->getConfig();
  if ($server && function_exists($config_form)) {
    drupal_set_title(t('@server configuration', array('@server' => $server->getName())));
    $form = $config_form($config);

    $form['submit'] = array(
      '#type' => 'submit', 
      '#value' => t('Save'),
    );
    $form['server'] = array('#type' => 'value', '#value' => $server);

    return $form;
  }
  drupal_not_found();
  exit();
}

/**
 * Validation for voip server configuration. If the server defines a validation
 * function it uses that.
 */
function voip_admin_server_form_validate($form, &$form_state) {
  // Pass validation to voip server
  $config_form = $form_state['values']['server']->getConfigForm();
  $function = $config_form .'_validate';
  if (function_exists($function)) {
    $function($form, $form_state);
  }
}

function voip_admin_server_form_submit($form, &$form_state) {
  $server = $form_state['values']['server'];
  
  // Remove unecessary values
  unset($form_state['values']['op'], $form_state['values']['submit'], $form_state['values']['server'], $form_state['values']['form_token'], $form_state['values']['form_id']);
  
  //Save VoIP Voice
  VoipVoice::setDefaultVoice($form_state['values']['voipcall_voice_id']);
  VoipVoice::setDefaultManVoice($form_state['values']['voipcall_man_voice_id']);
  VoipVoice::setDefaultWomanVoice($form_state['values']['voipcall_woman_voice_id']);
  unset($form_state['values']['voipcall_voice_id'], $form_state['values']['voipcall_man_voice_id'], $form_state['values']['voipcall_woman_voice_id']);
  
  // store server configuration
  $server_id = $server->getId();
  variable_set('voip_'. $server_id .'_settings', $form_state['values']);
  drupal_set_message(t('The server settings have been saved.'));

  $form_state['redirect'] = 'admin/voip/servers';
}

